#include "SMJS_Netprops.h"
#include "SMJS_Entity.h"
#include "SMJS_Plugin.h"
#include "k7v8macros.h"
#include "modules/MEntities.h"
#include "boost/bind.hpp"

WRAPPED_CLS_CPP(SMJS_Netprops, SMJS_BaseWrapped)

struct SMJS_Netprops_CachedValueData {
	void *ent;
	void *addr;
	edict_t *edict;
	size_t actual_offset;
	SendProp *prop;
};

void DestroyDataCallback(Isolate* isolate, v8::Persistent<v8::Value> *object, SMJS_Netprops_CachedValueData *parameter){
	delete parameter;
}


void SMJS_NPValueCacher::InsertCachedValue(PLUGIN_ID plId, std::string key, v8::Persistent<v8::Value> value){
	auto &vec = cachedValues.find(plId)->second;
	vec.insert(std::make_pair(key, value));
}

v8::Persistent<v8::Value> SMJS_NPValueCacher::FindCachedValue(PLUGIN_ID plId, std::string key){
	auto &vec = cachedValues.find(plId)->second;
	auto it = vec.find(key);
	if(it != vec.end()){
		return it->second;
	}

	return v8::Persistent<v8::Value>();
}
void SMJS_NPValueCacher::InitCacheForPlugin(SMJS_Plugin *pl){
	cachedValues.insert(std::make_pair(pl->id, std::map<std::string, v8::Persistent<v8::Value>>()));
}

SMJS_Netprops::SMJS_Netprops() {
	
}

SMJS_Netprops::~SMJS_Netprops(){
	for(auto plIt = cachedValues.begin(); plIt != cachedValues.end(); ++plIt){
		auto pl = GetPlugin(plIt->first);
		for(auto it = plIt->second.begin(); it != plIt->second.end(); ++it){
			auto obj = v8::Handle<v8::Object>::Cast(it->second);
			if(obj.IsEmpty()) continue;
			auto hiddenValue = obj->GetHiddenValue(v8::String::New("SMJS::dataPtr"));
			if(hiddenValue.IsEmpty() || hiddenValue->IsUndefined() || hiddenValue->IsNull()) continue;
			SMJS_Netprops_CachedValueData *data = (SMJS_Netprops_CachedValueData *) v8::Handle<External>::Cast(hiddenValue)->Value();

			it->second.MakeWeak<v8::Value, SMJS_Netprops_CachedValueData>(data, DestroyDataCallback);
		}
	}
}


void SMJS_Netprops::OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper){
	SMJS_BaseWrapped::OnWrapperAttached(plugin, wrapper);
	InitCacheForPlugin(plugin);
	cachedValues.insert(std::make_pair(plugin->id, std::map<std::string, v8::Persistent<v8::Value>>()));
}

void VectorGetter(Local<String> prop, const PropertyCallbackInfo<Value>& args){
	SMJS_Netprops_CachedValueData *data = (SMJS_Netprops_CachedValueData *) v8::Handle<External>::Cast(args.This()->GetHiddenValue(v8::String::New("SMJS::dataPtr")))->Value();
	
	Vector *vec = (Vector*)data->addr;
	if(prop == v8::String::New("x")){
		RETURN_NUMBER(vec->x);
	}else if(prop == v8::String::New("y")){
		RETURN_NUMBER(vec->y);
	}else if(prop == v8::String::New("z")){
		RETURN_NUMBER(vec->z);
	}

	RETURN_UNDEFINED;
}

void VectorSetter(Local<String> prop, Local<Value> value, const PropertyCallbackInfo<void>& args){
	SMJS_Netprops_CachedValueData *data = (SMJS_Netprops_CachedValueData *) v8::Handle<External>::Cast(args.This()->GetHiddenValue(v8::String::New("SMJS::dataPtr")))->Value();
	
	Vector *vec = (Vector*)data->addr;
	if(prop == v8::String::New("x")){
		vec->x = value->NumberValue();
	}else if(prop == v8::String::New("y")){
		vec->y = value->NumberValue();
	}else if(prop == v8::String::New("z")){
		vec->z = value->NumberValue();
	}

	if(data->edict != NULL) gamehelpers->SetEdictStateChanged(data->edict, data->actual_offset);
}

