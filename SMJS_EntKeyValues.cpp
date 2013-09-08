#include "SMJS_EntKeyValues.h"
#include "SMJS_Entity.h"
#include "SMJS_Plugin.h"
#include "modules/MEntities.h"
#include "SMJS_Interfaces.h"

#include <conio.h>

WRAPPED_CLS_CPP(SMJS_EntKeyValues, SMJS_BaseWrapped)

SMJS_EntKeyValues::SMJS_EntKeyValues(){
	
}

SMJS_EntKeyValues::~SMJS_EntKeyValues(){
	
}


void SMJS_EntKeyValues::OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper){
	SMJS_BaseWrapped::OnWrapperAttached(plugin, wrapper);

}

void SMJS_EntKeyValues::SetKeyValue(v8::Local<v8::String> prop, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<Value>& args){
	Local<Value> _intfld = args.This()->GetInternalField(0); 
	SMJS_EntKeyValues *self = dynamic_cast<SMJS_EntKeyValues*>((SMJS_BaseWrapped*)Handle<External>::Cast(_intfld)->Value());

	v8::String::AsciiValue propStr(prop);

	if(value->IsString()){
		v8::String::AsciiValue valueStr(value->ToString());
		serverTools->SetKeyValue(self->entWrapper->ent, *propStr, *valueStr);
		RETURN(value);
	}else if(value->IsNumber()){
		serverTools->SetKeyValue(self->entWrapper->ent, *propStr, (float) value->NumberValue());
		RETURN(value);
	}

	THROW("Entity KeyValues can only be strings or floats, for other types, vector for example, use: \"0.0 0.0 0.0\"");
}