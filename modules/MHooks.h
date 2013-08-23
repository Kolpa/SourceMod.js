#pragma once
#include "SMJS_Module.h"

#include <sourcehook.h>
#include <sh_list.h>

class MHooks :
	public SMJS_Module,
	public ISMEntityListener
{
public:
	enum HookType
	{
		HookType_OnSpellStart,
		HookType_OnSpellStartPost,

		HookType_MAX
	};

	enum HookReturn
	{
		HookRet_Successful,
		HookRet_InvalidEntity,
		HookRet_InvalidHookType,
		HookRet_NotSupported,
		HookRet_BadEntForHookType,
	};

private:
	class HookInfo
	{
	public:
		int entity;
		SMJS_Plugin *plugin;
		v8::Handle<v8::Function> callback;
	};

	typedef SourceHook::List<HookInfo> HookList;

public:
	MHooks();
	virtual ~MHooks();

	WRAPPED_CLS(MHooks, SMJS_Module) {
		temp->SetClassName(v8::String::NewSymbol("HooksModule"));

	}

	virtual void OnPluginDestroyed(SMJS_Plugin *plugin) __override;

	// ISMEntityListener
public:
	virtual void OnEntityDestroyed(CBaseEntity *pEntity);

	// Hook callbacks
public:
	void Hook_OnSpellStart();
	void Hook_OnSpellStartPost();

public:
	FUNCTION_DECL(Hook);

private:
	void SetupHooks();
	HookReturn HookInternal(int entity, HookType type, SMJS_Plugin *pPlugin, v8::Handle<v8::Function> callback);
	void Unhook(int entIndex, HookType type);

private:
	IGameConfig *m_pGameConf;
	ISDKHooks *m_pSDKHooks;

	HookList m_Hooks[HookType_MAX];
};
