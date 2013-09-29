#include "MPlugin.h"
#include "SMJS_Plugin.h"
#include "extension.h"

WRAPPED_CLS_CPP(MPlugin, SMJS_Module);

MPlugin* g_MPlugin;

PLUGIN_ID MPlugin::masterPlugin = -1;

MPlugin::MPlugin(){
	identifier = "plugin";
	g_MPlugin = this;
}

void MPlugin::OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper){
	auto obj = wrapper->ToObject();
	
}

void MPlugin::OnPluginDestroyed(SMJS_Plugin *plugin){
	if (pluginInterfaces.size() > (size_t) plugin->id){
		pluginInterfaces[plugin->id] = v8::Persistent<v8::Object>();
	}

	for(auto it = interfaceAddedCallbacks.begin(); it != interfaceAddedCallbacks.end(); it != interfaceAddedCallbacks.end() ? ++it : interfaceAddedCallbacks.end()){
		if(it->dir != plugin->GetDir()) continue;
		it = interfaceAddedCallbacks.erase(it);
	}
}


FUNCTION_M(MPlugin::isSandboxed)
	RETURN_SCOPED(v8::Boolean::New(GetPluginRunning()->IsSandboxed()));
END

	
FUNCTION_M(MPlugin::loadPlugin)
	if(GetPluginRunning()->IsSandboxed()) THROW("This function is not allowed to be called in sandboxed plugins");
	
	PSTR(dir);
	RETURN_SCOPED(v8::Boolean::New(LoadPlugin(*dir) != NULL));
END


FUNCTION_M(MPlugin::exists)
	PSTR(dir);
	RETURN_SCOPED(v8::Boolean::New(SMJS_Plugin::GetPluginByDir(*dir) != NULL));
END

FUNCTION_M(MPlugin::expose)
	POBJ(obj);

	auto plugin = GetPluginRunning();

	if (g_MPlugin->pluginInterfaces.size() <= (size_t) plugin->id)  g_MPlugin->pluginInterfaces.resize(plugin->id + 1);
	g_MPlugin->pluginInterfaces[plugin->id] = v8::Persistent<v8::Object>::New(obj);
	
	for(auto it = g_MPlugin->interfaceAddedCallbacks.begin(); it != g_MPlugin->interfaceAddedCallbacks.end(); ++it){
		if(it->dir != plugin->GetDir()) continue;

		auto cur = GetPlugin(it->pl);
		if(cur == NULL) continue;

		HandleScope handle_scope(cur->GetIsolate());
		Context::Scope context_scope(cur->GetContext());

		v8::Handle<v8::Value> args[1];
		args[0] = obj;

		it->callback->Call(cur->GetContext()->Global(), 1, args);
	}

	RETURN_UNDEF;
END

FUNCTION_M(MPlugin::get)
	PSTR(pl);
	PFUN(callback);
	
	auto plugin = GetPluginRunning();
	auto f = SMJS_Plugin::GetPluginByDir(*pl);

	bool hasInterface = false;

	if(f != NULL){
		if(g_MPlugin->pluginInterfaces.size() > (size_t) f->id){
			hasInterface = !g_MPlugin->pluginInterfaces[f->id].IsEmpty();
		}
	}

	if(f == NULL || !hasInterface){
		InterfaceAddedCallback st;
		st.pl = plugin->id;
		st.dir = std::string(*pl);
		st.callback = v8::Persistent<v8::Function>::New(callback);
		g_MPlugin->interfaceAddedCallbacks.push_back(st);
	}else{
		v8::Handle<v8::Value> args[1];
		args[0] = g_MPlugin->pluginInterfaces[f->id];
		callback->Call(plugin->GetContext()->Global(), 1, args);
	}

	RETURN_UNDEF;
END

FUNCTION_M(MPlugin::getApiVersion)
	RETURN_SCOPED(v8::Int32::New(GetPluginRunning()->GetApiVersion()));
END

FUNCTION_M(MPlugin::setMasterPlugin)
	if(GetPluginRunning()->IsSandboxed()) THROW("This function is not allowed to be called in sandboxed plugins");
	masterPlugin = GetPluginRunning()->GetId();

	RETURN_UNDEF;
END