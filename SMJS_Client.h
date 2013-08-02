#ifndef _INCLUDE_SMJS_CLIENT_H_
#define _INCLUDE_SMJS_CLIENT_H_

#include "SMJS.h"
#include "SMJS_BaseWrapped.h"
#include "SMJS_Entity.h"
#include "irecipientfilter.h"

class SMJS_Client : public SMJS_Entity {
public:
	bool inGame;
	int authStage;
	bool connected;

	SMJS_Client(edict_t *edict);
	void OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper);
	void ReattachEntity();

	virtual void Destroy(){
		edict = NULL;
	}

	FUNCTION_DECL(getName);
	FUNCTION_DECL(printToChat);
	FUNCTION_DECL(sendAudio);
	FUNCTION_DECL(printToConsole);
	FUNCTION_DECL(isInGame);
	FUNCTION_DECL(fakeCommand);
	FUNCTION_DECL(isFake);
	FUNCTION_DECL(isReplay);
	FUNCTION_DECL(isSourceTV);
	FUNCTION_DECL(getAuthString);
	FUNCTION_DECL(kick);
	FUNCTION_DECL(changeTeam);

#if SOURCE_ENGINE == SE_DOTA
	FUNCTION_DECL(invalidCommand);
#endif
	

	WRAPPED_CLS(SMJS_Client, SMJS_Entity) {
		temp->SetClassName(v8::String::NewSymbol("Client"));
		WRAPPED_FUNC(getName);
		WRAPPED_FUNC(printToChat);
		WRAPPED_FUNC(sendAudio);
		WRAPPED_FUNC(printToConsole);
		WRAPPED_FUNC(isInGame);
		WRAPPED_FUNC(fakeCommand);
		WRAPPED_FUNC(isFake);
		WRAPPED_FUNC(isReplay);
		WRAPPED_FUNC(isSourceTV);
		WRAPPED_FUNC(getAuthString);
		WRAPPED_FUNC(kick);
		WRAPPED_FUNC(changeTeam);

#if SOURCE_ENGINE == SE_DOTA
			WRAPPED_FUNC(invalidCommand);
#endif
	}

private:
	SMJS_Client();
};

class SingleRecipientFilter : public IRecipientFilter {
public:
	SingleRecipientFilter(int idx)
		:index(index._index = idx)
	{
	}

	virtual bool IsReliable( void ) const __override{
		return true;
	}

	virtual bool IsInitMessage( void ) const __override{
		return false;
	}

	virtual int GetRecipientCount( void ) const __override{
		return 1;
	}

	virtual CEntityIndex GetRecipientIndex( int slot ) const __override{
		//*client = gamehelpers->EdictOfIndex(index);
		return index;
	}

private:
	CEntityIndex index;
};

#endif
