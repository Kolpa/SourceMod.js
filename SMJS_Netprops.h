#ifndef _INCLUDE_SMJS_NETPROPS_H_
#define _INCLUDE_SMJS_NETPROPS_H_

#include "SMJS.h"
#include "smsdk_ext.h"
#include <map>


#include "SMJS_BaseWrapped.h"
#include "SMJS_SimpleWrapped.h"
#include "dt_send.h"
#include "server_class.h"

#include "boost/function.hpp"
#include "boost/bind.hpp"


class SMJS_Entity;
class SendProp;
class SMJS_DataTable;
class SMJS_GameRulesProps;

class SMJS_NPValueCacher {
public:
	std::map<PLUGIN_ID, std::map<std::string, v8::Persistent<v8::Value>>> cachedValues;

protected:
	void InitCacheForPlugin(SMJS_Plugin *pl);
	inline void InsertCachedValue(PLUGIN_ID plId, std::string key, v8::Persistent<v8::Value> value);
	inline v8::Persistent<v8::Value> FindCachedValue(PLUGIN_ID plId, std::string key);
};

class SMJS_Netprops : public SMJS_BaseWrapped, public SMJS_NPValueCacher {
public:
	SMJS_Entity *entWrapper;

	std::map<PLUGIN_ID, std::map<std::string, v8::Persistent<v8::Value>>> cachedValues;

	SMJS_Netprops();
	virtual ~SMJS_Netprops();

	void OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper);

	
	WRAPPED_CLS(SMJS_Netprops, SMJS_BaseWrapped) {
		temp->SetClassName(v8::String::NewSymbol("Netprops"));
		temp->InstanceTemplate()->SetNamedPropertyHandler(SGetNetProp, SSetNetProp);
	}

	static bool GetEntityPropInfo(void *ent, const char *propName, sm_sendprop_info_t *propInfo);
	static bool GetClassPropInfo(const char *classname, const char *propName, sm_sendprop_info_t *propInfo);

protected:
	v8::Persistent<v8::Value> GenerateThenFindCachedValue(PLUGIN_ID plId, std::string key, void *ent, edict_t *edict, SendProp *p, size_t offset);

	static void SGetNetProp(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& args);
	static void SSetNetProp(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<Value>& args);

	static v8::Handle<v8::Value> SGetNetProp(void *ent, edict_t *edict, SendProp *p, size_t offset, bool *isCacheable = NULL, const char *name = NULL);
	static void SSetNetProp(void *ent, edict_t *edict, SendProp *p, size_t offset, v8::Local<v8::Value> value, boost::function<v8::Persistent<v8::Value> ()> getCache, const char *name = NULL);
	
	friend SMJS_NPValueCacher;
	friend SMJS_DataTable;
	friend SMJS_GameRulesProps;
};

class SMJS_DataTable : public SMJS_SimpleWrapped {
private:
	void *ent;
	edict_t *edict;
	SendTable *pTable;
	size_t offset;
	std::map<uint32_t, v8::Persistent<v8::Value>> cachedValues;

	static void DTGetter(uint32_t index, const PropertyCallbackInfo<Value>& args);
	static void DTSetter(uint32_t index, Local<Value> value, const PropertyCallbackInfo<Value>& args);

	SMJS_DataTable() : SMJS_SimpleWrapped(NULL) {};

	v8::Persistent<v8::Value> GenerateThenFindCachedValue(uint32_t key, SendProp *p, size_t offset){
		bool isCacheable;
		auto res = v8::Persistent<v8::Value>::New(SMJS_Netprops::SGetNetProp(ent, edict, p, offset, &isCacheable));

		if(isCacheable){
			cachedValues.insert(std::make_pair(key, res));
		}

		return res;
	}

public:


	SMJS_DataTable(SMJS_Plugin *pl, void *_ent, edict_t *_edict, SendTable *_pTable, size_t _offset) : SMJS_SimpleWrapped(pl), pTable(_pTable), offset(_offset){
		ent = _ent;
		edict = _edict;
	}

	SIMPLE_WRAPPED_CLS(SMJS_DataTable, SMJS_SimpleWrapped){
		temp->SetClassName(v8::String::NewSymbol("DataTable"));
		// temp->InstanceTemplate()->Set(v8::String::New("length"), v8::Int32::New(pTable->GetNumProps()), v8::ReadOnly);
		temp->InstanceTemplate()->SetIndexedPropertyHandler(DTGetter, DTSetter, NULL, NULL, NULL);
	}

	void OnWrapperAttached(){
		
	}
};


#endif