void VectorToString(const v8::FunctionCallbackInfo<v8::Value>& args) {
	SMJS_Netprops_CachedValueData *data = (SMJS_Netprops_CachedValueData *) v8::Handle<External>::Cast(args.This()->GetHiddenValue(v8::String::New("SMJS::dataPtr")))->Value();
	Vector *vec = (Vector*)data->addr;

	char buffer[256];
	auto t = args.This();
	snprintf(buffer, sizeof(buffer), "[Vector %f %f %f]",
		vec->x,
		vec->y,
		vec->z
	);

	RETURN_STRING(buffer);
}

unsigned int strncopy(char *dest, const char *src, size_t count){
	if (!count){
		return 0;
	}

	char *start = dest;
	while ((*src) && (--count)){
		*dest++ = *src++;
	}
	*dest = '\0';

	return (dest - start);
}

v8::Persistent<v8::Value> SMJS_Netprops::GenerateThenFindCachedValue(PLUGIN_ID plId, std::string key, void *ent, edict_t *edict, SendProp *p, size_t offset){
	bool isCacheable;
	auto res = v8::Persistent<v8::Value>::New(SMJS_Netprops::SGetNetProp(ent, edict, p, offset, &isCacheable));

	if(isCacheable){
		InsertCachedValue(plId, key, res);
	}

	return res;
}

bool SMJS_Netprops::GetEntityPropInfo(void *ent, const char *propName, sm_sendprop_info_t *propInfo){
	IServerUnknown *pUnk = (IServerUnknown *)ent;
	IServerNetworkable *pNet = pUnk->GetNetworkable();
	if(!pNet) return false;
	
	return gamehelpers->FindSendPropInfo(pNet->GetServerClass()->GetName(), propName, propInfo);
}

bool SMJS_Netprops::GetClassPropInfo(const char *classname, const char *propName, sm_sendprop_info_t *propInfo){
	return gamehelpers->FindSendPropInfo(classname, propName, propInfo);
}

void SMJS_Netprops::SGetNetProp(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& args){
	SMJS_Netprops *self = (SMJS_Netprops*) Handle<External>::Cast(args.This()->GetInternalField(0))->Value();

	if(!self->entWrapper->IsValid()) RETURN_UNDEFINED;

	v8::String::AsciiValue str(property);
	
	std::string propNameStdString(*str);

	auto cachedValue = self->FindCachedValue(GetPluginRunning()->id, propNameStdString);
	if(!cachedValue.IsEmpty()) RETURN(cachedValue);

	sm_sendprop_info_t propInfo;
	
	IServerUnknown *pUnk = (IServerUnknown *)self->entWrapper->ent;
	IServerNetworkable *pNet = pUnk->GetNetworkable();
	if(!pNet){
		THROW("Entity is not networkable");
	}
	
	if(!gamehelpers->FindSendPropInfo(pNet->GetServerClass()->GetName(), *str, &propInfo)){
		RETURN_UNDEFINED;
	}

	bool isCacheable;
	auto ret = SGetNetProp(self->entWrapper->ent, self->entWrapper->edict, propInfo.prop, propInfo.actual_offset, &isCacheable, NULL);

	if(isCacheable){
		self->InsertCachedValue(GetPluginRunning()->id, propNameStdString, v8::Persistent<v8::Value>::New(ret));
	}

	RETURN(ret);
}


