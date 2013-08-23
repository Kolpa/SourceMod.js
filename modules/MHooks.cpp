#include "MHooks.h"

#include "smsdk_ext.h"
#include "ISDKHooks.h"

#include "SMJS_Plugin.h"
#include "SMJS_Entity.h"

#include "MEntities.h"

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
 * - Document the callback prototypes for each pre and post if applicable.
 */


SH_DECL_MANUALHOOK0_void(OnSpellStart, 0, 0, 0);

WRAPPED_CLS_CPP(MHooks, SMJS_Module)

MHooks *self;

#define SET_PRE_true(gamedataname) g_HookTypes[HookType_##gamedataname].supported = true;
#define SET_PRE_false(gamedataname)
#define SET_POST_true(gamedataname) g_HookTypes[HookType_##gamedataname##Post].supported = true;
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

struct HookTypeData
{
	const char *name;
	const char *dtReq;
	bool supported;
};

// Order MUST match HookType enum
static HookTypeData g_HookTypes[MHooks::HookType_MAX] =
{
	//   Hook name                DT required ("" for no req)  Supported (always false til later)
	{ "OnSpellStart",             "DT_DOTABaseAbility",        false },
};

MHooks::MHooks() :
	m_pSDKHooks(nullptr),
	m_pGameConf(nullptr)
{
	self = this;

	identifier = "hooks";

	// For entity listeners
	sharesys->AddDependency(myself, "sdkhooks.ext", true, true);

	SM_GET_LATE_IFACE(SDKHOOKS, m_pSDKHooks);

	if (m_pSDKHooks == nullptr)
	{
		g_pSM->LogError(myself, "Could not find interface: " SMINTERFACE_SDKHOOKS_NAME);
	}
	else
	{
		m_pSDKHooks->AddEntityListener(this);
	}

	if (!gameconfs->LoadGameConfigFile("smjs.hooks", &m_pGameConf, nullptr, 0))
	{
		g_pSM->LogError(myself, "Failed to load smjs.hooks game config.");
	}


}

MHooks::~MHooks()
{
	if (m_pSDKHooks != nullptr)
	{
		m_pSDKHooks->RemoveEntityListener(this);
	}
}

void MHooks::SetupHooks()
{
	int offset;

	//			gamedata          pre    post
	// (pre is not necessarily a prehook, just named without "Post" appeneded)

	CHECKOFFSET(OnSpellStart, true, true);
}

void MHooks::OnPluginDestroyed(SMJS_Plugin *plugin)
{
	for (int i = 0; i < ARRAYSIZE(m_Hooks); ++i)
	{
		HookList::iterator iter = m_Hooks[i].begin();
		while (iter != m_Hooks[i].end())
		{
			HookInfo hookInfo = *iter;
			if (plugin == hookInfo.plugin)
			{
				Unhook(hookInfo.entity, (HookType)i);
				iter = m_Hooks[i].erase(iter);
				continue;
			}

			iter++;
		}
	}
}

void MHooks::OnEntityDestroyed(CBaseEntity *pEntity)
{
	int entIndex = gamehelpers->EntityToBCompatRef(pEntity);

	for (int i = 0; i < ARRAYSIZE(m_Hooks); ++i)
	{
		HookList::iterator iter = m_Hooks[i].begin();
		while (iter != m_Hooks[i].end())
		{
			HookInfo hookInfo = *iter;
			if (entIndex == hookInfo.entity)
			{
				Unhook(entIndex, (HookType)i);
				iter = m_Hooks[i].erase(iter);
				continue;
			}

			iter++;
		}
	}
}

FUNCTION_M(MHooks::Hook)

	PENT(entity);
	PINT(type);
	PFUN(callback);

	HookReturn ret = self->HookInternal(entity->entIndex, (HookType)type, GetPluginRunning(), callback);
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

static bool UTIL_FindDataTable(SendTable *pTable,
	const char *name,
	sm_sendprop_info_t *info,
	unsigned int offset)
{
	const char *pname;
	int props = pTable->GetNumProps();
	SendProp *prop;
	SendTable *table;

	for (int i = 0; i < props; i++)
	{
		prop = pTable->GetProp(i);

		if ((table = prop->GetDataTable()) != NULL)
		{
			pname = table->GetName();
			if (pname && strcmp(name, pname) == 0)
			{
				info->prop = prop;
				info->actual_offset = offset + info->prop->GetOffset();
				return true;
			}

			if (UTIL_FindDataTable(table,
				name,
				info,
				offset + prop->GetOffset())
				)
			{
				return true;
			}
		}
	}

	return false;
}

