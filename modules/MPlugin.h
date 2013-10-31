#pragma once

#include "SMJS.h"
#include "SMJS_Module.h"
#include <vector>
#include <map>

struct InterfaceAddedCallback {
	PLUGIN_ID pl;
	std::string dir;
	v8::Persistent<v8::Function> callback;
};

class MPlugin : public SMJS_Module {
public:
	MPlugin();

	static PLUGIN_ID masterPlugin;

	std::vector<v8::Persistent<v8::Object>> pluginInterfaces;
	std::vector<InterfaceAddedCallback> interfaceAddedCallbacks;



	void OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper);
	void OnPluginDestroyed(SMJS_Plugin *plugin);
	void Init();

	FUNCTION_DECL(isSandboxed);
	FUNCTION_DECL(loadPlugin);
	FUNCTION_DECL(exists);
	FUNCTION_DECL(expose);
	FUNCTION_DECL(get);
	FUNCTION_DECL(getApiVersion);
	FUNCTION_DECL(setMasterPlugin);
	FUNCTION_DECL(getCallerIdentifier);

	WRAPPED_CLS(MPlugin, SMJS_Module) {
		temp->SetClassName(v8::String::NewSymbol("PluginModule"));

		WRAPPED_FUNC(isSandboxed);
		WRAPPED_FUNC(loadPlugin);
		WRAPPED_FUNC(exists);
		WRAPPED_FUNC(expose);
		WRAPPED_FUNC(get);
		WRAPPED_FUNC(getApiVersion);
		WRAPPED_FUNC(setMasterPlugin);
		WRAPPED_FUNC(getCallerIdentifier);
	}
};
