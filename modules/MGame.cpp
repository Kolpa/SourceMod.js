#include "MGame.h"
#include "SMJS_Plugin.h"
#include "sh_vector.h"
#include "server_class.h"
#include "MEntities.h"
#include "MSocket.h"
#include "SMJS_Interfaces.h"
#include "SMJS_GameRules.h"
#include "SMJS_Event.h"

SH_DECL_HOOK6(IServerGameDLL, LevelInit, SH_NOATTRIB, false, bool, const char *, const char *, const char *, const char *, bool, bool);
SH_DECL_HOOK0_void(IServerGameDLL, LevelShutdown, SH_NOATTRIB, false);
SH_DECL_HOOK0_void(IServerGameDLL, ServerActivate, SH_NOATTRIB, false);
SH_DECL_HOOK1_void(IServerGameDLL, Think, SH_NOATTRIB, 0, bool);
SH_DECL_HOOK2(IGameEventManager2, FireEvent, SH_NOATTRIB, 0, bool, IGameEvent *, bool);



WRAPPED_CLS_CPP(MGame, SMJS_Module)

struct TeamInfo {
	const char *ClassName;
	CBaseEntity *pEnt;
};

SourceHook::CVector<TeamInfo> g_Teams;
MGame *self;

MGame::MGame() :
	m_pSDKHooks(nullptr),
	m_pGameConf(nullptr)
{
	self = this;
	identifier = "game";

	SH_ADD_HOOK(IServerGameDLL, ServerActivate, gamedll, SH_MEMBER(this, &MGame::OnPreServerActivate), false);
	SH_ADD_HOOK(IServerGameDLL, ServerActivate, gamedll, SH_MEMBER(this, &MGame::OnServerActivate), true);

	SH_ADD_HOOK(IServerGameDLL, Think, gamedll, SH_MEMBER(this, &MGame::OnThink), true);

	SH_ADD_HOOK(IServerGameDLL, LevelShutdown, gamedll, SH_STATIC(MGame::LevelShutdown), false);

	SH_ADD_HOOK(IGameEventManager2, FireEvent, gameevents, SH_STATIC(MGame::OnFireEvent), false);
	//SH_ADD_HOOK(IGameEventManager2, FireEvent, gameevents, SH_STATIC(MGame::OnFireEvent_Post), true);

	smutils->AddGameFrameHook(MGame::OnGameFrame);

	// For entity listeners
	sharesys->AddDependency(myself, "sdkhooks.ext", true, true);

	SM_GET_LATE_IFACE(SDKHOOKS, m_pSDKHooks);

	if (m_pSDKHooks == nullptr)
	{
		g_pSM->LogError(myself, "Could not find interface: " SMINTERFACE_SDKHOOKS_NAME);
	}
	else
	{
		m_pSDKHooks->AddEntityListener(self);
	}

	char errormsg[256];
	if (!gameconfs->LoadGameConfigFile("smjs.game", &m_pGameConf, errormsg, sizeof(errormsg)))
	{
		g_pSM->LogError(myself, "%s", errormsg);
	}

	SetupEntHooks();
}

MGame::~MGame()
{
	if (m_pSDKHooks != nullptr)
	{
		m_pSDKHooks->RemoveEntityListener(self);
	}
}

void MGame::OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper){
	auto obj = wrapper->ToObject();

	obj->Set(v8::String::New("rules"), rules.GetWrapper(plugin));
}

void MGame::OnPreServerActivate(){
	ClearEntityWrappers();
}

void MGame::OnServerActivate(){
	InitTeamNatives();

	self->rules.rulesProps.proxy = FindEntityByClassname(-1, "dota_gamerules");
	self->rules.rulesProps.gamerules = sdkTools->GetGameRules();
	
	if(self->rules.rulesProps.gamerules == NULL){
		printf("Could not find game rules!\n");
	}

	self->CallGlobalFunction("OnMapStart");
}

void MGame::LevelShutdown(){
	ClearEntityWrappers();
}