MHooks::HookReturn MHooks::HookInternal(int entity, HookType type, SMJS_Plugin *pPlugin, v8::Handle<v8::Function> callback)
{
	if (!g_HookTypes[type].supported)
		return HookRet_NotSupported;

	CBaseEntity *pEntity = gamehelpers->ReferenceToEntity(entity);
	if (!pEntity)
		return HookRet_InvalidEntity;
	if (type < 0 || type >= HookType_MAX)
		return HookRet_InvalidHookType;

	if (!!strcmp(g_HookTypes[type].dtReq, ""))
	{
		sm_sendprop_info_t spi;
		IServerUnknown *pUnk = (IServerUnknown *) pEntity;

		IServerNetworkable *pNet = pUnk->GetNetworkable();
		if (pNet && !UTIL_FindDataTable(pNet->GetServerClass()->m_pTable, g_HookTypes[type].dtReq, &spi, 0))
			return HookRet_BadEntForHookType;
	}

	bool bHooked = false;
	for (auto iter = m_Hooks[type].begin(); iter != m_Hooks[type].end(); ++iter)
	{
		HookInfo hookInfo = (*iter);
		if (hookInfo.entity == entity)
		{
			bHooked = true;
			break;
		}
	}

	if (!bHooked)
	{
		switch (type)
		{
		case HookType_OnSpellStart:
			SH_ADD_MANUALHOOK(OnSpellStart, pEntity, SH_MEMBER(this, &MHooks::Hook_OnSpellStart), false);
			break;
		case HookType_OnSpellStartPost:
			SH_ADD_MANUALHOOK(OnSpellStart, pEntity, SH_MEMBER(this, &MHooks::Hook_OnSpellStartPost), true);
			break;
		}
	}

	// Add hook to hook list
	HookInfo hookInfo;
	hookInfo.entity = entity;
	hookInfo.plugin = pPlugin;
	hookInfo.callback = callback;
	m_Hooks[type].push_back(hookInfo);

	return HookRet_Successful;
}

void MHooks::Unhook(int entIndex, HookType type)
{
	CBaseEntity *pEntity = gamehelpers->ReferenceToEntity(entIndex);

	int hookCount = 0;
	HookList::iterator iter = m_Hooks[type].begin();
	while (iter != m_Hooks[type].end())
	{
		HookInfo hookInfo = *iter;
		if (entIndex == hookInfo.entity)
			hookCount++;

		iter++;
	}

	if (hookCount == 1)
	{
		switch (type)
		{
		case HookType_OnSpellStart:
			SH_REMOVE_MANUALHOOK(OnSpellStart, pEntity, SH_MEMBER(this, &MHooks::Hook_OnSpellStart), false);
			break;
		case HookType_OnSpellStartPost:
			SH_REMOVE_MANUALHOOK(OnSpellStart, pEntity, SH_MEMBER(this, &MHooks::Hook_OnSpellStartPost), true);
			break;
		}
	}
}

void MHooks::Hook_OnSpellStart()
{
	CBaseEntity *pAbility = META_IFACEPTR(CBaseEntity);

	bool handled = false;

	HookList::iterator iter = m_Hooks[HookType_OnSpellStart].begin();
	for (auto iter = m_Hooks[HookType_OnSpellStart].begin(); iter != m_Hooks[HookType_OnSpellStart].end(); ++iter)
	{
		HookInfo pHook = *iter;
		SMJS_Plugin *pPlugin = pHook.plugin;

		HandleScope handle_scope(pPlugin->GetIsolate());
		Context::Scope context_scope(pPlugin->GetContext());

		SMJS_Entity *entityWrapper = GetEntityWrapper(pAbility);
		v8::Handle<v8::Value> args[1];
		args[0] = entityWrapper->GetWrapper(pPlugin);
		auto ret = pHook.callback->Call(pPlugin->GetContext()->Global(), 1, args);
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

void MHooks::Hook_OnSpellStartPost()
{
	CBaseEntity *pAbility = META_IFACEPTR(CBaseEntity);

	HookList::iterator iter = m_Hooks[HookType_OnSpellStart].begin();
	for (auto iter = m_Hooks[HookType_OnSpellStart].begin(); iter != m_Hooks[HookType_OnSpellStart].end(); ++iter)
	{
		HookInfo pHook = *iter;
		SMJS_Plugin *pPlugin = pHook.plugin;

		HandleScope handle_scope(pPlugin->GetIsolate());
		Context::Scope context_scope(pPlugin->GetContext());

		SMJS_Entity *entityWrapper = GetEntityWrapper(pAbility);
		v8::Handle<v8::Value> args[1];
		args[0] = entityWrapper->GetWrapper(pPlugin);
		pHook.callback->Call(pPlugin->GetContext()->Global(), 1, args);
	}

	RETURN_META(MRES_IGNORED);
}
