#ifndef _INCLUDE_SMJS_GAMERULESPROPS_H_
#define _INCLUDE_SMJS_GAMERULESPROPS_H_

#include "SMJS.h"
#include "SMJS_BaseWrapped.h"
#include "SMJS_Netprops.h"

class SMJS_GameRulesProps : public SMJS_BaseWrapped, public SMJS_NPValueCacher {
public:
	void *gamerules;
	void *proxy;

	SMJS_GameRulesProps();

	WRAPPED_CLS(SMJS_GameRulesProps, SMJS_BaseWrapped){
		temp->InstanceTemplate()->SetNamedPropertyHandler(GetRulesProp, SetRulesProp);
	}

	v8::Persistent<v8::Value> GenerateThenFindCachedValue(PLUGIN_ID plId, std::string key, SendProp *p, size_t offset){
		bool isCacheable;
		auto res = v8::Persistent<v8::Value>::New(SMJS_Netprops::SGetNetProp(gamerules, NULL, p, offset, &isCacheable));

		if(isCacheable){
			InsertCachedValue(plId, key, res);
		}

		return res;
	}

	void OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper);

	static void GetRulesProp(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& args);
	static void SetRulesProp(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<Value>& args);
};

#endif