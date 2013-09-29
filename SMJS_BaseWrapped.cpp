#include "extension.h"
#include "SMJS_BaseWrapped.h"
#include "SMJS_Plugin.h"

void DecWrappedRefCount(Isolate* isolate, v8::Persistent<v8::Value> *object, SMJS_BaseWrapped *parameter){
	SMJS_BaseWrapped *wrapped = (SMJS_BaseWrapped*) parameter;
	wrapped->DecRef();

	object->Dispose();
    object->Clear();
}

v8::Persistent<v8::Value> SMJS_BaseWrapped::GetWrapper(SMJS_Plugin *plugin){
	return v8::Persistent<v8::Value>::New(plugin->GetIsolate(), v8::Undefined());
}

void SMJS_BaseWrapped::OnPluginDestroyed(SMJS_Plugin *plugin){
	DecRef();
	wrappers.erase(plugin->id);
}

void SMJS_BaseWrapped::Destroy(){
	if(refCount == 0){
		delete this;
	}else{
		destroying = true;
		for(auto it = wrappers.begin(); it != wrappers.end(); ++it){
			it->second.MakeWeak<v8::Value, SMJS_BaseWrapped>(this, DecWrappedRefCount);
		}
	}
}

std::unordered_map<PLUGIN_ID, v8::Persistent<FunctionTemplate>> SMJS_BaseWrapped::templates;

v8::Persistent<v8::FunctionTemplate> SMJS_BaseWrapped::GetTemplateForPlugin(SMJS_Plugin *plugin, bool cache){
	auto it = templates.find(plugin->id);
	if(it != templates.end()) return it->second;
		
	auto temp = Persistent<FunctionTemplate>::New(FunctionTemplate::New());;
		
	auto inst = temp->InstanceTemplate();
	inst->SetInternalFieldCount(1);

	auto proto = Persistent<Template>::New(temp->PrototypeTemplate());
		
	if(cache) SetupTemplate(temp, proto);
		
	templates.insert(std::pair<PLUGIN_ID, v8::Persistent<v8::FunctionTemplate>>(plugin->id, temp));
	return temp;
}