void MGame::OnGameFrame(bool simulating){
	self->CallGlobalFunction("OnGameFrame");
}

void MGame::OnThink(bool finalTick){
	SMJS_Ping();
	uv_run(uvLoop, UV_RUN_DEFAULT);
	MSocket::OnThink(finalTick);
}

bool MGame::OnFireEvent(IGameEvent *pEvent, bool bDontBroadcast){
	if(META_RESULT_STATUS >= MRES_SUPERCEDE) RETURN_META_VALUE(MRES_IGNORED, false);
	if(pEvent == NULL) RETURN_META_VALUE(MRES_IGNORED, false);

	const char *name = pEvent->GetName();
	
	auto eventObject = new SMJS_Event(pEvent, !bDontBroadcast, false);
	int len = GetNumPlugins();
	for(int i = 0; i < len; ++i){
		SMJS_Plugin *pl = GetPlugin(i);
		if(pl == NULL) continue;

		HandleScope handle_scope(pl->GetIsolate());
		Context::Scope context_scope(pl->GetContext());

		auto vec = pl->GetEventHooks(name);
		if(vec->size() == 0) continue;
		
		v8::Handle<v8::Value> args = eventObject->GetWrapper(pl);

		for(auto it = vec->begin(); it != vec->end(); ++it){
			(*it)->Call(pl->GetContext()->Global(), 1, &args);

			if(eventObject->blocked){
				eventObject->Destroy();
				gameevents->FreeEvent(pEvent);
				RETURN_META_VALUE(MRES_SUPERCEDE, false);
			}
		}
	}

	bool broadcast = eventObject->broadcast;
	eventObject->Destroy();

	//FIXME: OnFireEvent_Post is broken, some kind of incompatibility with SourceMod

	if(broadcast != !bDontBroadcast){
		RETURN_META_VALUE_NEWPARAMS(MRES_IGNORED, true, &IGameEventManager2::FireEvent, (pEvent, !broadcast));
		OnFireEvent_Post(pEvent, !broadcast);
	}else{
		OnFireEvent_Post(pEvent, bDontBroadcast);
	}



	RETURN_META_VALUE(MRES_IGNORED, true);
}

bool MGame::OnFireEvent_Post(IGameEvent *pEvent, bool bDontBroadcast){
	if(META_RESULT_STATUS >= MRES_SUPERCEDE) RETURN_META_VALUE(MRES_IGNORED, false);
	if(pEvent == NULL) RETURN_META_VALUE(MRES_IGNORED, false);

	const char *name = pEvent->GetName();
	
	auto eventObject = new SMJS_Event(pEvent, !bDontBroadcast, true);
	int len = GetNumPlugins();
	for(int i = 0; i < len; ++i){
		SMJS_Plugin *pl = GetPlugin(i);
		if(pl == NULL) continue;

		HandleScope handle_scope(pl->GetIsolate());
		Context::Scope context_scope(pl->GetContext());

		auto vec = pl->GetEventPostHooks(name);
		if(vec->size() == 0) continue;
		
		v8::Handle<v8::Value> args = eventObject->GetWrapper(pl);

		for(auto it = vec->begin(); it != vec->end(); ++it){
			(*it)->Call(pl->GetContext()->Global(), 1, &args);
		}
	}

	eventObject->Destroy();
	RETURN_META_VALUE(MRES_IGNORED, true);
}


bool FindNestedDataTable(SendTable *pTable, const char *name){
	if (strcmp(pTable->GetName(), name) == 0){
		return true;
	}

	int props = pTable->GetNumProps();
	SendProp *prop;

	for (int i=0; i<props; i++){
		prop = pTable->GetProp(i);
		if (prop->GetDataTable()){
			if (FindNestedDataTable(prop->GetDataTable(), name)){
				return true;
			}
		}
	}

	return false;
}