void SMJS_Netprops::SSetNetProp(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<Value>& args){
	SMJS_Netprops *self = (SMJS_Netprops*) Handle<External>::Cast(args.This()->GetInternalField(0))->Value();

	if(!self->entWrapper->IsValid()) RETURN_UNDEFINED;
	
	std::string propNameStdString(*v8::String::AsciiValue(property));
	sm_sendprop_info_t propInfo;
	
	IServerUnknown *pUnk = (IServerUnknown *)self->entWrapper->ent;
	IServerNetworkable *pNet = pUnk->GetNetworkable();
	if(!pNet){
		THROW("Entity is not networkable");
	}

	
	if(!gamehelpers->FindSendPropInfo(pNet->GetServerClass()->GetName(), *v8::String::AsciiValue(property), &propInfo)){
		RETURN_UNDEFINED;
	}

	boost::function<v8::Persistent<v8::Value> ()> f(boost::bind(&SMJS_Netprops::GenerateThenFindCachedValue, self, GetPluginRunning()->id, propNameStdString, self->entWrapper->ent, self->entWrapper->edict, propInfo.prop, propInfo.actual_offset));

	SSetNetProp(self->entWrapper->ent, self->entWrapper->edict, propInfo.prop, propInfo.actual_offset, value, f);
}

void SMJS_Netprops::SSetNetProp(void *ent, edict_t *edict, SendProp *p, size_t offset, v8::Local<v8::Value> value, boost::function<v8::Persistent<v8::Value> ()> getCache, const char *name){
	std::string propNameStdString(name != NULL ? name : p->GetName());

	int bit_count = p->m_nBits;
	bool is_unsigned = ((p->GetFlags() & SPROP_UNSIGNED) == SPROP_UNSIGNED);
	void *addr = (void*) ((intptr_t) ent + offset);

	auto type = p->GetType();

	// A workaround for some weird netprops
	if(bit_count == 0 && strlen(propNameStdString.c_str()) >= 5 && std::strncmp(propNameStdString.c_str(), "m_vec", 5) == 0){
		type = DPT_Vector;
	}

	switch(type){
	case DPT_Int:
	case DPT_Int64:
	{
		// If it's an integer with 21 bits and starts with m_h, it MUST be an entity, if it's not...
		//  oh well... return an integer
		if(bit_count == 21 && strlen(propNameStdString.c_str()) >= 3 && std::strncmp(propNameStdString.c_str(), "m_h", 3) == 0){
			CBaseHandle &hndl = *(CBaseHandle *)addr;
			if(value->IsNull() || value->IsUndefined()){
				hndl.Set(NULL);
			}else{
				auto obj = value->ToObject();
				if(!obj.IsEmpty()){
					Local<Value> _intfld = obj->GetInternalField(0);
					SMJS_Entity* ent = dynamic_cast<SMJS_Entity*>((SMJS_BaseWrapped*)Handle<External>::Cast(_intfld)->Value());
					if(ent == NULL){
						v8::ThrowException(v8::Exception::TypeError(v8::String::New("Entity field can only contain an entity")));
						return;
					}

					
					IHandleEntity *pHandleEnt = (IHandleEntity *)ent->ent;
					hndl.Set(pHandleEnt);
				}
			}
		}
		
		if (bit_count < 1){
			*(int32_t * )addr = value->Int32Value();
		}else if (bit_count > 32){
			if(!value->IsArray()) THROW("Value must be an array");
			auto obj = value->ToObject();

			if(obj->Get(v8::String::New("length"))->Uint32Value() != 2) {
				THROW("Value must be an array with 2 elements");
			}

			auto lowBits = obj->Get(0);
			auto highBits = obj->Get(1);
			
			if(!lowBits->IsNumber() || !highBits->IsNumber()) THROW("Value must be an array with 2 numeric elements");

			*(uint32_t * )addr = lowBits->Uint32Value();
			*(uint32_t * )((intptr_t) addr + 4) = highBits->Uint32Value();
		}else if (bit_count >= 17){
			*(int32_t * )addr = value->Int32Value();
		}else if (bit_count >= 9){
			if (is_unsigned){
				*(uint16_t * )addr = value->Int32Value();
			}else{
				*(int16_t * )addr = value->Int32Value();
			}
		}else if (bit_count >= 2){
			if (is_unsigned){
				*(uint8_t * )addr = value->Int32Value();
			}else{
				*(int8_t * )addr = value->Int32Value();
			}
		}else{
			*(bool *)addr = value->BooleanValue();
		}
	}; break;
	case DPT_Float:
		*(float*)addr = (float) value->NumberValue();
		break;
	case DPT_Vector: {
		auto obj = value->ToObject();
		if(obj.IsEmpty()){
			v8::ThrowException(v8::Exception::TypeError(v8::String::New("You can only set a vector field to a vector object")));
			return;
		}
		if(!obj->Has(v8::String::New("x")) || !obj->Has(v8::String::New("y")) || !obj->Has(v8::String::New("z"))){
			v8::ThrowException(v8::Exception::TypeError(v8::String::New("You can only set a vector field to a vector object that has the properties x, y and z")));
			return;
		}

		auto tmp = getCache();
		if(tmp.IsEmpty()){
			SGetNetProp(ent, edict, p, offset, NULL);
			tmp = getCache();
		}

		auto vec = tmp->ToObject();

		vec->Set(v8::String::New("x"), obj->Get(v8::String::New("x")));
		vec->Set(v8::String::New("y"), obj->Get(v8::String::New("y")));
		vec->Set(v8::String::New("z"), obj->Get(v8::String::New("z")));
	}; break;

	case DPT_String: {
		int maxlen = DT_MAX_STRING_BUFFERSIZE;
		v8::String::Utf8Value param1(value->ToString());

		char *LEAK_FIX_ME = new char[maxlen];
		
		strncopy(LEAK_FIX_ME, *param1, maxlen);
		*(char **)addr = LEAK_FIX_ME;
	}; break;

	default: THROW("Invalid Netprop Type");
	}

	if(edict != NULL) gamehelpers->SetEdictStateChanged(edict, offset);
}

