#include "SMJS_VKeyValues.h"
#include "SMJS_Plugin.h"

WRAPPED_CLS_CPP(SMJS_VKeyValues, SMJS_BaseWrapped);

SMJS_VKeyValues::~SMJS_VKeyValues(){
	for(auto it = restoreValues.begin(); it != restoreValues.end(); ++it){
		delete it->second;
	}
}

void SMJS_VKeyValues::Restore(){
	for(auto it = restoreValues.begin(); it != restoreValues.end(); ++it){
		if(it->second == NULL){
			kv->RemoveSubKey(kv->FindKey(it->first.c_str(), false));
		}else{
			switch(it->second->m_iDataType){
				case KeyValues::TYPE_STRING:
					kv->SetString2(it->first.c_str(), it->second->asString);
				break;
				case KeyValues::TYPE_INT:
					kv->SetInt(it->first.c_str(), it->second->asInt);
				break;
				case KeyValues::TYPE_FLOAT:
					kv->SetInt(it->first.c_str(), it->second->asFloat);
				break;
			}
		}
	}
}

 void SMJS_VKeyValues::GetKeyValue(Local<String> prop, const PropertyCallbackInfo<Value>& args){
	SMJS_VKeyValues *self = dynamic_cast<SMJS_VKeyValues*>((SMJS_BaseWrapped*)Handle<External>::Cast(args.This()->GetInternalField(0))->Value());
	if(self->kv == NULL) THROW("Invalid keyvalue object");

	v8::String::Utf8Value str(prop);
	switch(self->kv->GetDataType(*str)){
		case KeyValues::TYPE_NONE: RETURN_UNDEF;
		case KeyValues::TYPE_STRING: RETURN_STRING(self->kv->GetString(*str));
		case KeyValues::TYPE_INT: RETURN_INT(self->kv->GetInt(*str));
		case KeyValues::TYPE_FLOAT: RETURN_NUMBER(self->kv->GetFloat(*str));
		default: THROW_VERB("Unknown data type %d", self->kv->GetDataType(*str));
	}
}

void SMJS_VKeyValues::SetKeyValue(Local<String> prop, Local<Value> value, const PropertyCallbackInfo<Value>& args){
	SMJS_VKeyValues *self = dynamic_cast<SMJS_VKeyValues*>((SMJS_BaseWrapped*)Handle<External>::Cast(args.This()->GetInternalField(0))->Value());
	if(self->kv == NULL) THROW("Invalid keyvalue object");
	
	v8::String::Utf8Value str(prop);
	std::string propStdName(*str);

	switch(self->kv->GetDataType(*str)){
		case KeyValues::TYPE_STRING:
			{
				auto it = self->restoreValues.find(propStdName);
				if(it == self->restoreValues.end()){
					auto r = new VKeyValuesRestore();
					r->asString = self->kv->GetString(*str);
					r->m_iDataType = KeyValues::TYPE_STRING;
					self->restoreValues.insert(std::make_pair(propStdName, r));
				}

				v8::String::Utf8Value vstr(value);
				self->kv->SetString2(*str, *vstr);
			}
		break;
		case KeyValues::TYPE_INT:
			{
				auto it = self->restoreValues.find(propStdName);
					if(it == self->restoreValues.end()){
						auto r = new VKeyValuesRestore();
						r->asInt = self->kv->GetInt(*str);
						r->m_iDataType = KeyValues::TYPE_INT;
						self->restoreValues.insert(std::make_pair(propStdName, r));
					}

				self->kv->SetInt(*str, (int) value->NumberValue());
			}
		break;
		case KeyValues::TYPE_FLOAT:
			{
				auto it = self->restoreValues.find(propStdName);
				if(it == self->restoreValues.end()){
						auto r = new VKeyValuesRestore();
						r->asFloat = self->kv->GetFloat(*str);
						r->m_iDataType = KeyValues::TYPE_FLOAT;
						self->restoreValues.insert(std::make_pair(propStdName, r));
					}

				self->kv->SetFloat(*str, value->NumberValue());
			}
		break;

		case KeyValues::TYPE_NONE:
			{
				auto it = self->restoreValues.find(propStdName);
				if(it == self->restoreValues.end()){
					self->restoreValues.insert(std::make_pair(propStdName, (VKeyValuesRestore*) NULL));
				}
				v8::String::Utf8Value vstr(value);
				self->kv->SetString2(*str, *vstr);
			}
		break;
		default: THROW_VERB("Unknown data type %d", self->kv->GetDataType(*str));
	}

	RETURN(value);
}