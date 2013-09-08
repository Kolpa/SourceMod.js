#include "modules/MServer.h"
#include "SMJS_Plugin.h"
#include "MClient.h"

WRAPPED_CLS_CPP(MServer, SMJS_Module);

FUNCTION_M(MServer::print)
	ARG_COUNT(1);
	PSTR(str);

	int len = str.length();
	char *buffer = new char[len + 2];
	memcpy(buffer, *str, len);
	buffer[len] = '\n';
	buffer[len + 1] = '\0';

	META_CONPRINT(buffer);
	RETURN_UNDEF;
END

FUNCTION_M(MServer::command)
	if(GetPluginRunning()->IsSandboxed()) THROW("Function not available in sandboxed plugins");

	ARG_COUNT(1);
	PSTR(str);

	int len = str.length();
	char *buffer = new char[len + 2];
	memcpy(buffer, *str, len);
	buffer[len] = '\n';
	buffer[len + 1] = '\0';

	engine->ServerCommand(buffer);
	RETURN_UNDEF;
END

FUNCTION_M(MServer::execute)
	if(GetPluginRunning()->IsSandboxed()) THROW("Function not available in sandboxed plugins");
	
	ARG_COUNT(0);
	engine->ServerExecute();
	RETURN_UNDEF;
END

FUNCTION_M(MServer::getPort)
	ARG_COUNT(0);
	auto cvar = icvar->FindVar("hostport");
	if(cvar == NULL) RETURN_UNDEFINED;
	RETURN_INT(cvar->GetInt());
END

FUNCTION_M(MServer::getIP)
	ARG_COUNT(0);
	auto cvar = icvar->FindVar("ip");
	if(cvar == NULL) RETURN_UNDEFINED;
	RETURN_STRING(cvar->GetString());
END

FUNCTION_M(MServer::userIdToClient)
	PINT(userid);
	int client = playerhelpers->GetClientOfUserId(userid);
	if(client <= 0) RETURN_NULL;
	if(clients[client] == NULL) RETURN_NULL;
	RETURN_SCOPED(clients[client]->GetWrapper(GetPluginRunning()));
END

FUNCTION_M(MServer::changeMap)
	PSTR(mapName);
	PSTR(reason);

	if(!gamehelpers->IsMapValid(*mapName)) THROW_VERB("Invalid map \"%s\"", *mapName);

	// Workaround
	char buffer[512];
	snprintf(buffer, sizeof(buffer), "map \"%s\"\n", *mapName);
	engine->ServerCommand(buffer);
	engine->ServerExecute();

	//engine->ChangeLevel(*mapName, *reason);

	RETURN_UNDEF;
END

FUNCTION_M(MServer::getMap)
	RETURN_SCOPED(v8::String::New(gamehelpers->GetCurrentMap()));
END

FUNCTION_M(MServer::isMapValid)
	PSTR(mapName);
	RETURN_SCOPED(v8::Boolean::New(gamehelpers->IsMapValid(*mapName)));
END

FUNCTION_M(MServer::log)
	PSTR(str);
	smutils->LogMessage(myself, "%s", str);
	RETURN_UNDEF;
END

class ClientArray : public SMJS_BaseWrapped {
public:

	ClientArray(){
		
	}

	WRAPPED_CLS(ClientArray, SMJS_BaseWrapped) {
		temp->SetClassName(v8::String::NewSymbol("ClientArray"));
		proto->Set("length", v8::Int32::New(0));
		temp->InstanceTemplate()->SetIndexedPropertyHandler(GetClient);
	}

	virtual void OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper){
		if(GetPluginRunning()->GetApiVersion() < 3){
			wrapper->ToObject()->Set(v8::String::NewSymbol("length"), v8::Int32::New(MAXCLIENTS + 1), v8::ReadOnly);
		}else{
			wrapper->ToObject()->Set(v8::String::NewSymbol("length"), v8::Int32::New(MAXCLIENTS), v8::ReadOnly);
		}
	}

	static void GetClient(uint32_t index, const PropertyCallbackInfo<Value>& args){
		if(index >= MAXCLIENTS) RETURN_UNDEF;

		if(GetPluginRunning()->GetApiVersion() < 3){
			if(index == 0) RETURN_NULL;
			if(index >= MAXCLIENTS) RETURN_NULL;
			if(clients[index] == NULL) RETURN_NULL;
			RETURN(clients[index]->GetWrapper(GetPluginRunning()));
		}else{
			if(clients[index + 1] == NULL) RETURN_NULL;
			if(index + 1 >= MAXCLIENTS) RETURN_NULL;
			RETURN(clients[index + 1]->GetWrapper(GetPluginRunning()));
		}
	}
};

WRAPPED_CLS_CPP(ClientArray, SMJS_BaseWrapped);

ClientArray clientArray;

void MServer::OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper){
	wrapper->ToObject()->Set(v8::String::New("clients"), clientArray.GetWrapper(plugin));
}