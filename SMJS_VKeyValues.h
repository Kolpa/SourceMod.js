#ifndef _INCLUDE_SMJS_KEYVALUES_H_
#define _INCLUDE_SMJS_KEYVALUES_H_

#include "SMJS.h"
#include "SMJS_BaseWrapped.h"
#include "SMJS_Interfaces.h"
#include "KeyValues.h"
#include <unordered_map>

class KeyValues2 : public KeyValues {
public:
	void SetString2( const char *keyName, const char *value ){
		KeyValues *dat = FindKey( keyName, true );

		if ( dat ){
			dat->m_wsValue = NULL;

			if (!value){
				// ensure a valid value
				value = "";
			}

			// allocate memory for the new value and copy it in
			int len = Q_strlen( value );
			dat->m_sValue = new char[len + 1];
			Q_memcpy( dat->m_sValue, value, len+1 );

			dat->m_iDataType = TYPE_STRING;
		}
	}
};

struct VKeyValuesRestore {
	union {
		int asInt;
		float asFloat;
		const char *asString;
	};
	char m_iDataType;
};

class SMJS_VKeyValues : public SMJS_BaseWrapped {
public:
	~SMJS_VKeyValues();

	KeyValues2 *kv;
	SMJS_VKeyValues(KeyValues2 *kv) : kv(kv){};
	std::unordered_map<std::string, VKeyValuesRestore*> restoreValues;

	void Restore();

	static void GetKeyValue(Local<String> property, const PropertyCallbackInfo<Value>& args);
	static void SetKeyValue(Local<String> property, Local<Value> value, const PropertyCallbackInfo<Value>& args);

	WRAPPED_CLS(SMJS_VKeyValues, SMJS_BaseWrapped) {
		temp->SetClassName(v8::String::NewSymbol("VKeyValues"));
		temp->InstanceTemplate()->SetNamedPropertyHandler(GetKeyValue, SetKeyValue);
	}
};

#endif