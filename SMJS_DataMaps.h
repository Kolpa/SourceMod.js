#ifndef _INCLUDE_SMJS_DATAMAPS_H_
#define _INCLUDE_SMJS_DATAMAPS_H_

#include "SMJS.h"
#include "smsdk_ext.h"
#include <map>

#include "SMJS_BaseWrapped.h"
#include "SMJS_SimpleWrapped.h"
#include "dt_send.h"
#include "server_class.h"

class SMJS_Entity;
class SMJS_DataMaps : public SMJS_BaseWrapped {
public:
	SMJS_Entity *entWrapper;


	WRAPPED_CLS(SMJS_DataMaps, SMJS_BaseWrapped) {
		temp->SetClassName(v8::String::NewSymbol("DataMaps"));
		temp->InstanceTemplate()->SetNamedPropertyHandler(SGetDataMap, SSetDataMap);
	}

	static void SGetDataMap(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& args);
	static void SSetDataMap(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<Value>& args);
};

#endif