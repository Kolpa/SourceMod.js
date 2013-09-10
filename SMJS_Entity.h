#ifndef _INCLUDE_SMJS_ENTITY_H_
#define _INCLUDE_SMJS_ENTITY_H_

#include "SMJS.h"
#include "SMJS_BaseWrapped.h"
#include "SMJS_Netprops.h"
#include "SMJS_EntKeyValues.h"
#include "SMJS_DataMaps.h"

class CBaseEntity;

class SMJS_Entity : public SMJS_BaseWrapped {
public:
	CBaseEntity *ent;
	edict_t *edict;

	
	bool isEdict;
	SMJS_Netprops netprops;
	SMJS_EntKeyValues keyvalues;
	SMJS_DataMaps datamaps;

	SMJS_Entity(CBaseEntity *ent);
	void OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper);

	void SetEntity(CBaseEntity *ent);
	inline void MarkInvalid(){
		valid = false;
		ent = NULL;
		edict = NULL;
	}

	inline bool IsValid(){ return valid; }
	inline int GetIndex(){ return entIndex; }

	virtual void Destroy();

	FUNCTION_DECL(isValid);
	FUNCTION_DECL(getClassname);
	FUNCTION_DECL(input);
	FUNCTION_DECL(removeEdict);
	FUNCTION_DECL(setData);
	FUNCTION_DECL(getData);
	FUNCTION_DECL(setDataEnt);
	FUNCTION_DECL(teleport);
	FUNCTION_DECL(setRotation);
	FUNCTION_DECL(setVelocity);
	FUNCTION_DECL(changeTeam);

	WRAPPED_CLS(SMJS_Entity, SMJS_BaseWrapped) {
		temp->SetClassName(v8::String::NewSymbol("Entity"));

		proto->Set(v8::String::New("index"), v8::Int32::New(-1));
		proto->Set("netprops", v8::Null());
		proto->Set("keyvalues", v8::Null());
		proto->Set("datamaps", v8::Null());

		WRAPPED_FUNC(isValid);
		WRAPPED_FUNC(getClassname);
		WRAPPED_FUNC(input);
		WRAPPED_FUNC(removeEdict);
		WRAPPED_FUNC(setData);
		WRAPPED_FUNC(getData);
		WRAPPED_FUNC(setDataEnt);
		WRAPPED_FUNC(teleport);
		WRAPPED_FUNC(setRotation);
		WRAPPED_FUNC(setVelocity);
		WRAPPED_FUNC(changeTeam);

	}

protected:
	bool valid;
	int entIndex;

private:
	SMJS_Entity();
};

#endif