v8::Handle<v8::Value> SMJS_Netprops::SGetNetProp(void *ent, edict_t *edict, SendProp *p, size_t offset, bool *isCacheable, const char *name){
	std::string propNameStdString(name != NULL ? name : p->GetName());
	
	int bit_count = p->m_nBits;
	bool is_unsigned = ((p->GetFlags() & SPROP_UNSIGNED) == SPROP_UNSIGNED);
	void *addr = (void*) ((intptr_t) ent + offset);

	if(isCacheable != NULL) *isCacheable = false;

	auto type = p->GetType();

	// A workaround for some weird netprops
	if(bit_count == 0 && strlen(propNameStdString.c_str()) >= 5 && std::strncmp(propNameStdString.c_str(), "m_vec", 5) == 0){
		type = DPT_Vector;
	}

	switch(type){
	case DPT_Int:
	case DPT_Int64:
	{
		int v;
		// If it's an integer with 21 bits and starts with m_h, it MUST be an entity, if it's not...
		//  oh well... return an integer
		if(bit_count == 21 && strlen(propNameStdString.c_str()) >= 3 && std::strncmp(propNameStdString.c_str(), "m_h", 3) == 0){
			CBaseHandle &hndl = *(CBaseHandle *)addr;
			CBaseEntity *pHandleEntity = gamehelpers->ReferenceToEntity(hndl.GetEntryIndex());
			
			if (pHandleEntity && hndl == reinterpret_cast<IHandleEntity *>(pHandleEntity)->GetRefEHandle()){
				SMJS_Entity *entity = GetEntityWrapper(pHandleEntity);
				return entity->GetWrapper(GetPluginRunning());
			}else{
				return v8::Null();
			}
		}
		
		if (bit_count < 1){
			v = *(int32_t * )addr;
		}else if(bit_count > 32){
			auto arr = v8::Array::New(2);
			arr->Set(0, v8::Int32::New(*(uint32_t * )((intptr_t) addr + 4)));
			arr->Set(1, v8::Int32::New(*(uint32_t * )addr));
			return arr;
		}else if (bit_count >= 17){
			v = *(int32_t *) addr;
		}else if (bit_count >= 9){
			if (is_unsigned){
				v = *(uint16_t *) addr;
			}else{
				v = *(int16_t *) addr;
			}
		}else if (bit_count >= 2){
			if (is_unsigned){
				v = *(uint8_t *) addr;
			}else{
				v = *(int8_t *) addr;
			}
		}else{
			return v8::Boolean::New((*(bool *)addr));
		}

		return v8::Int32::New(v);
	}; break;
	case DPT_Float: return v8::Number::New(*(float*)addr);
	case DPT_Vector: {
		auto obj = v8::Object::New();
		auto data = new SMJS_Netprops_CachedValueData();
		data->ent = ent;
		data->addr = addr;
		data->actual_offset = offset;
		data->edict = edict;
		data->prop = p;

		auto ext = v8::External::New(data);
		obj->SetHiddenValue(v8::String::New("SMJS::dataPtr"), ext);
		obj->SetAccessor(v8::String::New("x"), VectorGetter, VectorSetter, ext);
		obj->SetAccessor(v8::String::New("y"), VectorGetter, VectorSetter, ext);
		obj->SetAccessor(v8::String::New("z"), VectorGetter, VectorSetter, ext);
		obj->Set(v8::String::New("toString"), v8::FunctionTemplate::New(VectorToString)->GetFunction());

		if(isCacheable != NULL) *isCacheable = true;
		return obj;
	}; break;

	case DPT_String: return v8::String::New((char *)addr);

	case DPT_DataTable: {
		SendTable *pTable = p->GetDataTable();
		if (!pTable){
			{
				char buffer[521];
				snprintf(buffer, sizeof(buffer), "Error looking up DataTable for prop %s", name != NULL ? name : p->GetName());
				v8::ThrowException(v8::String::New(buffer));
			}
			return v8::Undefined();
		}

		SMJS_DataTable *table = new SMJS_DataTable(GetPluginRunning(), ent, edict, pTable, offset);
		if(isCacheable != NULL) *isCacheable = true;
		return table->GetWrapper();
	}break;

	default:
		v8::ThrowException(v8::String::New("Invalid Netprop Type"));
		return v8::Undefined();
	}

	return v8::Undefined();
}