void MGame::InitTeamNatives(){
	g_Teams.clear();
	g_Teams.resize(1);

	int edictCount = gpGlobals->maxEntities;
	for (int i=0; i<edictCount; i++){
		edict_t *pEdict = gamehelpers->EdictOfIndex(i);
		if (!pEdict || pEdict->IsFree()){
			continue;
		}
		if (!pEdict->GetNetworkable()){
			continue;
		}

		ServerClass *pClass = pEdict->GetNetworkable()->GetServerClass();

		if(FindNestedDataTable(pClass->m_pTable, "DT_Team")){
			SendProp *pTeamNumProp = gamehelpers->FindInSendTable(pClass->GetName(), "m_iTeamNum");

			if (pTeamNumProp != NULL){
				int offset = pTeamNumProp->GetOffset();
				CBaseEntity *pEnt = pEdict->GetUnknown()->GetBaseEntity();
				int TeamIndex = *(int *)((unsigned char *)pEnt + offset);

				if (TeamIndex >= (int)g_Teams.size()){
					g_Teams.resize(TeamIndex+1);
				}
				g_Teams[TeamIndex].ClassName = pClass->GetName();
				g_Teams[TeamIndex].pEnt = pEnt;
			}
		}
	}
}

CBaseEntity* MGame::NativeFindEntityByClassname(int startIndex, char *searchname){
	CBaseEntity *pEntity;

	if (startIndex == -1){
		pEntity = (CBaseEntity *)serverTools->FirstEntity();
	}else{
		pEntity = gamehelpers->ReferenceToEntity(startIndex);
		if (!pEntity){
			return NULL;
		}

		pEntity = (CBaseEntity *)serverTools->NextEntity(pEntity);
	}

	if (!pEntity) return NULL;

	const char *classname;
	int lastletterpos = strlen(searchname) - 1;

	static int offset = -1;
	if (offset == -1){
		offset = gamehelpers->FindInDataMap(gamehelpers->GetDataMap(pEntity), "m_iClassname")->fieldOffset;
	}

	string_t s;


	while (pEntity){
		if ((s = *(string_t *)((uint8_t *)pEntity + offset)) == NULL_STRING){
			pEntity = (CBaseEntity *)serverTools->NextEntity(pEntity);
			continue;
		}

		classname = STRING(s);

		
		if (searchname[lastletterpos] == '*'){
			if (strncasecmp(searchname, classname, lastletterpos) == 0)
			{
				return pEntity;
			}
		}else if (strcasecmp(searchname, classname) == 0){
			return pEntity;
		}

		pEntity = (CBaseEntity *)serverTools->NextEntity(pEntity);
	}

	return NULL;
}

CBaseEntity* MGame::FindEntityByClassname(int startIndex, char *searchname){
	return NativeFindEntityByClassname(startIndex, searchname);
}

FUNCTION_M(MGame::hook)
	ARG_COUNT(2);
	PSTR(str);
	PFUN(callback);
	GetPluginRunning()->GetHooks(*str)->push_back(v8::Persistent<v8::Function>::New(callback));
	RETURN_UNDEF;
END

FUNCTION_M(MGame::getTeamClientCount)
	ARG_COUNT(1);
	PINT(teamindex);

	if (teamindex >= (int)g_Teams.size() || !g_Teams[teamindex].ClassName){
		THROW_VERB("Team index %d is invalid (d%d)", teamindex, teamindex >= (int)g_Teams.size() ? 0 : !!g_Teams[teamindex].ClassName);
	}

	SendProp *pProp = gamehelpers->FindInSendTable(g_Teams[teamindex].ClassName, "\"player_array\"");
	ArrayLengthSendProxyFn fn = pProp->GetArrayLengthProxy();

	RETURN_SCOPED(v8::Int32::New(fn(g_Teams[teamindex].pEnt, 0)));
END

FUNCTION_M(MGame::precacheModel)
	ARG_BETWEEN(1, 2);
	PSTR(str);
	bool preload = false;
	
	if(args.Length() >= 2){
		preload = args[1]->BooleanValue();
	}


	RETURN_SCOPED(v8::Integer::New(engine->PrecacheModel(*str, preload)));
END

