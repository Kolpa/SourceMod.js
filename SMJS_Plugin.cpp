#include "extension.h"
#include "SMJS_Plugin.h"
#include "modules/MPlugin.h"


std::vector<SMJS_Module*> modules;
std::vector<SMJS_Plugin*> plugins;
PLUGIN_ID numPlugins = 0;



inline const char* ToCString(const String::Utf8Value& value) {
  return *value ? *value : "NULL";
}

void SMJS_AddModule(SMJS_Module *module){
	module->IncRef();
	modules.push_back(module);
}


SMJS_Plugin *GetPlugin(PLUGIN_ID id){
	if(id < 0 || id > (int) plugins.size()) return NULL;
	return plugins[id];
}

int GetNumPlugins(){
	return plugins.size();
}

SMJS_Plugin* SMJS_Plugin::GetPluginByDir(const char *dir){
	for(auto it = plugins.begin(); it != plugins.end(); ++it){
		if(*it == NULL) continue;
		if(strcmp((*it)->dir.c_str(), dir) == 0) return *it;
	}

	return NULL;
}

SMJS_Plugin::SMJS_Plugin(bool isSandboxed){
	apiVersion = SMJS_API_VERSION;
	id = numPlugins++;
	plugins.push_back(this);
	this->isSandboxed = isSandboxed;
	apiVersion = SMJS_API_VERSION;

	isolate = mainIsolate; //v8::Isolate::New();

	HandleScope handle_scope(isolate);

	v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();

	global->Set(v8::String::New("print"), v8::FunctionTemplate::New(JSN_Print));
	global->Set(v8::String::New("__require"), v8::FunctionTemplate::New(JSN_Require));
	
	context = v8::Persistent<v8::Context>::New(isolate, v8::Context::New(isolate, NULL, global));
	context->SetEmbedderData(1, v8::External::New(this));

	if(isSandboxed){
		context->AllowCodeGenerationFromStrings(false);
		context->SetErrorMessageForCodeGenerationFromStrings(v8::String::New("Sandboxed scripts cannot generate code from strings"));
	}
}

void SMJS_Plugin::LoadModules(){
	HandleScope handle_scope(isolate);
	Context::Scope context_scope(context);
	
	for(auto it = modules.begin(); it != modules.end(); ++it) {
		SMJS_Module *module = (*it);
		if(!module->sandboxed && isSandboxed) continue;

		loadedModules.push_back(module);
		context->Global()->Set(v8::String::New(module->identifier.c_str()), module->GetWrapper(this), ReadOnly);
	}
	
	v8::Script::New(v8::String::New(scriptSMStr), &v8::ScriptOrigin(v8::String::New("sm.js")), scriptSMData, v8::String::New(dir.c_str()))->Run();
	v8::Script::New(v8::String::New(scriptDotaStr), &v8::ScriptOrigin(v8::String::New("dota.js")), scriptDotaData, v8::String::New(dir.c_str()))->Run();
}

SMJS_Plugin::~SMJS_Plugin(){
	for(auto it = loadedModules.begin(); it != loadedModules.end(); ++it){
		(*it)->OnPluginDestroyed(this);
	}

	for(auto it = destroyCallbackFuncs.begin(); it != destroyCallbackFuncs.end(); ++it){
		(*it)(this);
	}

	for(auto it = destroyCallbackHandlers.begin(); it != destroyCallbackHandlers.end(); ++it){
		(*it)->OnPluginDestroyed(this);
	}

	//context.MakeWeak(isolate, NULL, NULL);
	context.Dispose();
	//v8::V8::TerminateExecution(isolate);
	//isolate->Dispose();
	plugins[id] = NULL;
}

void SMJS_Plugin::RegisterDestroyCallback(DestroyCallback func){
	destroyCallbackFuncs.push_back(func);
}

void SMJS_Plugin::RegisterDestroyCallback(IPluginDestroyedHandler *ptr){
	destroyCallbackHandlers.push_back(ptr);
}

std::vector<v8::Persistent<v8::Function>>* SMJS_Plugin::GetHooks(char const *type){
	std::string typeStd(type);
	auto it = hooks.find(typeStd);
	if(it != hooks.end()){
		return &it->second;
	}

	
	hooks.insert(std::make_pair(typeStd, std::vector<v8::Persistent<v8::Function>>()));

	auto vec = GetHooks(type);

	if(apiVersion < 5){
		HandleScope handle_scope(isolate);
		Context::Scope context_scope(context);
		auto g = context->Global()->Get(v8::String::New(type));
		if(!g.IsEmpty() && g->IsFunction()){
			vec->push_back(v8::Persistent<v8::Function>::New(v8::Local<v8::Function>::Cast(g)));
		}
	}

	return vec;
}

std::vector<v8::Persistent<v8::Function>>* SMJS_Plugin::GetEventHooks(char const *type){
	std::string typeStd(type);
	auto it = eventHooks.find(typeStd);
	if(it != eventHooks.end()){
		return &it->second;
	}

	
	eventHooks.insert(std::make_pair(typeStd, std::vector<v8::Persistent<v8::Function>>()));
	return GetEventHooks(type);
}