SIMPLE_WRAPPED_CLS_CPP(SMJS_DataTable, SMJS_SimpleWrapped);

void SMJS_DataTable::DTGetter(uint32_t index, const PropertyCallbackInfo<Value>& args){
	SMJS_DataTable *self = (SMJS_DataTable*) v8::Handle<v8::External>::Cast(args.This()->GetInternalField(0))->Value();

	// Index can't be < 0, don't even test for it
	if(index >= (uint32_t) self->pTable->GetNumProps()) THROW_VERB("Index %d out of bounds (length %d)", index, self->pTable->GetNumProps());

	auto it = self->cachedValues.find(index);
	if(it != self->cachedValues.end()){
		RETURN(it->second);
	}

	auto pProp = self->pTable->GetProp(index);

	bool isCacheable;
	auto res = SMJS_Netprops::SGetNetProp(self->ent, self->edict, pProp, self->offset + pProp->GetOffset(), &isCacheable, self->pTable->GetName());
	if(isCacheable){
		self->cachedValues.insert(std::make_pair(index, v8::Persistent<v8::Value>::New(res)));
	}

	RETURN(res);
}

void SMJS_DataTable::DTSetter(uint32_t index, Local<Value> value, const PropertyCallbackInfo<Value>& args){
	SMJS_DataTable *self = (SMJS_DataTable*) v8::Handle<v8::External>::Cast(args.This()->GetInternalField(0))->Value();

	if(index >= (uint32_t) self->pTable->GetNumProps()) THROW_VERB("Index %d out of bounds (length %d)", index, self->pTable->GetNumProps());

	auto pProp = self->pTable->GetProp(index);
	boost::function<v8::Persistent<v8::Value> ()> f(boost::bind(&SMJS_DataTable::GenerateThenFindCachedValue, self, index, pProp, self->offset + pProp->GetOffset()));

	SMJS_Netprops::SSetNetProp(self->ent, self->edict, pProp, self->offset + pProp->GetOffset(), value, f, self->pTable->GetName());
}