FUNCTION_M(MGame::findEntityByClassname)
	ARG_COUNT(2);
	PINT(startIndex);
	PSTR(searchname);

	if(startIndex != -1 && !gamehelpers->ReferenceToEntity(startIndex)){
		THROW_VERB("Entity %d is invalid", startIndex);
	}

	CBaseEntity *ent = FindEntityByClassname(startIndex, *searchname);

	if(ent == NULL){
		return v8::Null();
	}

	RETURN_SCOPED(GetEntityWrapper(ent)->GetWrapper(GetPluginRunning()));
END

FUNCTION_M(MGame::findEntitiesByClassname)
	ARG_COUNT(1);
	PSTR(searchnameTmp);
	const char *searchname = *searchnameTmp;

	auto arr = v8::Array::New();
	int length = 0;
	int lastIndex = -1;

	CBaseEntity *pEntity = (CBaseEntity *)serverTools->FirstEntity();
	if (!pEntity) RETURN_SCOPED(arr);
	const char *classname;

	int lastletterpos = strlen(searchname) - 1;

	static int offset = -1;
	if (offset == -1){
		offset = gamehelpers->FindInDataMap(gamehelpers->GetDataMap(pEntity), "m_iClassname")->fieldOffset;
	}

	string_t s;


	while (pEntity){
		if ((s = *(string_t *)((uint8_t *)pEntity + offset)) == NULL_STRING){
			pEntity = (CBaseEntity *)serverTools->NextEntity(pEntity);
			continue;
		}

		classname = STRING(s);
		
		if (searchname[lastletterpos] == '*'){
			if (strncasecmp(searchname, classname, lastletterpos) == 0){
				arr->Set(length++, GetEntityWrapper(pEntity)->GetWrapper(GetPluginRunning()));
			}
		}else if (strcasecmp(searchname, classname) == 0){
			arr->Set(length++, GetEntityWrapper(pEntity)->GetWrapper(GetPluginRunning()));
		}

		pEntity = (CBaseEntity *)serverTools->NextEntity(pEntity);
	}

	RETURN_SCOPED(arr);
END

FUNCTION_M(MGame::findEntityByTargetname)
	CBaseEntity *pEntity = (CBaseEntity *)serverTools->FirstEntity();;
	if (!pEntity) return v8::Null();

	PSTR(searchname);

	static int offset = -1;
	if (offset == -1){
		offset = gamehelpers->FindInDataMap(gamehelpers->GetDataMap(pEntity), "m_iName")->fieldOffset;
	}

	string_t s;


	while (pEntity){
		if ((s = *(string_t *)((uint8_t *)pEntity + offset)) == NULL_STRING){
			pEntity = (CBaseEntity *)serverTools->NextEntity(pEntity);
			continue;
		}

		if (strcasecmp(*searchname, STRING(s)) == 0){
			RETURN_SCOPED(GetEntityWrapper(pEntity)->GetWrapper(GetPluginRunning()));
		}

		pEntity = (CBaseEntity *)serverTools->NextEntity(pEntity);
	}

	return v8::Null();
END

FUNCTION_M(MGame::getTime)
	RETURN_SCOPED(v8::Number::New(gpGlobals->curtime));
END

FUNCTION_M(MGame::hookEvent)
	GET_INTERNAL(MGame*, self);
	PSTR(name);
	PFUN(callback);

	bool post = true;
	if(args.Length() > 2){
		post = args[2]->BooleanValue();
	}
	
	if (!gameevents->FindListener(self, *name)){
		if (!gameevents->AddListener(self, *name, true)){
			RETURN_SCOPED(v8::Boolean::New(false));
		}
	}

	auto plugin = GetPluginRunning();
	if(post){
		plugin->GetEventPostHooks(*name)->push_back(v8::Persistent<v8::Function>::New(callback));
	}else{
		plugin->GetEventHooks(*name)->push_back(v8::Persistent<v8::Function>::New(callback));
	}

	RETURN_SCOPED(v8::Boolean::New(true));
END