std::vector<v8::Persistent<v8::Function>>* SMJS_Plugin::GetEventPostHooks(char const *type){
	std::string typeStd(type);
	auto it = eventPostHooks.find(typeStd);
	if(it != eventPostHooks.end()){
		return &it->second;
	}

	
	eventPostHooks.insert(std::make_pair(typeStd, std::vector<v8::Persistent<v8::Function>>()));
	return GetEventHooks(type);
}

bool SMJS_Plugin::RunString(const char* name, const char *source, bool asGlobal, v8::Handle<v8::Value> *result){
	HandleScope handle_scope(isolate);
	Context::Scope context_scope(context);

	TryCatch try_catch;
	v8::ScriptOrigin origin(String::New(name), v8::Integer::New(0), v8::Integer::New(0));
	Handle<Script> script;
	if(asGlobal){
		script = Script::Compile(v8::String::New(source), &origin, NULL, v8::String::New(dir.c_str()));
	}else{
		char *buffer = new char[strlen(source) + 200];
		strcpy(buffer,
		"(function(){\
			var exports = {};\
			(function(exports){\
		");

		strcat(buffer, source);

		strcat(buffer,
			"})(exports);\
			return exports;\
		})();");

		script = Script::Compile(v8::String::New(buffer), &origin, NULL, v8::String::New(dir.c_str()));
		delete buffer;
	}


	if(script.IsEmpty()) {
		// Print errors that happened during compilation.
		ReportException(&try_catch);
		return false;
	} else {
		v8::Handle<v8::Value> res = script->Run();
		if (res.IsEmpty()) {
			ReportException(&try_catch);
			return false;
		}
		
		if(result != NULL) *result = handle_scope.Close(res);

		return true;
	}
}

void SMJS_Plugin::ReportException(TryCatch* try_catch){
	HandleScope handle_scope(isolate);

	fprintf(stderr, "----------------------------- JAVASCRIPT EXCEPTION -----------------------------");

	Handle<Message> message = try_catch->Message();
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
				v8::String::Utf8Value scriptName(frame->GetScriptName());
				v8::String::Utf8Value funName(frame->GetFunctionName());
				
				snprintf(buffer, sizeof(buffer), "%s%s - %s @ line %d\n", buffer, *scriptName, *funName, frame->GetLineNumber());
			}
		}
	}

	fprintf(stderr, "%s", buffer);
	fprintf(stderr, "--------------------------------------------------------------------------------\n");

	if(MPlugin::masterPlugin != -1){
		auto master = GetPlugin(MPlugin::masterPlugin);
		auto scriptData = message->GetScriptData();

		if(master != NULL && master != this){
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

bool SMJS_Plugin::LoadFile(const char* file, bool asGlobal, v8::Handle<v8::Value> *result){
	char path[512];
#if WIN32
	snprintf(path, sizeof(path), "%s\\%s", GetPath(), file);
#else
	snprintf(path, sizeof(path), "%s/%s", GetPath(), file);
#endif
	
	FILE* fileHandle = fopen(path, "rb");
	if(fileHandle == NULL){
		// Fall back to read the .js file from the lib folder
		char buffer[512];
		smutils->BuildPath(Path_SM, buffer, sizeof(buffer), "/sourcemod.js/lib/%s.js", file);
		fileHandle = fopen(buffer, "rb");
		if(fileHandle == NULL) return false;
	}

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

	char filenameBuffer[512];
	snprintf(filenameBuffer, sizeof(filenameBuffer), "%s/%s", dir.c_str(), file);

	bool res = RunString(filenameBuffer, source, asGlobal, result);
	delete[] source;
	return res;
}

void SMJS_Plugin::CheckApi(){
	char *file = "API_VERSION";
	char path[PLATFORM_MAX_PATH];
#if WIN32
	snprintf(path, sizeof(path), "%s\\%s", GetPath(), file);
#else
	snprintf(path, sizeof(path), "%s/%s", GetPath(), file);
#endif
	FILE* fileHandle = fopen(path, "r");
	if(fileHandle == NULL){
		fileHandle = fopen(path, "w+");
		if(fileHandle == NULL) return;

		char str[8];
		snprintf(str, sizeof(str), "%d", apiVersion);
		fwrite(str, sizeof(str[0]), strlen(str), fileHandle);
		fclose(fileHandle);
		return;
	}

	char str[8];
	fread(str, sizeof(str[0]), sizeof(str) / sizeof(str[0]), fileHandle);
	fclose(fileHandle);
	apiVersion = atoi(str);
	if(apiVersion == 0) apiVersion = SMJS_API_VERSION;
}

void JSN_Print(const FunctionCallbackInfo<Value>& args){
	HandleScope handle_scope(args.GetIsolate());

	bool first = true;
	for (int i = 0; i < args.Length(); i++) {
		if (first) {
			first = false;
		} else {
			printf(" ");
		}
		String::Utf8Value str(args[i]);
		const char* cstr = ToCString(str);
		printf("%s", cstr);
	}
	printf("\n");
	fflush(stdout);
	RETURN_UNDEF;
}

FUNCTION(JSN_Require)
	ARG_COUNT(1);
	ARG_str(filename, 0);

	if(strstr(*filename, "../") != NULL || strstr(*filename, "..\\") != NULL){
		THROW("Invalid file path");
	}

	SMJS_Plugin *plugin = GetPluginRunning();

	v8::Handle<v8::Value> res;

	if(!plugin->LoadFile(*filename, false, &res)){
		THROW("Failed to load file");
	}

	RETURN_SCOPED(res);
END