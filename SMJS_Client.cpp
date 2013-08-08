#include "SMJS_Client.h"
#include "SMJS_Plugin.h"
#include "game/shared/protobuf/usermessages.pb.h"

#if SOURCE_ENGINE == SE_DOTA
	#include "game/shared/dota/protobuf/dota_usermessages.pb.h"
#endif

WRAPPED_CLS_CPP(SMJS_Client, SMJS_Entity)

SMJS_Client::SMJS_Client(edict_t *edict) : SMJS_Entity(NULL) {
	this->edict = edict;
	inGame = false;
	authStage = 0;
	
	entIndex = gamehelpers->IndexOfEdict(edict);
}

void SMJS_Client::OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper){
	SMJS_Entity::OnWrapperAttached(plugin, wrapper);
	
}

void SMJS_Client::ReattachEntity(){
	auto tmp = edict->GetNetworkable();
	if(tmp != NULL){
		SetEntity(tmp->GetBaseEntity());
	}
}


FUNCTION_M(SMJS_Client::getName)
	GET_INTERNAL(SMJS_Client*, self);
	if(self->edict == NULL) THROW("Invalid edict");
	RETURN_SCOPED(v8::String::New(playerhelpers->GetGamePlayer(self->edict)->GetName()));
END

void SendMessage(int clientIndex, int dest, const char *str){
	SingleRecipientFilter filter(clientIndex);

	CUserMsg_TextMsg textmsg;
	textmsg.set_dest(dest);
	textmsg.add_param(str);
	textmsg.add_param("");
	textmsg.add_param("");
	textmsg.add_param("");
	textmsg.add_param("");

	engine->SendUserMessage(filter, UM_TextMsg, textmsg);
}

FUNCTION_M(SMJS_Client::printToChat)
	GET_INTERNAL(SMJS_Client*, self);
	if(!self->valid) THROW("Invalid entity");
	
	PSTR(str);
	
	SendMessage(self->entIndex, TEXTMSG_DEST_CHAT, *str);
	RETURN_UNDEF;
END

FUNCTION_M(SMJS_Client::printToConsole)
	GET_INTERNAL(SMJS_Client*, self);
	if(!self->valid) THROW("Invalid entity");
	PSTR(str);

	SendMessage(self->entIndex, TEXTMSG_DEST_CONSOLE, *str);
	RETURN_UNDEF;
END

FUNCTION_M(SMJS_Client::isInGame)
	GET_INTERNAL(SMJS_Client*, self);
	if(!self->valid) return v8::Boolean::New(false);
	RETURN_SCOPED(v8::Boolean::New(self->inGame));
END

FUNCTION_M(SMJS_Client::isFake)
	GET_INTERNAL(SMJS_Client*, self);
	if(self->edict == NULL) THROW("Invalid edict");
	RETURN_SCOPED(v8::Boolean::New(playerhelpers->GetGamePlayer(self->edict)->IsFakeClient()));
END

FUNCTION_M(SMJS_Client::isReplay)
	GET_INTERNAL(SMJS_Client*, self);
	if(self->edict == NULL) THROW("Invalid edict");
	RETURN_SCOPED(v8::Boolean::New(playerhelpers->GetGamePlayer(self->edict)->IsReplay()));
END

FUNCTION_M(SMJS_Client::isSourceTV)
	GET_INTERNAL(SMJS_Client*, self);
	if(self->edict == NULL) THROW("Invalid edict");
	RETURN_SCOPED(v8::Boolean::New(playerhelpers->GetGamePlayer(self->edict)->IsSourceTV()));
END

FUNCTION_M(SMJS_Client::fakeCommand)
	GET_INTERNAL(SMJS_Client*, self);
	if(!self->valid) THROW("Invalid entity");
	PSTR(str);
	engine->ClientCommand(self->entIndex, *str);
	RETURN_UNDEF;
END

FUNCTION_M(SMJS_Client::getAuthString)
	GET_INTERNAL(SMJS_Client*, self);
	if(self->edict == NULL) THROW("Invalid edict");
	RETURN_SCOPED(v8::String::New(engine->GetPlayerNetworkIDString(self->entIndex - 1)));
END

FUNCTION_M(SMJS_Client::kick)
	GET_INTERNAL(SMJS_Client*, self);
	if(!self->valid) THROW("Invalid entity");
	PSTR(str);
	char *kickReason = new char[str.length() + 1];
	strcpy(kickReason, *str);
	auto gameplayer = playerhelpers->GetGamePlayer(self->edict);
	if(gameplayer == NULL){
		THROW("Client cannot be kicked at this time");
	}
		
	gameplayer->Kick(kickReason);

	RETURN_UNDEF;
END

FUNCTION_M(SMJS_Client::changeTeam)
	GET_INTERNAL(SMJS_Client*, self);
	PINT(team);

	if(!self->valid) THROW("Invalid entity");
	playerhelpers->GetGamePlayer(self->edict)->GetPlayerInfo()->ChangeTeam(team);

	RETURN_UNDEF;
END

#if SOURCE_ENGINE == SE_DOTA

FUNCTION_M(SMJS_Client::invalidCommand)
	GET_INTERNAL(SMJS_Client*, self);
	PSTR(str);
	CDOTAUserMsg_InvalidCommand msg;
	msg.set_message(*str);

	SingleRecipientFilter filter(self->entIndex);
	engine->SendUserMessage(filter, DOTA_UM_InvalidCommand, msg);

	RETURN_UNDEF;
END

#endif