FUNCTION_M(MGame::createEntity)
	PSTR(name);
	//TODO check if the map is running
	
	CBaseEntity *pEntity = (CBaseEntity *) serverTools->CreateEntityByName(*name);
	if(pEntity == NULL) RETURN_SCOPED(v8::Null());
	RETURN_SCOPED(GetEntityWrapper(pEntity)->GetWrapper(GetPluginRunning()));
END

FUNCTION_M(MGame::getPropOffset)
	PSTR(serverCls);
	PSTR(propName);
	sm_sendprop_info_t prop;
	if(SMJS_Netprops::GetClassPropInfo(*serverCls, *propName, &prop)){
		RETURN_INT(prop.actual_offset);
	}

	RETURN_INT(-1);
END

FUNCTION_M(MGame::getEntityByIndex)
	PINT(index);
	
	auto edict = gamehelpers->EdictOfIndex(index);
	if(edict == NULL) RETURN_SCOPED(v8::Null());
	
	auto ent = edict->GetNetworkable()->GetBaseEntity();
	if(ent == NULL) RETURN_SCOPED(v8::Null());

	RETURN_SCOPED(GetEntityWrapper(ent)->GetWrapper(GetPluginRunning()));
END

void pauseGame(){
	SMJS_Pause();
	v8::V8::RemoveCallCompletedCallback(pauseGame);
}

FUNCTION_M(MGame::pause)
	if(GetPluginRunning()->IsSandboxed()) THROW("This function is not allowed to be called in sandboxed plugins");
	v8::V8::AddCallCompletedCallback(pauseGame);
	
	RETURN_UNDEF;
END

FUNCTION_M(MGame::resume)
	if(GetPluginRunning()->IsSandboxed()) THROW("This function is not allowed to be called in sandboxed plugins");
	SMJS_Resume();
	RETURN_UNDEF;
END

/*
 * To add new HookTypes, do the following for each:
 * - Add to HookType enum in MHooks.h, once each for pre and post if applicable.
 * - Add same hook type values to dota.js or whichever js file has them.
 * - Add the hook DECL once below this comment.
 * - Add one line to the g_HookTypes definition below.
 * - Add the offset check line once to MHooks::SetupHooks
 * - In MHooks::HookInternal, add a case for each pre and post if applicable.
 * - In MHooks::Unhook, add a case for each pre and post if applicable.
 * - Create member functions for each pre and post if applicable.
 * - Add a gamedata section in smjs.hooks.txt for the new function.
 * - Document the callback prototypes for each pre and post if applicable.
 */


SH_DECL_MANUALHOOK0_void(OnSpellStart, 0, 0, 0);

#define SET_PRE_true(gamedataname) g_EntHookTypes[EntHookType_##gamedataname].supported = true;
#define SET_PRE_false(gamedataname)
#define SET_POST_true(gamedataname) g_EntHookTypes[EntHookType_##gamedataname##Post].supported = true;
#define SET_POST_false(gamedataname)

