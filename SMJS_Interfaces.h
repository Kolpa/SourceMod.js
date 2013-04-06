#pragma once
#include "SMJS.h"
#include <ISmmPlugin.h>
#include <eiface.h>
#include <igameevents.h>
#include <iplayerinfo.h>
#include <random.h>
#include <filesystem.h>
#include <IEngineSound.h>
#include "toolframework/itoolentity.h"
#include "ISDKTools.h"
#include "IBinTools.h"

extern IVEngineServer *engine;
extern IServerGameDLL *gamedll;
extern IServerGameClients *serverClients;
extern ICvar *icvar;
extern ISmmPluginManager *g_pMMPlugins;
extern CGlobalVars *gpGlobals;
extern IGameEventManager2 *gameevents;
extern SourceHook::CallClass<IVEngineServer> *enginePatch;
extern SourceHook::CallClass<IServerGameDLL> *gamedllPatch;
extern IUniformRandomStream *engrandom;
extern IPlayerInfoManager *playerinfo;
extern IBaseFileSystem *basefilesystem;
extern IEngineSound *enginesound;
// extern IServerPluginHelpers *serverpluginhelpers;
extern IServerPluginCallbacks *vsp_interface;
extern IServerTools *serverTools;
extern SourceMod::ISDKTools *sdkTools;
extern SourceMod::IBinTools *binTools;

bool SMJS_InitInterfaces(ISmmAPI *ismm, char *error, size_t maxlen, bool late);
void SMJS_InitLateInterfaces();