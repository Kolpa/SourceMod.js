#include "SMJS.h"
#include "extension.h"
#include "modules/MSocket.h"
#include "modules/MPlugin.h"
#include <cstdlib>

v8::Isolate *mainIsolate;


time_t lastPing;
bool isPaused = false;

uv_thread_t checkerThread;
uv_mutex_t primaryMutex;
uv_loop_t *uvLoop;

//TODO: Read this from sourcemod.js/dota.js
const char *scriptDotaStr = NULL;
v8::ScriptData *scriptDotaData;

static void SMJS_OnPausedTick();
static void OnMessage(Handle<Message> message, Handle<Value> error);

void checkerThreadFunc(void*);

void SMJS_Init(){
	uvLoop = uv_default_loop();

	mainIsolate = v8::Isolate::GetCurrent();
	HandleScope handle_scope(mainIsolate);

	V8::SetCaptureStackTraceForUncaughtExceptions(true, 10, (StackTrace::StackTraceOptions)(StackTrace::kLineNumber | StackTrace::kColumnOffset | StackTrace::kScriptName | StackTrace::kFunctionName | StackTrace::kScriptNameOrSourceURL));

	char smjsPath[512];
	smutils->BuildPath(Path_SM, smjsPath, sizeof(smjsPath), "sourcemod.js");

	scriptDotaStr = SMJS_FileToString("dota.js", smjsPath);
	if(scriptDotaStr == NULL){
		printf("File \"dota.js\" missing\n");
		getchar();
	}
	scriptDotaData = v8::ScriptData::PreCompile(v8::String::New(scriptDotaStr));

	v8::V8::AddMessageListener(OnMessage);

	uv_mutex_init(&primaryMutex);
	SMJS_Ping();
	uv_thread_create(&checkerThread, checkerThreadFunc, NULL);
}

void checkerThreadFunc(void*){
	while(1){
		uv_mutex_lock(&primaryMutex);

		int now = time(NULL);
		if(!isPaused && now - lastPing > 30){
			v8::V8::TerminateExecution(mainIsolate);
			printf("SERVER STOPPED THINKING, QUITTING\n");
			threader->ThreadSleep(3000);

			// In production, just let the server die so it can be restarted
			// Otherwise we'd have to close the dialog that Windows creates before
			// the server is restarted again
#ifdef DEBUG
			abort();
#else
#ifdef WIN32
			TerminateProcess(GetCurrentProcess(), 3);
#else
			exit(3);
#endif
#endif
		}

		uv_mutex_unlock(&primaryMutex);
#ifdef WIN32
		Sleep(1000);
#else
		sleep(1);
#endif
	}
}