#define CHECKOFFSET(gamedataname, supportsPre, supportsPost) \
	offset = 0; \
	m_pGameConf->GetOffset(#gamedataname, &offset); \
	if (offset > 0) \
{ \
	SH_MANUALHOOK_RECONFIGURE(gamedataname, offset, 0, 0); \
	SET_PRE_##supportsPre(gamedataname) \
	SET_POST_##supportsPost(gamedataname) \
}

struct EntHookTypeData
{
	const char *name;
	const char *dtReq;
	bool supported;
};

// Order MUST match EntHookType enum
static EntHookTypeData g_EntHookTypes[MGame::EntHookType_MAX] =
{
	//   Hook name                DT required ("" for no req)  Supported (always false til later)
	{ "OnSpellStart", "DT_DOTABaseAbility", false },
};


void MGame::SetupEntHooks()
{
	int offset;

	//			gamedata          pre    post
	// (pre is not necessarily a prehook, just named without "Post" appeneded)

	CHECKOFFSET(OnSpellStart, true, true);
}

void MGame::OnPluginDestroyed(SMJS_Plugin *plugin)
{
	for (int i = 0; i < ARRAYSIZE(m_EntHooks); ++i)
	{
		EntHookList::iterator iter = m_EntHooks[i].begin();
		while (iter != m_EntHooks[i].end())
		{
			EntHookInfo *hookInfo = *iter;
			if (plugin == hookInfo->plugin)
			{
				Unhook(hookInfo->entity, (EntHookType)i);
				hookInfo->callback.Dispose();
				iter = m_EntHooks[i].erase(iter);
				continue;
			}

			iter++;
		}
	}
}

void MGame::OnEntityDestroyed(CBaseEntity *pEntity)
{
	int entIndex = gamehelpers->EntityToBCompatRef(pEntity);

	for (int i = 0; i < ARRAYSIZE(m_EntHooks); ++i)
	{
		EntHookList::iterator iter = m_EntHooks[i].begin();
		while (iter != m_EntHooks[i].end())
		{
			EntHookInfo *hookInfo = *iter;
			if (entIndex == hookInfo->entity)
			{
				Unhook(entIndex, (EntHookType)i);
				iter = m_EntHooks[i].erase(iter);
				continue;
			}

			iter++;
		}
	}
}

FUNCTION_M(MGame::hookEnt)
	PENT(entity);
	PINT(type);
	PFUN(cbfunc);

	auto callback = v8::Persistent<v8::Function>::New(cbfunc);

	EntHookReturn ret = self->HookInternal(entity->entIndex, (EntHookType)type, GetPluginRunning(), callback);
	switch (ret)
	{
	case HookRet_InvalidEntity:
		THROW_VERB("Entity %d is invalid", entity->entIndex);
		break;
	case HookRet_InvalidHookType:
		THROW("Invalid hook type specified");
		break;
	case HookRet_NotSupported:
		THROW("Hook type not supported on this game");
		break;
	case HookRet_BadEntForHookType:
		{
			const char *pClassname = gamehelpers->GetEntityClassname(entity->ent);
			if (!pClassname)
			{
				THROW_VERB("Hook type not valid for this type of entity (%i).", entity->entIndex);
			}
			else
			{
				THROW_VERB("Hook type not valid for this type of entity (%s)", pClassname);
			}

			break;
		}
	}

	RETURN_UNDEF;
END

static bool UTIL_ContainsDataTable(SendTable *pTable, const char *name)
{
	const char *pname;
	int props = pTable->GetNumProps();
	SendProp *prop;
	SendTable *table;

	if (strcmp(name, pTable->GetName()) == 0)
		return true;

	for (int i = 0; i < props; i++)
	{
		prop = pTable->GetProp(i);

		if ((table = prop->GetDataTable()) != NULL)
		{
			pname = table->GetName();
			if (pname && strcmp(name, pname) == 0)
			{
				return true;
			}

			if (UTIL_ContainsDataTable(table, name))
			{
				return true;
			}
		}
	}

	return false;
}

MGame::EntHookReturn MGame::HookInternal(int entity, EntHookType type, SMJS_Plugin *pPlugin, v8::Persistent<v8::Function> callback)
{
	if (!g_EntHookTypes[type].supported)
		return HookRet_NotSupported;

	CBaseEntity *pEntity = gamehelpers->ReferenceToEntity(entity);
	if (!pEntity)
		return HookRet_InvalidEntity;
	if (type < 0 || type >= EntHookType_MAX)
		return HookRet_InvalidHookType;

	if (!!strcmp(g_EntHookTypes[type].dtReq, ""))
	{
		IServerUnknown *pUnk = (IServerUnknown *) pEntity;

		IServerNetworkable *pNet = pUnk->GetNetworkable();
		if (pNet && !UTIL_ContainsDataTable(pNet->GetServerClass()->m_pTable, g_EntHookTypes[type].dtReq))
			return HookRet_BadEntForHookType;
	}

	bool bHooked = false;
	for (auto iter = m_EntHooks[type].begin(); iter != m_EntHooks[type].end(); ++iter)
	{
		EntHookInfo *hookInfo = *iter;
		if (hookInfo->entity == entity)
		{
			bHooked = true;
			break;
		}
	}

	if (!bHooked)
	{
		switch (type)
		{
		case EntHookType_OnSpellStart:
			SH_ADD_MANUALHOOK(OnSpellStart, pEntity, SH_MEMBER(self, &MGame::Hook_OnSpellStart), false);
			break;
		case EntHookType_OnSpellStartPost:
			SH_ADD_MANUALHOOK(OnSpellStart, pEntity, SH_MEMBER(self, &MGame::Hook_OnSpellStartPost), true);
			break;
		}
	}

	// Add hook to hook list

	// the erase on the list when hook is removed will delete this
	EntHookInfo *hookInfo = new EntHookInfo;

	hookInfo->entity = entity;
	hookInfo->plugin = pPlugin;
	hookInfo->callback = callback;
	m_EntHooks[type].push_back(hookInfo);

	return HookRet_Successful;
}

void MGame::Unhook(int entIndex, EntHookType type)
{
	CBaseEntity *pEntity = gamehelpers->ReferenceToEntity(entIndex);

	int hookCount = 0;
	EntHookList::iterator iter = m_EntHooks[type].begin();
	while (iter != m_EntHooks[type].end())
	{
		EntHookInfo *hookInfo = *iter;
		if (entIndex == hookInfo->entity)
			hookCount++;

		iter++;
	}

	if (hookCount == 1)
	{
		switch (type)
		{
		case EntHookType_OnSpellStart:
			SH_REMOVE_MANUALHOOK(OnSpellStart, pEntity, SH_MEMBER(self, &MGame::Hook_OnSpellStart), false);
			break;
		case EntHookType_OnSpellStartPost:
			SH_REMOVE_MANUALHOOK(OnSpellStart, pEntity, SH_MEMBER(self, &MGame::Hook_OnSpellStartPost), true);
			break;
		}
	}
}

void MGame::Hook_OnSpellStart()
{
	CBaseEntity *pAbility = META_IFACEPTR(CBaseEntity);

	bool handled = false;

	for (auto iter = m_EntHooks[EntHookType_OnSpellStart].begin(); iter != m_EntHooks[EntHookType_OnSpellStart].end(); ++iter)
	{
		EntHookInfo *pHook = *iter;
		SMJS_Plugin *pPlugin = pHook->plugin;

		HandleScope handle_scope(pPlugin->GetIsolate());
		Context::Scope context_scope(pPlugin->GetContext());

		SMJS_Entity *entityWrapper = GetEntityWrapper(pAbility);
		v8::Handle<v8::Value> args[1];
		args[0] = entityWrapper->GetWrapper(pPlugin);
		auto ret = pHook->callback->Call(pPlugin->GetContext()->Global(), 1, args);
		if (ret.IsEmpty() || ret->IsUndefined() || ret->IsString())
			continue;

		int res = ret->Int32Value();
		if (res >= Pl_Stop)
			RETURN_META(MRES_SUPERCEDE);

		if (res >= Pl_Handled)
			handled = true;
	}

	if (handled)
		RETURN_META(MRES_SUPERCEDE);

	RETURN_META(MRES_IGNORED);
}

void MGame::Hook_OnSpellStartPost()
{
	CBaseEntity *pAbility = META_IFACEPTR(CBaseEntity);

	for (auto iter = m_EntHooks[EntHookType_OnSpellStart].begin(); iter != m_EntHooks[EntHookType_OnSpellStart].end(); ++iter)
	{
		EntHookInfo *pHook = *iter;
		SMJS_Plugin *pPlugin = pHook->plugin;

		HandleScope handle_scope(pPlugin->GetIsolate());
		Context::Scope context_scope(pPlugin->GetContext());

		SMJS_Entity *entityWrapper = GetEntityWrapper(pAbility);
		v8::Handle<v8::Value> args[1];
		args[0] = entityWrapper->GetWrapper(pPlugin);
		pHook->callback->Call(pPlugin->GetContext()->Global(), 1, args);
	}

	RETURN_META(MRES_IGNORED);
}