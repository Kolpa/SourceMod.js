#include "SMJS_Entity.h"
#include "SMJS_Plugin.h"
#include "modules/MEntities.h"
#include "datamap.h"

#define SIZEOF_VARIANT_T		20

#define ENTWRAPPER_TO_CBASEENTITY(arg, buffer) \
	auto _obj##arg = args[arg]->ToObject(); \
	if(_obj##arg.IsEmpty()) THROW_VERB("Argument %d is not an entity", arg); \
	Local<Value> _intfld##arg = _obj##arg->GetInternalField(0); \
	SMJS_Entity* _ptr##arg = dynamic_cast<SMJS_Entity*>((SMJS_Base*)Handle<External>::Cast(_intfld)->Value()); \
	if(!_ptr##arg) THROW_VERB("Argument %d is not an entity", arg); \
	buffer = _ptr##arg->ent;
	


WRAPPED_CLS_CPP(SMJS_Entity, SMJS_BaseWrapped)

SMJS_Entity::SMJS_Entity(CBaseEntity *ent){
	this->ent = NULL;

	datamaps.entWrapper = this;
	netprops.entWrapper = this;
	isEdict = false;
	entIndex = -1;
	
	SetEntity(ent);

	keyvalues.entWrapper = this;
}

void SMJS_Entity::Destroy(){
	if(valid){
		SetEntityWrapper(this->ent, NULL);
		valid = false;
	}

	SMJS_BaseWrapped::Destroy();
}

void SMJS_Entity::SetEntity(CBaseEntity *ent){
	if(this->ent != NULL){
		if(this->ent != ent){
			throw "Cannot set entity twice";
		}

		return;
	}

	this->valid = false;
	if(ent == NULL) return;

	this->ent = ent;

	IServerUnknown *pUnk = (IServerUnknown *)ent;
	IServerNetworkable *pNet = pUnk->GetNetworkable();
	if(pNet){
		edict = pNet->GetEdict();
		entIndex = gamehelpers->IndexOfEdict(edict);
	}

	this->valid = true;
}

void GetEntityIndex(Local<String> property, const PropertyCallbackInfo<Value>& args){
	Local<Value> _intfld = args.This()->GetInternalField(0);
	SMJS_Entity* self = dynamic_cast<SMJS_Entity*>((SMJS_Base*)Handle<External>::Cast(_intfld)->Value());

	RETURN_INT(self->GetIndex());
}

void SMJS_Entity::OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper){
	auto obj = wrapper->ToObject();
	obj->SetAccessor(v8::String::New("index"), GetEntityIndex);

	obj->Set(v8::String::New("netprops"), netprops.GetWrapper(plugin));
	obj->Set(v8::String::New("keyvalues"), keyvalues.GetWrapper(plugin));
	obj->Set(v8::String::New("datamaps"), datamaps.GetWrapper(plugin));

}

FUNCTION_M(SMJS_Entity::isValid)
	GET_INTERNAL(SMJS_Entity*, self);
	RETURN_SCOPED(v8::Boolean::New(self->valid));
END

FUNCTION_M(SMJS_Entity::getClassname)
	GET_INTERNAL(SMJS_Entity*, self);
	if(!self->valid) THROW("Invalid entity");
	
	RETURN_STRING(gamehelpers->GetEntityClassname(self->ent));
END

FUNCTION_M(SMJS_Entity::input)
	static ICallWrapper *g_pAcceptInput = NULL;
	if (!g_pAcceptInput){
		int offset;
		if (!sdkToolsConf->GetOffset("AcceptInput", &offset)){
			THROW("\"AcceptEntityInput\" not supported by this mod");
		}
	
		offset = 44; // Temporary workaround

		PassInfo pass[6];
		pass[0].type = PassType_Basic;
		pass[0].flags = PASSFLAG_BYVAL;
		pass[0].size = sizeof(const char *);
		pass[1].type = pass[2].type = PassType_Basic;
		pass[1].flags = pass[2].flags = PASSFLAG_BYVAL;
		pass[1].size = pass[2].size = sizeof(CBaseEntity *);

		pass[3].type = PassType_Object;
		pass[3].flags = PASSFLAG_BYVAL|PASSFLAG_OCTOR|PASSFLAG_ODTOR|PASSFLAG_OASSIGNOP;
		pass[3].size = SIZEOF_VARIANT_T;
		pass[4].type = PassType_Basic;
		pass[4].flags = PASSFLAG_BYVAL;
		pass[4].size = sizeof(int);
		pass[5].type = PassType_Basic;
		pass[5].flags = PASSFLAG_BYVAL;
		pass[5].size = sizeof(bool);
	
		if (!(g_pAcceptInput=binTools->CreateVCall(offset, 0, 0, &pass[5], pass, 5))){
			THROW("\"AcceptEntityInput\" wrapper failed to initialized");
		}
	}

	ARG_BETWEEN(1, 4);

	CBaseEntity *pActivator, *pCaller, *pDest;
	
	GET_INTERNAL(SMJS_Entity*, self);
	if(!self->valid) THROW("Invalid entity");

	char *inputname;
	unsigned char vstk[sizeof(void *) + sizeof(const char *) + sizeof(CBaseEntity *)*2 + SIZEOF_VARIANT_T + sizeof(int)];
	unsigned char *vptr = vstk;
	
	pDest = self->ent;

	PSTR(inputNameTmp);
	inputname = *inputNameTmp;

	if(args.Length() >= 3){
		ENTWRAPPER_TO_CBASEENTITY(2, pActivator);
	}else{
		pActivator = NULL;
	}

	if(args.Length() >= 4){
		ENTWRAPPER_TO_CBASEENTITY(3, pCaller);
	}else{
		pCaller = NULL;
	}

	unsigned char variantTmp[SIZEOF_VARIANT_T] = {0};
	unsigned char *variant = variantTmp;

	if(args.Length() < 2){
		*(int *)variant = 0;
		variant += sizeof(int)*3;
		*(unsigned long *)variant = INVALID_EHANDLE_INDEX;
		variant += sizeof(unsigned long);
		*(fieldtype_t *)variant = FIELD_VOID;
	}else{
		auto &value = args[1];
		if(value->IsString()){
			v8::String::Utf8Value str(value);
			*(string_t *)variant = MAKE_STRING(*str);
			vptr += sizeof(int)*3 + sizeof(unsigned long);
			*(fieldtype_t *)variant = FIELD_STRING;
		}else if(value->IsInt32()){
			*(int *)variant = value->Int32Value();
			variant += sizeof(int)*3 + sizeof(unsigned long);
			*(fieldtype_t *)variant = FIELD_INTEGER;
		}else if(value->IsNumber()){
			*(float *)variant = (float) value->NumberValue();
			variant += sizeof(int)*3 + sizeof(unsigned long);
			*(fieldtype_t *)variant = FIELD_FLOAT;
		}else if(value->IsBoolean()){
			*(bool *)variant = value->BooleanValue();
			variant += sizeof(int)*3 + sizeof(unsigned long);
			*(fieldtype_t *)variant = FIELD_BOOLEAN;
		}else if(value->IsObject()){
			auto obj = value->ToObject();
			if(obj.IsEmpty()) THROW("Invalid input type");
			SMJS_Entity* smjsEnt = dynamic_cast<SMJS_Entity*>((SMJS_Base*)Handle<External>::Cast(obj->GetInternalField(0))->Value());
		
			if(smjsEnt != NULL){
				CBaseHandle bHandle = reinterpret_cast<IHandleEntity *>(smjsEnt->ent)->GetRefEHandle();
			
				variant += sizeof(int)*3;
				*(unsigned long *)variant = (unsigned long)(bHandle.ToInt());
				variant += sizeof(unsigned long);
				*(fieldtype_t *)variant = FIELD_EHANDLE;

			// TODO Add support for vectors
			}else{
				THROW("Invalid input type");
			}
		}else{
			THROW("Invalid input type");
		}
	}

	*(void **)vptr = pDest;
	vptr += sizeof(void *);
	*(const char **)vptr = inputname;
	vptr += sizeof(const char *);
	*(CBaseEntity **)vptr = pActivator;
	vptr += sizeof(CBaseEntity *);
	*(CBaseEntity **)vptr = pCaller;
	vptr += sizeof(CBaseEntity *);
	memcpy(vptr, variantTmp, SIZEOF_VARIANT_T);
	vptr += SIZEOF_VARIANT_T;
	*(int *)vptr = 0; // Unknown
	
	bool ret;
	g_pAcceptInput->Execute(vstk, &ret);
	RETURN_SCOPED(v8::Boolean::New(ret));
END

FUNCTION_M(SMJS_Entity::removeEdict)
	GET_INTERNAL(SMJS_Entity*, self);

	if(!self->valid) THROW("Invalid entity");
	if(self->edict == NULL) THROW("Entity does not have an edict");

	engine->RemoveEdict(self->edict);

	RETURN_UNDEF;
END

FUNCTION_M(SMJS_Entity::setData)
	GET_INTERNAL(SMJS_Entity*, self);
	if(!self->valid) THROW("Invalid entity");

	PINT(offset);
	PINT(size);
	PINT(value);

	if(offset < 0 || offset > (2 << 15)){
		THROW_VERB("Invalid offset %d", offset);
	}

	switch(size){
	case 1:
		*(uint8_t*)((intptr_t) self->ent + offset) = value;
		break;
	case 2:
		*(uint16_t*)((intptr_t) self->ent + offset) = value;
		break;
	case 4:
		*(uint32_t*)((intptr_t) self->ent + offset) = value;
		break;

	default: THROW_VERB("Invalid size %d", size);
	}

	RETURN_UNDEF;
END

FUNCTION_M(SMJS_Entity::getData)
	GET_INTERNAL(SMJS_Entity*, self);
	if(!self->valid) THROW("Invalid entity");

	PINT(offset);
	PINT(size);

	if(offset < 0 || offset > (2 << 15)){
		THROW_VERB("Invalid offset %d", offset);
	}

	switch(size){
	case 1:
		RETURN_INT(*(uint8_t*)((intptr_t) self->ent + offset));
		break;
	case 2:
		RETURN_INT(*(uint16_t*)((intptr_t) self->ent + offset));
		break;
	case 4:
		RETURN_INT(*(uint32_t*)((intptr_t) self->ent + offset));
		break;

	default: THROW_VERB("Invalid size %d", size);
	}

	RETURN_UNDEF;
END

FUNCTION_M(SMJS_Entity::setDataEnt)
	GET_INTERNAL(SMJS_Entity*, self);
	if(!self->valid) THROW("Invalid entity");
	SMJS_Entity* other = NULL;

	PINT(offset);
	POBJ_NULLABLE(otherTmp);

	if(!otherTmp->IsNull()){
		auto inte = otherTmp->GetInternalField(0);
		if(inte.IsEmpty()){
			THROW("Invalid other entity");
		}

		other = dynamic_cast<SMJS_Entity*>((SMJS_Base*) v8::Handle<v8::External>::Cast(inte)->Value());
		if(other == NULL) THROW("Invalid other entity");
	}


	void *addr = (void*) ((intptr_t) self->ent + offset);
	CBaseHandle &hndl = *(CBaseHandle *)addr;

	if(other == NULL){
		hndl.Set(NULL);
	}else{
		if(!other->valid) THROW("Invalid other entity");
		IHandleEntity *pHandleEnt = (IHandleEntity *)other->ent;
		hndl.Set(pHandleEnt);
	}

	RETURN_UNDEF;
END

FUNCTION_M(SMJS_Entity::teleport)
	GET_INTERNAL(SMJS_Entity*, self);
	if(!self->valid) THROW("Invalid entity");

	static ICallWrapper *g_pTeleport = NULL;
	if (!g_pTeleport){
		int offset;
		if (!sdkToolsConf->GetOffset("Teleport", &offset)){
			THROW("\"Teleport\" not supported by this mod");
		}

		// Temporary fix until the sourcemod offset is updated
		offset = 117;
	
		PassInfo pass[3];
		pass[0].type = PassType_Basic;
		pass[0].flags = PASSFLAG_BYVAL;
		pass[0].size = sizeof(void *);
		
		pass[1].type = PassType_Basic;
		pass[1].flags = PASSFLAG_BYVAL;
		pass[1].size = sizeof(void *);
		
		pass[2].type = PassType_Basic;
		pass[2].flags = PASSFLAG_BYVAL;
		pass[2].size = sizeof(void *);
		
		if (!(g_pTeleport = binTools->CreateVCall(offset, 0, 0, NULL, pass, 3))){
			THROW("\"Teleport\" wrapper failed to initialized");
		}
	}
	
	PVEC(x, y, z);

	unsigned char vstk[sizeof(void *) * 4];
	unsigned char *vptr = vstk;

	Vector vec(x, y, z);
	
	*(void **)vptr = self->ent;
	vptr += sizeof(void *);

	*(void **)vptr = &vec;
	vptr += sizeof(void *);

	*(void **)vptr = NULL;
	vptr += sizeof(void *);

	*(void **)vptr = NULL;
	vptr += sizeof(void *);

	void *ret;
	g_pTeleport->Execute(vstk, &ret);

	RETURN_UNDEF;
END

FUNCTION_M(SMJS_Entity::setRotation)
		GET_INTERNAL(SMJS_Entity*, self);
	if(!self->valid) THROW("Invalid entity");

	static ICallWrapper *g_pTeleport = NULL;
	if (!g_pTeleport){
		int offset;
		if (!sdkToolsConf->GetOffset("Teleport", &offset)){
			THROW("\"Teleport\" not supported by this mod");
		}

		// Temporary fix until the sourcemod offset is updated
		offset = 117;
	
		PassInfo pass[3];
		pass[0].type = PassType_Basic;
		pass[0].flags = PASSFLAG_BYVAL;
		pass[0].size = sizeof(void *);
		
		pass[1].type = PassType_Basic;
		pass[1].flags = PASSFLAG_BYVAL;
		pass[1].size = sizeof(void *);
		
		pass[2].type = PassType_Basic;
		pass[2].flags = PASSFLAG_BYVAL;
		pass[2].size = sizeof(void *);
		
		if (!(g_pTeleport = binTools->CreateVCall(offset, 0, 0, NULL, pass, 3))){
			THROW("\"Teleport\" wrapper failed to initialized");
		}
	}
	
	PVEC(x, y, z);

	unsigned char vstk[sizeof(void *) * 4];
	unsigned char *vptr = vstk;

	Vector vec(x, y, z);
	
	*(void **)vptr = self->ent;
	vptr += sizeof(void *);

	*(void **)vptr = NULL;
	vptr += sizeof(void *);

	*(void **)vptr = &vec;
	vptr += sizeof(void *);

	*(void **)vptr = NULL;
	vptr += sizeof(void *);

	void *ret;
	g_pTeleport->Execute(vstk, &ret);

	RETURN_UNDEF;
END

FUNCTION_M(SMJS_Entity::setVelocity)
	GET_INTERNAL(SMJS_Entity*, self);
	if(!self->valid) THROW("Invalid entity");

	static ICallWrapper *g_pTeleport = NULL;
	if (!g_pTeleport){
		int offset;
		if (!sdkToolsConf->GetOffset("Teleport", &offset)){
			THROW("\"Teleport\" not supported by this mod");
		}

		// Temporary fix until the sourcemod offset is updated
		offset = 117;
	
		PassInfo pass[3];
		pass[0].type = PassType_Basic;
		pass[0].flags = PASSFLAG_BYVAL;
		pass[0].size = sizeof(void *);
		
		pass[1].type = PassType_Basic;
		pass[1].flags = PASSFLAG_BYVAL;
		pass[1].size = sizeof(void *);
		
		pass[2].type = PassType_Basic;
		pass[2].flags = PASSFLAG_BYVAL;
		pass[2].size = sizeof(void *);
		
		if (!(g_pTeleport = binTools->CreateVCall(offset, 0, 0, NULL, pass, 3))){
			THROW("\"Teleport\" wrapper failed to initialized");
		}
	}
	
	PVEC(x, y, z);

	unsigned char vstk[sizeof(void *) * 4];
	unsigned char *vptr = vstk;

	Vector vec(x, y, z);
	
	*(void **)vptr = self->ent;
	vptr += sizeof(void *);

	*(void **)vptr = NULL;
	vptr += sizeof(void *);

	*(void **)vptr = NULL;
	vptr += sizeof(void *);

	*(void **)vptr = &vec;
	vptr += sizeof(void *);

	void *ret;
	g_pTeleport->Execute(vstk, &ret);

	RETURN_UNDEF;
END

FUNCTION_M(SMJS_Entity::changeTeam)
	GET_INTERNAL(SMJS_Entity*, self);
	if(!self->valid) THROW("Invalid entity");

	static ICallWrapper *g_pChangeTeam = NULL;
	if (!g_pChangeTeam){
		int offset;
		/*
		if (!sdkToolsConf->GetOffset("ChangeTeam", &offset)){
			THROW("\"ChangeTeam\" not supported by this mod");
		}*/

		offset = 100;
	
		PassInfo pass[1];
		pass[0].type = PassType_Basic;
		pass[0].flags = PASSFLAG_BYVAL;
		pass[0].size = sizeof(int);
		
		if (!(g_pChangeTeam = binTools->CreateVCall(offset, 0, 0, NULL, pass, 1))){
			THROW("\"ChangeTeam\" wrapper failed to initialized");
		}
	}
	
	PNUM(team);

	unsigned char vstk[sizeof(void *) + sizeof(int)];
	unsigned char *vptr = vstk;
	
	*(void **)vptr = self->ent;
	vptr += sizeof(void *);

	*(int *)vptr = team;
	vptr += sizeof(void *);

	void *ret;
	g_pChangeTeam->Execute(vstk, &ret);

	RETURN_UNDEF;
END