void OnMessage(Handle<Message> message, Handle<Value> error){
	HandleScope handle_scope(mainIsolate);

	fprintf(stderr, "----------------------------- JAVASCRIPT EXCEPTION -----------------------------");


	v8::String::Utf8Value tmp(message->Get());
	const char* exception_string = *tmp;
	char buffer[2048];
	buffer[0] = '\0';

	if (message.IsEmpty()) {
		// V8 didn't provide any extra information about this error; just
		// print the exception.
		
		snprintf(buffer, sizeof(buffer), "%s%s\n", buffer, exception_string);
	} else {
		// Print (filename):(line number): (message).
		String::Utf8Value filename(message->GetScriptResourceName());
		const char* filename_string = *filename;
		int linenum = message->GetLineNumber();
		snprintf(buffer, sizeof(buffer), "%s%s:%d: %s\n", buffer, filename_string, linenum, exception_string);
		// Print line of source code.
		String::Utf8Value sourceline(message->GetSourceLine());
		const char* sourceline_string = *sourceline;
		snprintf(buffer, sizeof(buffer), "%s%s\n", buffer, sourceline_string);
		// Print wavy underline (GetUnderline is deprecated).
		int start = message->GetStartColumn();
		for (int i = 0; i < start; i++) {
			snprintf(buffer, sizeof(buffer), "%s%c", buffer, ' ');
		}
		int end = message->GetEndColumn();
		for (int i = start; i < end; i++) {
			snprintf(buffer, sizeof(buffer), "%s%c", buffer, '^');
		}
		snprintf(buffer, sizeof(buffer), "%s%c", buffer, '\n');

		auto stackTrace = message->GetStackTrace();
		if(!stackTrace.IsEmpty()){
			int len = stackTrace->GetFrameCount();

			for(int i = 0; i < len; ++i){
				auto frame = stackTrace->GetFrame(i);
				v8::String::Utf8Value scriptName(frame->GetScriptNameOrSourceURL());
				v8::String::Utf8Value funName(frame->GetFunctionName());
				
				snprintf(buffer, sizeof(buffer), "%s%s: %s:%d\n",
					buffer,
					*scriptName == NULL ? "(?)" : *scriptName,
					*funName,
					frame->GetLineNumber());
			}
		}
	}

	fprintf(stderr, "%s", buffer);
	fprintf(stderr, "--------------------------------------------------------------------------------\n");
	if(MPlugin::masterPlugin != -1){
		auto master = GetPlugin(MPlugin::masterPlugin);
		auto scriptData = message->GetScriptData();

		if(master != NULL){
			v8::Handle<v8::Value> args[2];
			args[0] = v8::String::New("?");
			if(!scriptData.IsEmpty()){
				args[0] = scriptData->ToString();
			}

			args[1] = v8::String::New(buffer);
			auto hooks = master->GetHooks("OnPluginError");
			for(auto it = hooks->begin(); it != hooks->end(); ++it){
				(*it)->Call(master->GetContext()->Global(), 2, args);
			}
		}
	}
}

void SMJS_Ping(){
	uv_mutex_lock(&primaryMutex);
	lastPing = time(NULL);
	uv_mutex_unlock(&primaryMutex);
}

void SMJS_Pause(){
	printf("Entering in deep sleep\n");

	uv_mutex_lock(&primaryMutex);
	if(isPaused){
		uv_mutex_unlock(&primaryMutex);
		return;
	}

	isPaused = true;
	uv_mutex_unlock(&primaryMutex);

	while(1){
		uv_mutex_lock(&primaryMutex);
		if(!isPaused){
			uv_mutex_unlock(&primaryMutex);
			return;
		}

		uv_mutex_unlock(&primaryMutex);

		SMJS_OnPausedTick();

#ifdef WIN32
		Sleep(100);
#else
		usleep(100000);
#endif
		
	}
}

void SMJS_Resume(){
	uv_mutex_lock(&primaryMutex);
	if(!isPaused){
		uv_mutex_unlock(&primaryMutex);
		return;
	}

	lastPing = time(NULL);
	isPaused = false;
	uv_mutex_unlock(&primaryMutex);
}

void SMJS_OnPausedTick(){
	MSocket::Process();
}

char* SMJS_FileToString(const char* file, const char* dir){
	char path[512];
	if(dir != NULL){
#if WIN32
		snprintf(path, sizeof(path), "%s\\%s", dir, file);
#else
		snprintf(path, sizeof(path), "%s/%s", dir, file);
#endif
	}else{
		strcpy(path, file);
	}

	FILE* fileHandle = fopen(path, "rb");
	if(fileHandle == NULL) return false;

	fseek(fileHandle, 0, SEEK_END);
	size_t size = ftell(fileHandle);
	rewind(fileHandle);
	
	char* source = new char[size + 1];
	size_t i = 0;
	while(i < size) {
		i += fread(&source[i], 1, size - i, fileHandle);
		if(ferror(fileHandle)){
			printf("Error reading file %s\n", path);
			perror(NULL);
			fclose(fileHandle);
			delete[] source;
			return false;
		}
	}
	source[size] = '\0';
	fclose(fileHandle);

	return source;
}