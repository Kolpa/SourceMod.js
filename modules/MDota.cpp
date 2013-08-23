#include "modules/MDota.h"
#include "SMJS_Plugin.h"
#include "modules/MEntities.h"
#include "CDetour/detours.h"
#include "SMJS_Client.h"
#include "SMJS_Entity.h"
#include "SMJS_VKeyValues.h"
#include "sh_memory.h"
#include "game/shared/protobuf/usermessages.pb.h"
#include "game/shared/dota/protobuf/dota_usermessages.pb.h"
#include "MemoryUtils.h"

#define WAIT_FOR_PLAYERS_COUNT_SIG "\x83\x3D****\x00\x7E\x19\x8B\x0D****\x83\x79\x30\x00"
#define WAIT_FOR_PLAYERS_COUNT_SIG_LEN 19

#define USE_NETPROP_OFFSET(varName, clsName, propName) \
	static int varName = 0; \
	if(varName == 0){ \
		sm_sendprop_info_t prop; \
		if(!SMJS_Netprops::GetClassPropInfo(#clsName, #propName, &prop)){ \
			THROW("Couldn't find netprop " #clsName " in " #propName); \
		} \
		varName = prop.actual_offset; \
	}

#define FIND_DOTA_PTR(name) \
	if(!dotaConf->GetMemSig(#name, (void**) &name) || name == NULL){ \
		smutils->LogError(myself, "Couldn't sigscan " #name); \
	} else { \
		name = *(void**)name; \
	}

#define FIND_DOTA_PTR2(name, type) \
	if(!dotaConf->GetMemSig(#name, (void**) &name) || name == NULL){ \
		smutils->LogError(myself, "Couldn't sigscan " #name); \
	} else { \
		name = *(type**)name; \
	}
	
#define FIND_DOTA_PTR_NEW(name, signature, len) \
	if((*((void**)&name) = g_MemUtils.FindPattern(serverFac, signature, len)) == NULL){ \
		smutils->LogError(myself, "Couldn't sigscan " #name); \
	}

#define FIND_DOTA_FUNC(name) \
	if(!dotaConf->GetMemSig(#name, (void**) &name) || name == NULL){ \
		smutils->LogError(myself, "Couldn't sigscan " #name); \
	} \

WRAPPED_CLS_CPP(MDota, SMJS_Module);



enum LobbyType {
	LT_INVALID = -1,
	LT_PUBLIC_MM = 0,
	LT_PRACTISE,
	LT_TOURNAMENT,
	LT_TUTORIAL,
	LT_COOP_BOTS,
	LT_TEAM_MM,
	LT_SOLO_QUEUE
};

struct CUnitOrders
{
	int		m_iPlayerID;
	int		m_iUnknown;
	CUtlVector<int>	m_SelectedUnitEntIndexes;
	int		m_iOrderType;
	int		m_iTargetEntIndex;
	int		m_iAbilityEntIndex;
	Vector	m_TargetPos;
	bool	m_bQueueOrder;
};



struct LobbyData {
	void **vtable;
	uint8_t padding01[80];

	int gameMode; // 84
	int unknown01; // 88, values seen: 2, 3

	uint8_t padding02[8];

	LobbyType lobbyType; // 100: 0, 2, 5, 6
	bool bAllowCheats; // 104
	bool bFillWithBots; // 105
	uint8_t padding03[2];
	void **pSomethingAboutTeams; // 108
	int numTeams; // 112
	uint8_t padding04[44];
	uint32_t matchID; // 160
	uint8_t padding05[28];
	void **unknown; // 192
	int unknown02; // 196
	uint8_t padding06[8];
	int unknown03; // 208
	uint8_t padding07[4];
	bool bAllowSpectators; // 216
	uint8_t padding08[44];
	void **vtable2; // 264
};

enum FieldType {
	FT_Void = 0,
	FT_Float = 1,
	FT_Int = 5
};

struct AbilityField {
	uint32_t flags1; // EE FF EE FF (LE)
	union {
		int32_t asInt;
		float asFloat;
	} value;
		
	uint32_t flags2; // EE FF EE FF (LE)
	FieldType type;
};

struct AbilityData {
	char *field;
	char *strValue;
	char *unknown;

	struct {
		int32_t		something;
		FieldType	type;
	} root;

	AbilityField values[12];

	uint32_t	flags;
};


class CRecipientFilter : public IRecipientFilter
{
private:
        bool                            m_bReliable;
        bool                            m_bInitMessage;
        CUtlVector< int >               m_Recipients;
       
        // If using prediction rules, the filter itself suppresses local player
        bool                            m_bUsingPredictionRules;
        // If ignoring prediction cull, then external systems can determine
        //  whether this is a special case where culling should not occur
        bool                            m_bIgnorePredictionCull;

public:
						CRecipientFilter() { m_Recipients.SetSize(2048); m_bReliable = true; m_bInitMessage = true; m_bUsingPredictionRules = false; m_bIgnorePredictionCull = false; }
        virtual         ~CRecipientFilter() {}
 
        virtual bool    IsReliable( void ) const { return m_bReliable; }
		virtual bool    IsInitMessage( void ) const { return m_bInitMessage; }
 
        virtual int             GetRecipientCount( void ) const { return m_Recipients.Count(); }
        virtual CEntityIndex             GetRecipientIndex( int slot ) const
        {
                if (slot < 0 || slot >= m_Recipients.Count())
                        return -1;
               
                return m_Recipients[slot];
        }

		void AddIndex( CEntityIndex index )
		{
			m_Recipients.AddToTail(index.Get());
		}
};

static IGameConfig *dotaConf = NULL;
static void *LoadParticleFile;
static void *CreateUnit;
static void *EndCooldown;
static void *SetRuneType;
static void *SpawnRune;
static void *GetCursorTarget;
static void *ParticleManagerFunc;
static void *CreateParticleEffect;
static void *SetParticleControlEnt;
static void *SetParticleControl;
static void *ApplyDamage;
static void *AddNewModifier;
static void *CreateAbility;
static void *SetAbilityByIndex;
static void *MakeTeamLose;
static void *RemoveModifierByName;
static void *LookupAttachment;
static void *SetHealth;
static void *Heal;
static void *FindModifierByName;
static void *ExecuteOrders;
static void *GetCursorLocation;

static uint8_t GetParticleManager[4];

static void *gamerules;
static void *GameManager;

static int particleIndex = 100000; //i don't know how to properly hook into the particle index system yet so i'll just do this bullshit

static int waitingForPlayersCount = 10;
static int* waitingForPlayersCountPtr = NULL;
static int* expRequiredForLevel = NULL;

static CDetour *parseUnitDetour;
static CDetour *getAbilityValueDetour;
static CDetour *clientPickHeroDetour;
static CDetour *heroBuyItemDetour;
static CDetour *unitThinkDetour;
static CDetour *heroSpawnDetour;
static CDetour *isDeniableDetour;

static void (*UTIL_Remove)(IServerNetworkable *oldObj);
static void **FindUnitsInRadius;

static void* (*FindClearSpaceForUnit)(void *unit, Vector vec, int bSomething);
static CBaseEntity* (*DCreateItem)(const char *item, void *unit, void *unit2);
static int (__stdcall *DGiveItem)(CBaseEntity *inventory, int a4, int a5, char a6); // eax = 0, ecx = item
static void (*DDestroyItem)(CBaseEntity *item);
static int (__stdcall *StealAbility)(CBaseEntity *hero, CBaseEntity *newAbility, int slot, int somethingUsuallyZero);
static CBaseEntity* (*DCreateItemDrop)(Vector location);
static void **DLinkItemDrop;

// Because we need to pass a vector for the game to fill,
// when it tries to expand it, it'll corrupt the heap because the way that
// we allocate memory isn't the same one as the server.dll is allocating it
// (we're not using Valve's allocator)
// So we need to create a vector large enough that the game won't try to expand it
// and we'll be fine.
static CUtlVector<CBaseHandle> findClearSpaceForUnitOutput(0, 2048);

static bool canSetState = false;

static void PatchVersionCheck();
static void PatchWaitForPlayersCount();

#include "modules/MDota_Detours.h"



MDota::MDota(){

	identifier = "dota";

	printf("Initializing dota.js module...\n");

	auto serverFac = g_SMAPI->GetServerFactory(false);

	PatchVersionCheck();
	PatchWaitForPlayersCount();

	char conf_error[255] = "";
	if (!gameconfs->LoadGameConfigFile("smjs.dota", &dotaConf, conf_error, sizeof(conf_error))){
		if (conf_error){
			smutils->LogError(myself, "Could not read smjs.dota.txt: %s", conf_error);
			return;
		}
	}

	CDetourManager::Init(g_pSM->GetScriptingEngine(), dotaConf);

	

	parseUnitDetour = DETOUR_CREATE_MEMBER(ParseUnit, "ParseUnit");
	if(parseUnitDetour) parseUnitDetour->EnableDetour();

	getAbilityValueDetour = DETOUR_CREATE_STATIC(GetAbilityValue, "GetAbilityValue");
	if(getAbilityValueDetour) getAbilityValueDetour->EnableDetour();

	clientPickHeroDetour = DETOUR_CREATE_STATIC(ClientPickHero, "ClientPickHero");
	if(clientPickHeroDetour) clientPickHeroDetour->EnableDetour();
	
	heroBuyItemDetour = DETOUR_CREATE_STATIC(HeroBuyItem, "HeroBuyItem");
	if(heroBuyItemDetour) heroBuyItemDetour->EnableDetour();

	unitThinkDetour = DETOUR_CREATE_MEMBER(UnitThink, "UnitThink");
	if(unitThinkDetour) unitThinkDetour->EnableDetour();

	heroSpawnDetour = DETOUR_CREATE_MEMBER(HeroSpawn, "HeroSpawn");
	if(heroSpawnDetour) heroSpawnDetour->EnableDetour();

	isDeniableDetour = DETOUR_CREATE_STATIC(IsDeniable, "IsDeniable");
	if(isDeniableDetour) isDeniableDetour->EnableDetour();

	FIND_DOTA_PTR(GameManager);

	FIND_DOTA_PTR_NEW(UTIL_Remove, "\x55\x8B\xEC\x83\xE4\xF8\x56\x8B\x75\x08\x57\x85\xF6\x74*\x8B\x46\x08\xF6\x80****\x01\x75*\x8B", 28);
	FIND_DOTA_FUNC(FindUnitsInRadius);
	FIND_DOTA_FUNC(LoadParticleFile);
	FIND_DOTA_FUNC(SpawnRune);
	FIND_DOTA_FUNC(CreateUnit);
	FIND_DOTA_FUNC(GetCursorTarget);
	FIND_DOTA_FUNC(FindClearSpaceForUnit);
	FIND_DOTA_FUNC(SetRuneType);
	FIND_DOTA_FUNC(ApplyDamage);
	FIND_DOTA_FUNC(ExecuteOrders);
	FIND_DOTA_FUNC(SetAbilityByIndex);
	FIND_DOTA_FUNC(RemoveModifierByName);
	FIND_DOTA_FUNC(GetCursorLocation);
	FIND_DOTA_FUNC(CreateParticleEffect);
	FIND_DOTA_FUNC(SetParticleControlEnt);
	FIND_DOTA_FUNC(SetParticleControl);
	FIND_DOTA_FUNC(AddNewModifier);
	FIND_DOTA_FUNC(SetHealth);
	FIND_DOTA_FUNC(Heal);
	FIND_DOTA_FUNC(CreateAbility);
	FIND_DOTA_FUNC(DCreateItem);
	FIND_DOTA_FUNC(ParticleManagerFunc);
	FIND_DOTA_FUNC(FindModifierByName);
	FIND_DOTA_FUNC(DGiveItem);
	FIND_DOTA_FUNC(DDestroyItem);
	FIND_DOTA_FUNC(MakeTeamLose);
	FIND_DOTA_FUNC(LookupAttachment);
	FIND_DOTA_FUNC(EndCooldown);
	FIND_DOTA_FUNC(StealAbility);
	FIND_DOTA_FUNC(DCreateItemDrop);
	FIND_DOTA_FUNC(DLinkItemDrop);
	
	expRequiredForLevel = (int*) memutils->FindPattern(g_SMAPI->GetServerFactory(false), "\x00\x00\x00\x00\xC8\x00\x00\x00\xF4\x01\x00\x00\x84\x03\x00\x00\x78\x05\x00\x00", 20);
	if(expRequiredForLevel == NULL){
		smutils->LogError(myself, "Couldn't find expRequiredForLevel\n");
	}
	
	//\x56\x57\x8B\xF9\xBE\x64\x0B\x00\x00\x8D\xA4\x24\x00\x00\x00\x00\x8B\xC7\xE8\x2A\x2A\x2A\x2A\x8B\x0C\x30\x83\xF9\xFF\x74\x2A\x8B\xC1\x25\xFF\xFF\x00\x00\xC1\xE0\x04\x05\x2A\x2A\x2A\x2A\xC1\xE9\x10\x39\x48\x04\x75\x2A\x8B\x00\xEB

	uint8_t *ptr = (uint8_t*)ParticleManagerFunc; //GetParticleManager is inlined in windows, this is a function that happens to contain the inlined dword. The rest is history
	SourceHook::SetMemAccess(ptr, 60, SH_MEM_READ | SH_MEM_WRITE | SH_MEM_EXEC);

	for(int i = 45; i < 49; i++){
		GetParticleManager[i-45] = ptr[i];
	}

	printf("Done!\n");
}

void MDota::OnWrapperAttached(SMJS_Plugin *plugin, v8::Persistent<v8::Value> wrapper){
	auto obj = wrapper->ToObject();
}


void PatchVersionCheck(){

	uint8_t *ptr = (uint8_t*) memutils->FindPattern(g_SMAPI->GetServerFactory(false), 
	"\x8B\x2A\x2A\x2A\x2A\x2A\x8B\x11\x8B\x82\x2A\x2A\x2A\x2A\xFF\xD0\x8B\x0D\x2A\x2A\x2A\x2A\x50\x51\x68\x2A\x2A\x2A\x2A"
	"\xFF\x2A\x2A\x2A\x2A\x2A\x8B\x0D\x2A\x2A\x2A\x2A\x8B\x11\x8B\x82\x2A\x2A\x2A\x2A\x83\xC4\x0C\x68\x2A\x2A\x2A\x2A\xFF"
	"\xD0"
	, 59);

	if(ptr == NULL){
		printf("Failed to patch version check!\n");
		smutils->LogError(myself, "Failed to patch version check 1!");
		return;
	}

	SourceHook::SetMemAccess(ptr, 59, SH_MEM_READ | SH_MEM_WRITE | SH_MEM_EXEC);

	for(int i = 35; i < 59; ++i){
		ptr[i] = 0x90; // NOP
	}

	//i dunno it works or something
	ptr = (uint8_t*) memutils->FindPattern(g_SMAPI->GetServerFactory(false), 
	"\x8B\x2A\x2A\x2A\x2A\x2A\x8B\x01\x8B\x2A\x2A\x2A\x2A\x2A\xFF\xD2\x89\x2A\x2A\x2A\x2A\x2A\x85\xC0\x74\x2A\x85\xF6\x74\x2A\x3B\xC6\x74\x2A\x8B\x2A\x2A\x2A\x2A\x2A\x8B\x01\x8B\x2A\x2A\x2A\x2A\x2A\xFF\xD2\x50\xA1\x2A\x2A\x2A\x2A\x50\x68\x2A\x2A\x2A\x2A\xFF\x2A\x2A\x2A\x2A\x2A\x83\xC4\x0C\x80\x2A\x2A\x2A\x2A\x2A\x2A\x74\x2A\x8B\x2A\x2A\x2A\x2A\x2A\x8B\x11", 
	84);


	if(ptr == NULL){
		printf("Failed to patch version check!\n");
		smutils->LogError(myself, "Failed to patch version check 2!");
		return;
	}

	SourceHook::SetMemAccess(ptr, 84, SH_MEM_READ | SH_MEM_WRITE | SH_MEM_EXEC);
	
	ptr[32] = 0xEB; // JZ --> JMP
}

void PatchWaitForPlayersCount(){
	uint8_t *ptr = (uint8_t*) memutils->FindPattern(g_SMAPI->GetServerFactory(false), WAIT_FOR_PLAYERS_COUNT_SIG, WAIT_FOR_PLAYERS_COUNT_SIG_LEN);

	if(ptr == NULL){
		printf("Failed to patch dota_wait_for_players_to_load_count\n");
		return;
	}

	SourceHook::SetMemAccess(ptr, WAIT_FOR_PLAYERS_COUNT_SIG_LEN, SH_MEM_READ | SH_MEM_WRITE | SH_MEM_EXEC);
	
	waitingForPlayersCountPtr = *((int **)((intptr_t) ptr + 2));
	*waitingForPlayersCountPtr = waitingForPlayersCount;
	*((int **)((intptr_t) ptr + 2)) = &waitingForPlayersCount;
}

FUNCTION_M(MDota::cursorLocation)
	PENT(unit);

	CBaseEntity *unitEnt;
	unitEnt = unit->ent;
	if(unitEnt == NULL) THROW("Invalid entity");

	Vector vec;
	void *tru;
	float thing = 0.0;
	__asm{
		mov edx, unitEnt
		mov edi, thing
		call GetCursorLocation
		mov tru, eax
	}
	printf("%d dd", tru);

	RETURN_UNDEF;
END

FUNCTION_M(MDota::executeOrders)
	PINT(playerId);
	PINT(type);
	POBJ(units);
	PENT(target);
	PENT(ability);
	PBOL(queue);
	PVEC(x,y,z);

	CUnitOrders orders;
	Vector vec(x,y,z);

	CUtlVector<int> vector;

	if(!units->IsArray()) THROW("Argument 3 must be an array of entities");

	auto arr = v8::Handle<v8::Array>::Cast(units);

	for(int i = 0; i < arr->Length(); i++){
		auto element = arr->Get(i)->ToObject();
		auto target = dynamic_cast<SMJS_Entity*>((SMJS_Base*) v8::Handle<v8::External>::Cast(element->GetInternalField(0))->Value());
		if(target == NULL) THROW("Invalid entity in array");
	
		vector.AddToTail(target->entIndex);
	}

	if(ability == NULL){
		orders.m_iAbilityEntIndex = -1;
	}else{
		orders.m_iAbilityEntIndex = ability->entIndex;
	}
	if(target == NULL){
		orders.m_iTargetEntIndex = -1;
	}else{
		orders.m_iTargetEntIndex = target->entIndex;
	}

	orders.m_bQueueOrder = queue;
	orders.m_TargetPos = vec;
	orders.m_SelectedUnitEntIndexes = vector;
	orders.m_iPlayerID = playerId;
	orders.m_iOrderType = type;
	orders.m_iUnknown = 0;

	auto tmp = &orders;

	__asm {
		push 1;
		push 1;
		push 1;
		push tmp
		mov edi, 0
		call ExecuteOrders
		add esp, 10h
	}

	RETURN_UNDEF;
END

FUNCTION_M(MDota::applyDamage)
	PENT(attacker);
	PENT(attacked);
	PENT(ability);
	PNUM(damage);
	PINT(damageType);

	CBaseEntity *attackerEnt;
	attackerEnt = attacker->ent;
	CBaseEntity *attackedEnt;
	attackedEnt = attacked->ent;
	CBaseEntity *abilityEnt;
	abilityEnt = ability->ent;

	if(attackerEnt == NULL || attackedEnt == NULL || abilityEnt == NULL) THROW("Entity cannot be null");

	auto pDamage = (float)damage;

	__asm {
		mov edi, attackedEnt 
		push 0
		push damageType
		push pDamage
		push abilityEnt
		push attackerEnt
		call ApplyDamage
		add esp, 14h
	}
	
	RETURN_UNDEF;
END

FUNCTION_M(MDota::loadParticleFile)
	ARG_COUNT(1);
	PSTR(file);
	char *fileStr = *file;
	__asm{
		mov eax, fileStr
		call LoadParticleFile
	}

	RETURN_UNDEF;
END

FUNCTION_M(MDota::getCursorTarget)
	PENT(ability);
	CBaseEntity *ent;
	ent = ability->ent;

	CBaseEntity *target;
	__asm {
		mov eax, ent
		call GetCursorTarget
		mov target, eax
	}

	RETURN_SCOPED(GetEntityWrapper(target)->GetWrapper(GetPluginRunning()));
END

FUNCTION_M(MDota::setAbilityByIndex)
	PENT(unit);
	PENT(ability);
	PINT(index);

	CBaseEntity *unitEnt;
	unitEnt = unit->ent;
	CBaseEntity *abilityEnt;
	abilityEnt = ability->ent;
	
	if(unitEnt == NULL || abilityEnt == NULL) THROW("Invalid entity");

	__asm {
		push index
		push abilityEnt
		mov eax, unitEnt
		call SetAbilityByIndex
	}

	RETURN_UNDEF;
END

FUNCTION_M(MDota::setRuneType)
	PENT(rune);
	PINT(runeType);
	
	CBaseEntity *ent;
	ent = rune->ent;

	__asm {
		mov eax, ent
		push runeType
		call SetRuneType
	}

	RETURN_UNDEF;
END

FUNCTION_M(MDota::createAbility)
	PENT(hero);
	PSTR(clsname);
	
	CBaseEntity *heroEnt;
	heroEnt = hero->ent;

	CBaseEntity *ability;

	__asm {
		push heroEnt
		push clsname
		call CreateAbility
		mov ability, eax
		add esp, 8	
	}

	RETURN_SCOPED(GetEntityWrapper(ability)->GetWrapper(GetPluginRunning()));
END

FUNCTION_M(MDota::sendAudio)
	POBJ(client);

	auto target = dynamic_cast<SMJS_Client*>((SMJS_Base*) v8::Handle<v8::External>::Cast(client->GetInternalField(0))->Value());
	if(target == NULL) THROW("Invalid target");
	
	PBOL(stop);
	PSTR(name);

	SingleRecipientFilter filter(target->entIndex);
	CUserMsg_SendAudio audiomsg;
	audiomsg.set_stop(stop);
	audiomsg.set_name(*name);
	
	engine->SendUserMessage(filter, UM_SendAudio, audiomsg);
	RETURN_UNDEF;
END

FUNCTION_M(MDota::releaseParticle)
	POBJ(client);
	PINT(index);

	auto target = dynamic_cast<SMJS_Client*>((SMJS_Base*) v8::Handle<v8::External>::Cast(client->GetInternalField(0))->Value());
	if(target == NULL) THROW("Invalid target");

	SingleRecipientFilter filter(target->entIndex);
	CDOTAUserMsg_ParticleManager particlemsg;
	particlemsg.set_index(index);
	particlemsg.set_type(DOTA_PARTICLE_MANAGER_EVENT_RELEASE);

	engine->SendUserMessage(filter, DOTA_UM_ParticleManager, particlemsg);

	RETURN_UNDEF;
END

FUNCTION_M(MDota::destroyParticle)
	POBJ(client);
	PINT(index);
	PBOL(destroyImmediately);

	auto target = dynamic_cast<SMJS_Client*>((SMJS_Base*) v8::Handle<v8::External>::Cast(client->GetInternalField(0))->Value());
	if(target == NULL) THROW("Invalid target");

	SingleRecipientFilter filter(target->entIndex);
	CDOTAUserMsg_ParticleManager particlemsg;
	particlemsg.set_index(index);
	particlemsg.set_type(DOTA_PARTICLE_MANAGER_EVENT_DESTROY);
	particlemsg.mutable_destroy_particle()->set_destroy_immediately(destroyImmediately);

	engine->SendUserMessage(filter, DOTA_UM_ParticleManager, particlemsg);
	RETURN_UNDEF;
END

FUNCTION_M(MDota::setParticleControlEnt)
	PENT(unit);
	PINT(controlPoint);
	PINT(unknown);
	PSTR(attachPoint);
	PINT(attachType);
	PINT(index);

	CBaseEntity *control;
	control = unit->ent;
	if(control == NULL) THROW("Entity cannot be null");

	void *entOffset = (void*)((uintptr_t)control + 656); 
	char *str = *attachPoint;

	__asm {
		push entOffset
		push control
		push unknown //idk
		push str
		push attachType //attach_type
		push controlPoint //control_point
		push index
		mov eax, control
		call SetParticleControlEnt
	}

	RETURN_UNDEF;
END

FUNCTION_M(MDota::setParticleOrient)
	POBJ(client);
	PINT(index);
	PINT(controlPoint);
	PVEC(xfwd, yfwd, zfwd);
	PVEC(xright, yright, zright);
	PVEC(xup, yup, zup);

	auto target = dynamic_cast<SMJS_Client*>((SMJS_Base*) v8::Handle<v8::External>::Cast(client->GetInternalField(0))->Value());
	if(target == NULL) THROW("Invalid target");

	SingleRecipientFilter filter(target->entIndex);
	CDOTAUserMsg_ParticleManager particlemsg;
	particlemsg.set_index(index);
	particlemsg.set_type(DOTA_PARTICLE_MANAGER_EVENT_UPDATE_ORIENTATION);
	auto orientmsg = particlemsg.mutable_update_particle_orient();

	auto fwdvec = orientmsg->mutable_forward();
	auto rightvec = orientmsg->mutable_right();
	auto upvec = orientmsg->mutable_up();

	fwdvec->set_x(xfwd);
	fwdvec->set_y(yfwd);
	fwdvec->set_z(zfwd);

	rightvec->set_x(xright);
	rightvec->set_y(yright);
	rightvec->set_z(zright);

	upvec->set_x(xup);
	upvec->set_y(yup);
	upvec->set_z(zup);

	engine->SendUserMessage(filter, DOTA_UM_ParticleManager, particlemsg);

	RETURN_UNDEF;
END

FUNCTION_M(MDota::setParticleControl)
	POBJ(client);
	PINT(index);
	PINT(controlPoint);
	PVEC(x,y,z);

	auto target = dynamic_cast<SMJS_Client*>((SMJS_Base*) v8::Handle<v8::External>::Cast(client->GetInternalField(0))->Value());
	if(target == NULL) THROW("Invalid target");

	SingleRecipientFilter filter(target->entIndex);
	CDOTAUserMsg_ParticleManager particlemsg;
	particlemsg.set_index(index);
	particlemsg.set_type(DOTA_PARTICLE_MANAGER_EVENT_UPDATE);

	auto updatemsg = particlemsg.mutable_update_particle();
	auto vector = updatemsg->mutable_position();
	updatemsg->set_control_point(controlPoint);
	vector->set_x(x);
	vector->set_y(y);
	vector->set_z(z);
	
	engine->SendUserMessage(filter, DOTA_UM_ParticleManager, particlemsg);

	RETURN_UNDEF;
END

FUNCTION_M(MDota::createParticleEffect)
	PENT(unit);
	PSTR(name);
	PINT(attach);

	CBaseEntity *ent;
	ent = unit->ent;

	if(ent == NULL) THROW("Entity cannot be null");
	if(attach == 2) ent = 0;

	const char *string = *name;

	__asm {
		mov eax, dword ptr GetParticleManager //particlemanager objectptr
		push 0 // idk object handle type or something 
		push ent
		push attach //attach_type
	    push particleIndex //index
		mov ecx, string //particlesystem name
		call CreateParticleEffect
	}

	particleIndex++;

	RETURN_SCOPED(v8::Int32::New(particleIndex - 1));
END

KeyValues *KeyValuesFromJsObject(v8::Handle<v8::Object> options, const char *setName){

	KeyValues *kv = new KeyValues(setName);

	auto properties = options->GetOwnPropertyNames();

	v8::String::Utf8Value str(properties->Get(0));

	for(int i = 0; i < properties->Length(); i++){
		v8::String::Utf8Value prop(properties->Get(i));
		kv->SetFloat(*prop, options->Get(String::New(*prop))->Int32Value());	
	}
	return kv;
}

FUNCTION_M(MDota::mapLine)
	POBJ(receiving);

	auto target = dynamic_cast<SMJS_Client*>((SMJS_Base*) v8::Handle<v8::External>::Cast(receiving->GetInternalField(0))->Value());
	if(target == NULL) THROW("Invalid target");

	PINT(playerId);
	PBOL(initial);
	PVEC(x,y,z);

	CDOTAUserMsg_MapLine mapmsg;
	mapmsg.set_player_id(playerId);

	mapmsg.mutable_mapline()->set_x(x);
	mapmsg.mutable_mapline()->set_y(y);
	mapmsg.mutable_mapline()->set_initial(initial);

	SingleRecipientFilter filter(target->entIndex);
	engine->SendUserMessage(filter, DOTA_UM_MapLine, mapmsg);
	RETURN_UNDEF;
END

FUNCTION_M(MDota::pingLocation)
	POBJ(receiving);

	auto target = dynamic_cast<SMJS_Client*>((SMJS_Base*) v8::Handle<v8::External>::Cast(receiving->GetInternalField(0))->Value());
	if(target == NULL) THROW("Invalid target");

	PINT(playerId);
	PINT(pingTarget);
	PBOL(directPing);
	PINT(type);
	PVEC(x,y,z);

	CDOTAUserMsg_LocationPing pingmsg;
	pingmsg.set_player_id(playerId);

	pingmsg.mutable_location_ping()->set_x(x);
	pingmsg.mutable_location_ping()->set_y(y);
	pingmsg.mutable_location_ping()->set_target(pingTarget);
	pingmsg.mutable_location_ping()->set_type(type);
	pingmsg.mutable_location_ping()->set_direct_ping(directPing);

	SingleRecipientFilter filter(target->entIndex);
	engine->SendUserMessage(filter, DOTA_UM_LocationPing, pingmsg);

	RETURN_UNDEF;
END

FUNCTION_M(MDota::worldLine)
	POBJ(receiving);

	auto target = dynamic_cast<SMJS_Client*>((SMJS_Base*) v8::Handle<v8::External>::Cast(receiving->GetInternalField(0))->Value());
	if(target == NULL) THROW("Invalid target");

	PINT(playerId);
	PBOL(end);
	PBOL(initial);
	PVEC(x,y,z);

	SingleRecipientFilter filter(target->entIndex);

	CDOTAUserMsg_WorldLine linemsg;
	linemsg.set_player_id(playerId);
	
	auto internalmsg = linemsg.mutable_worldline();

	internalmsg->set_x(x);
	internalmsg->set_y(y);
	internalmsg->set_z(z);
	
	
		internalmsg->set_end(end);
	
		internalmsg->set_initial(initial);
	

	engine->SendUserMessage(filter, DOTA_UM_WorldLine, linemsg);

	RETURN_UNDEF;
END

FUNCTION_M(MDota::removeModifier)
	 PENT(unit);
	 PSTR(modifier);

	 CBaseEntity *unitEnt;
	 unitEnt = unit->ent;

	 if(unitEnt == NULL) THROW("Invalid entity");

	 char *pModifier = *modifier;
	 void *modifierManager = (void*)((uintptr_t)unitEnt + 3004);  //I think this might crash on non-npc ents, so.. trees and misc. shit

	 __asm {
		 push pModifier
		 push modifierManager
		 call RemoveModifierByName
	 }
	
	RETURN_UNDEF;
END

FUNCTION_M(MDota::addNewModifier) 
	PENT(target);
	PENT(ability);
	PSTR(modifierName);
	PSTR(setName);
	POBJ(options);

	CBaseEntity *targetEnt;
	targetEnt = target->ent;

	CBaseEntity *abilityEnt;
	abilityEnt = ability->ent;

	if(targetEnt == NULL || abilityEnt == NULL) THROW("Entity cannot be null");

	void *modifierManager = (void*)((uintptr_t)targetEnt + 3004);  //I think this might crash on non-npc ents, so.. trees and misc. shit

	KeyValues *kv = KeyValuesFromJsObject(options, *setName);

	if(kv == NULL) THROW("options object misconfigured");

	char *modifier = *modifierName;

	__asm {
		mov ecx, modifierManager
		push 0
		push 4294967295
		push kv
		push modifier
		push abilityEnt
		push targetEnt
		call AddNewModifier
	}

	RETURN_UNDEF;
END

FUNCTION_M(MDota::spawnRune)
	CBaseEntity *rune;
	
	if (gamerules == NULL){
		gamerules = sdkTools->GetGameRules();
	}
	__asm {
		push gamerules
		call SpawnRune
	}
	
	RETURN_UNDEF;
END

FUNCTION_M(MDota::remove)
	PENT(ent);
	if(ent == NULL) THROW("Entity cannot be null");

	IServerUnknown *pUnk = (IServerUnknown *)ent->ent;
	IServerNetworkable *pNet = pUnk->GetNetworkable();
	if(pNet == NULL) THROW("Entity doesn't have a IServerNetworkable");

	UTIL_Remove(pNet);

	RETURN_UNDEF;
END

FUNCTION_M(MDota::heroIdToClassname)
	PINT(id);
	auto ret = HeroIdToClassname(id);
	if(ret == NULL) return v8::Null();
	RETURN_SCOPED(v8::String::New(ret));
END

FUNCTION_M(MDota::forceWin)
	PINT(team);
	if(team != 2 && team != 3) THROW_VERB("Invalid team %d", team);
	
	if (gamerules == NULL){
		gamerules = sdkTools->GetGameRules();
	}

	__asm {
		mov eax, gamerules
		push team
		call MakeTeamLose
	}

	RETURN_UNDEF;
END

FUNCTION_M(MDota::endCooldown)
	PENT(ability);
	CBaseEntity *ent;
	ent = ability->ent;

	__asm {
		push ent
		call EndCooldown
	}

	RETURN_UNDEF;
END

FUNCTION_M(MDota::createUnit)
	PSTR(tmp);
	PINT(team);
	
	CBaseEntity *ent;
	CBaseEntity *targetEntity;
	char *clsname = *tmp;
	

	if(args.Length() > 2){
		POBJ(otherTmp);
		auto inte = otherTmp->GetInternalField(0);
		if(inte.IsEmpty()){
			THROW("Invalid other entity");
		}

		auto other = dynamic_cast<SMJS_Entity*>((SMJS_Base*) v8::Handle<v8::External>::Cast(inte)->Value());
		if(other == NULL) THROW("Invalid other entity");
		targetEntity = other->ent;
	}else{
		targetEntity = gamehelpers->EdictOfIndex(0)->GetNetworkable()->GetBaseEntity();
	}

	__asm {
		mov		eax, clsname
		push	0
		push	1
		push	team
		push	targetEntity
		call	CreateUnit
		mov		ent, eax
		add		esp, 10h
	}
	
	if(ent == NULL) return v8::Null();
	RETURN_SCOPED(GetEntityWrapper(ent)->GetWrapper(GetPluginRunning()));
END

FUNCTION_M(MDota::findClearSpaceForUnit)
	POBJ(otherTmp);
	PVEC(x, y, z);

	CBaseEntity *targetEntity;
	auto inte = otherTmp->GetInternalField(0);
	if(inte.IsEmpty()){
		THROW("Invalid other entity");
	}

	auto other = dynamic_cast<SMJS_Entity*>((SMJS_Base*) v8::Handle<v8::External>::Cast(inte)->Value());
	if(other == NULL) THROW("Invalid other entity");
	targetEntity = other->ent;

	FindClearSpaceForUnit(targetEntity, Vector(x, y, z), true);
	RETURN_UNDEF;
END

FUNCTION_M(MDota::setWaitForPlayersCount)
	if(GetPluginRunning()->IsSandboxed()) THROW("This function is not allowed to be called in sandboxed plugins");	

	PINT(c);

	waitingForPlayersCount = c;
	if(waitingForPlayersCountPtr != NULL){
		*waitingForPlayersCountPtr = c;
	}

	auto d2fixupsConVar = icvar->FindVar("dota_wait_for_players_to_load_count");
	
	if(d2fixupsConVar != NULL){
		d2fixupsConVar->SetValue(c);
	}

	RETURN_UNDEF;
END

FUNCTION_M(MDota::giveItemToHero)
	USE_NETPROP_OFFSET(offset, CDOTA_BaseNPC, m_Inventory);

	PSTR(itemClsname);
	PENT(ent);
	if(ent == NULL){
		THROW("Must provide a valid hero");
	}

	auto item = DCreateItem(*itemClsname, ent->ent, ent->ent);
	if(item == NULL){
		RETURN_SCOPED(v8::Null());
	}

	auto inventory = (CBaseEntity*)((intptr_t)ent->ent + offset);

	int res;

	__asm{
		push	0
		push	-1
		push	3
		push	inventory
		mov		eax, 0
		mov		ecx, item
		call	DGiveItem
		mov		res, eax
	}

	if(res != -1) {
		DDestroyItem(item);
		RETURN_SCOPED(v8::Null());
	}

	RETURN_SCOPED(GetEntityWrapper(item)->GetWrapper(GetPluginRunning()));
END


FUNCTION_M(MDota::getTotalExpRequiredForLevel)
	if(expRequiredForLevel == NULL){
		RETURN_INT(0);
	}

	PINT(level);

	if(level < 1) THROW_VERB("Invalid level", level);
	if(level > 25) THROW_VERB("Invalid level", level);
	RETURN_INT(expRequiredForLevel[level - 1]);
END

FUNCTION_M(MDota::setTotalExpRequiredForLevel)
	if(expRequiredForLevel == NULL) RETURN_UNDEF;

	PINT(level);
	PINT(exp);

	if(level <= 1) THROW_VERB("Invalid level", level);
	if(level > 25) THROW_VERB("Invalid level", level);

	expRequiredForLevel[level - 1] = exp;

	RETURN_UNDEF;
END



FUNCTION_M(MDota::sendStatPopup)
	USE_NETPROP_OFFSET(offset, CDOTAPlayer, m_iPlayerID);
	
	POBJ(targetObj);

	auto target = dynamic_cast<SMJS_Client*>((SMJS_Base*) v8::Handle<v8::External>::Cast(targetObj->GetInternalField(0))->Value());
	if(target == NULL) THROW("Invalid target");


	PINT(type);
	if(args.Length() < 3) THROW("Argument 3 must be an array of strings");
	POBJ(tmpObj);
	if(!tmpObj->IsArray()) THROW("Argument 3 must be an array of strings");

	auto arr = v8::Handle<v8::Array>::Cast(tmpObj);

	CDOTAUserMsg_SendStatPopup msg;
	msg.set_player_id(*(int*)((intptr_t) target + offset));

	for(uint32_t i = 0, len = arr->Length(); i < len; ++i){
		v8::String::Utf8Value str(arr->Get(i));
		msg.mutable_statpopup()->add_stat_strings(*str, str.length());
	}

	SingleRecipientFilter filter(target->entIndex);
	engine->SendUserMessage(filter, DOTA_UM_SendStatPopup, msg);
	
	RETURN_SCOPED(v8::Boolean::New(true));
END

FUNCTION_M(MDota::setHeroAvailable)
	USE_NETPROP_OFFSET(stableOffset, CDOTAGameManagerProxy, m_StableHeroAvailable);
	USE_NETPROP_OFFSET(currentOffset, CDOTAGameManagerProxy, m_CurrentHeroAvailable);
	USE_NETPROP_OFFSET(culledOffset, CDOTAGameManagerProxy, m_CulledHeroes);
	
	PINT(hero);
	PBOL(available);

	if (hero > 128) THROW("Hero index too large!");

	*(bool*)((intptr_t) GameManager + stableOffset + hero) = available;
	*(bool*)((intptr_t) GameManager + currentOffset + hero) = available;
	*(bool*)((intptr_t) GameManager + culledOffset + hero) = !available;
	RETURN_UNDEF;
END

FUNCTION_M(MDota::createItemDrop)
	PENT(owner);
	PSTR(itemName);
	PVEC(x, y, z);

	CBaseEntity *pEntity;

	if(owner == NULL){
		THROW("Item must have an owner");
	}else{
		pEntity = owner->ent;
	}

	auto item = DCreateItem(*itemName, NULL, pEntity);
	if(item == NULL){
		RETURN_SCOPED(v8::Null());
	}

	auto drop = DCreateItemDrop(Vector(x, y, z));
	if(drop == NULL){
		DDestroyItem(item);
		RETURN_SCOPED(v8::Null());
	}

	__asm {
		mov			eax, item
		push		drop
		call		DLinkItemDrop
	}

	RETURN_SCOPED(GetEntityWrapper(drop)->GetWrapper(GetPluginRunning()));
END

FUNCTION_M(MDota::levelUpHero)
	PENT(unit);
	PBOL(playEffects);

	static ICallWrapper *g_pLevelUpHero = NULL;
	if (!g_pLevelUpHero){
		int offset;
		
		if (!dotaConf->GetOffset("HeroLevelUp", &offset)){
			THROW("\"HeroLevelUp\" not supported by this mod");
		}

		PassInfo pass[1];
		pass[0].type = PassType_Basic;
		pass[0].flags = PASSFLAG_BYVAL;
		pass[0].size = sizeof(bool);
		
		if (!(g_pLevelUpHero = binTools->CreateVCall(offset, 0, 0, NULL, pass, 1))){
			THROW("\"HeroLevelUp\" wrapper failed to initialized");
		}
	}

	unsigned char vstk[sizeof(void *) + sizeof(bool)];
	unsigned char *vptr = vstk;
	
	*(void **)vptr = unit->ent;
	vptr += sizeof(void *);

	*(bool *)vptr = playEffects;
	vptr += sizeof(bool);

	void *ret;
	g_pLevelUpHero->Execute(vstk, &ret);

	RETURN_UNDEF;
END

FUNCTION_M(MDota::levelUpAbility)
	USE_NETPROP_OFFSET(abilityPointsOffset, CDOTA_BaseNPC_Hero, m_iAbilityPoints);
	
	PENT(unit);
	PENT(ability);

	static ICallWrapper *g_pLevelUpHeroAbility = NULL;
	if (!g_pLevelUpHeroAbility){
		int offset;
		
		if (!dotaConf->GetOffset("HeroLevelUpAbility", &offset)){
			THROW("\"HeroLevelUpAbility\" not supported by this mod");
		}

		PassInfo pass[1];
		pass[0].type = PassType_Basic;
		pass[0].flags = PASSFLAG_BYVAL;
		pass[0].size = sizeof(void*);
		
		if (!(g_pLevelUpHeroAbility = binTools->CreateVCall(offset, 0, 0, NULL, pass, 1))){
			THROW("\"HeroLevelUpAbility\" wrapper failed to initialized");
		}
	}

	// This function subtracts one point from the ability points, without even checking if
	// it can. Add one point to it so it behaves like we'd expect.
	*(int*)((intptr_t) unit + abilityPointsOffset) += 1;

	unsigned char vstk[sizeof(void *) + sizeof(void *)];
	unsigned char *vptr = vstk;
	
	*(void **)vptr = unit->ent;
	vptr += sizeof(void *);

	*(void **)vptr = ability->ent;
	vptr += sizeof(void*);

	void *ret;
	g_pLevelUpHeroAbility->Execute(vstk, &ret);

	RETURN_UNDEF;
END

FUNCTION_M(MDota::hasModifier)
	PENT(unit);
	PSTR(modifier);
	PINT(unknown);

	CBaseEntity *unitEnt;
	unitEnt = unit->ent;

	if(unitEnt == NULL) THROW("Invalid entity");

	void *modifierManager = (void*)((uintptr_t)unitEnt + 3004);
	char *str = *modifier;

	void *buff;

	__asm {
		mov eax, str
		push 0 
		push modifierManager
		call FindModifierByName
		mov buff, eax
	}

	bool exists = false;
	if (buff != NULL) exists = true;

	RETURN_SCOPED(v8::Boolean::New(exists));
END

FUNCTION_M(MDota::heal)
	PENT(unit);
	PENT(ability);
	PINT(amount);

	CBaseEntity *unitEnt;
	unitEnt = unit->ent;
	CBaseEntity *abilityEnt;
	abilityEnt = ability->ent;

	if(unitEnt == NULL || abilityEnt == NULL) THROW("Invalid entity");

	__asm {
		mov eax, amount
		mov ecx, unitEnt
		push abilityEnt
		call Heal
	}

	RETURN_UNDEF;
END

FUNCTION_M(MDota::setHealth)
	PENT(unit);
	PINT(amount);

	CBaseEntity *unitEnt;
	unitEnt = unit->ent;

	__asm {
		mov ecx, unitEnt
		push amount
		call SetHealth
	}

	RETURN_UNDEF;
}

FUNCTION_M(MDota::giveExperienceToHero)
	PENT(unit);
	PNUM(amount);

	static ICallWrapper *g_pHeroGiveExperience = NULL;
	if (!g_pHeroGiveExperience){
		int offset;
		
		if (!dotaConf->GetOffset("HeroGiveExperience", &offset)){
			THROW("\"HeroGiveExperience\" not supported by this mod");
		}

		PassInfo pass[1];
		pass[0].type = PassType_Basic;
		pass[0].flags = PASSFLAG_BYVAL;
		pass[0].size = sizeof(float);
		
		if (!(g_pHeroGiveExperience = binTools->CreateVCall(offset, 0, 0, NULL, pass, 1))){
			THROW("\"HeroGiveExperience\" wrapper failed to initialized");
		}
	}

	unsigned char vstk[sizeof(void *) + sizeof(float)];
	unsigned char *vptr = vstk;
	
	*(void **)vptr = unit->ent;
	vptr += sizeof(void *);

	*(float *)vptr = amount;
	vptr += sizeof(float);

	void *ret;
	g_pHeroGiveExperience->Execute(vstk, &ret);

	RETURN_UNDEF;
END

FUNCTION_M(MDota::unitHasState)
	USE_NETPROP_OFFSET(stateOffset, CDOTA_BaseNPC, m_nUnitState);
	PENT(unit);
	PINT(state);
	
	bool res = (*(uint32_t*)((intptr_t) unit->ent + stateOffset) & (1 << state)) == (1 << state);

	RETURN_SCOPED(v8::Boolean::New(res));
END

FUNCTION_M(MDota::findUnitsInRadius)
	PENT(ent);
	PINT(team);
	PNUM(radius);
	PNUM(x);
	PNUM(y);
	PINT(targetTeamFlags);
	PINT(targetUnitTypeFlags);
	PINT(targetUnitStateFlags);

	float radiusF = radius;
	CBaseEntity *pEntity = ent ? ent->ent : NULL;

	
	Vector pos(x, y, 0.0f);

	findClearSpaceForUnitOutput.RemoveAll();

	Vector *posPointer = &pos;
	auto outputPointer = &findClearSpaceForUnitOutput;

	__asm {
		push    0
		push    0
		push    targetUnitStateFlags
		push    targetUnitTypeFlags
		push    targetTeamFlags
		push    radiusF
		push    pEntity
		push    posPointer
		push    team
		mov     eax, outputPointer
		call    FindUnitsInRadius
		add     esp, 36
	}

	auto pl = GetPluginRunning();
	auto arr = v8::Array::New(findClearSpaceForUnitOutput.Count());
	for(int i = 0; i < findClearSpaceForUnitOutput.Count(); ++i){
		CBaseEntity *foundEnt = gamehelpers->ReferenceToEntity(findClearSpaceForUnitOutput[i].GetEntryIndex());
		auto entWrapper = GetEntityWrapper(foundEnt);
		arr->Set(i, entWrapper->GetWrapper(pl));
	}

	RETURN_SCOPED(arr);
END

FUNCTION_M(MDota::setUnitState)
	USE_NETPROP_OFFSET(stateOffset, CDOTA_BaseNPC, m_nUnitState);

	PENT(unit);
	PINT(state);
	PBOL(value);

	if(!canSetState) THROW("Cannot set unit state at this time, use the Dota_UnitStatesGathered hook");

	if(value){
		*(uint32_t*)((intptr_t) unit->ent + stateOffset) |= (1 << state);
	}else{
		*(uint32_t*)((intptr_t) unit->ent + stateOffset) &= ~(1 << state);
	}

	RETURN_UNDEF;
END

FUNCTION_M(MDota::_unitInvade)
	PENT(unit);

	static ICallWrapper *g_pUnitInvade = NULL;
	if (!g_pUnitInvade){
		int offset;
		
		if (!dotaConf->GetOffset("UnitInvade", &offset)){
			THROW("\"UnitInvade\" not supported by this mod");
		}

		PassInfo pass[1];
		pass[0].type = PassType_Basic;
		pass[0].flags = PASSFLAG_BYVAL;
		pass[0].size = sizeof(char);
		
		if (!(g_pUnitInvade = binTools->CreateVCall(offset, 0, 0, NULL, pass, 1))){
			THROW("\"UnitInvade\" wrapper failed to initialized");
		}
	}

	unsigned char vstk[sizeof(void *) + sizeof(char)];
	unsigned char *vptr = vstk;
	
	*(void **)vptr = unit->ent;
	vptr += sizeof(void *);

	*(char *)vptr = 0;
	vptr += sizeof(char);

	void *ret;
	g_pUnitInvade->Execute(vstk, &ret);

	RETURN_UNDEF;
END

	const char* MDota::HeroIdToClassname(int id) {
	switch(id){
		case Hero_Base: return "npc_dota_hero_base";
		case Hero_AntiMage: return "npc_dota_hero_antimage";
		case Hero_Axe: return "npc_dota_hero_axe";
		case Hero_Bane: return "npc_dota_hero_bane";
		case Hero_Bloodseeker: return "npc_dota_hero_bloodseeker";
		case Hero_CrystalMaiden: return "npc_dota_hero_crystal_maiden";
		case Hero_DrowRanger: return "npc_dota_hero_drow_ranger";
		case Hero_EarthShaker: return "npc_dota_hero_earthshaker";
		case Hero_Juggernaut: return "npc_dota_hero_juggernaut";
		case Hero_Mirana: return "npc_dota_hero_mirana";
		case Hero_Morphling: return "npc_dota_hero_morphling";
		case Hero_ShadowFiend: return "npc_dota_hero_nevermore";
		case Hero_PhantomLancer: return "npc_dota_hero_phantom_lancer";
		case Hero_Puck: return "npc_dota_hero_puck";
		case Hero_Pudge: return "npc_dota_hero_pudge";
		case Hero_Razor: return "npc_dota_hero_razor";
		case Hero_SandKing: return "npc_dota_hero_sand_king";
		case Hero_StormSpirit: return "npc_dota_hero_storm_spirit";
		case Hero_Sven: return "npc_dota_hero_sven";
		case Hero_Tiny: return "npc_dota_hero_tiny";
		case Hero_VengefulSpirit: return "npc_dota_hero_vengefulspirit";
		case Hero_Windrunner: return "npc_dota_hero_windrunner";
		case Hero_Zeus: return "npc_dota_hero_zuus";
		case Hero_Kunkka: return "npc_dota_hero_kunkka";
		case Hero_Lina: return "npc_dota_hero_lina";
		case Hero_Lion: return "npc_dota_hero_lion";
		case Hero_ShadowShaman: return "npc_dota_hero_shadow_shaman";
		case Hero_Slardar: return "npc_dota_hero_slardar";
		case Hero_Tidehunter: return "npc_dota_hero_tidehunter";
		case Hero_WitchDoctor: return "npc_dota_hero_witch_doctor";
		case Hero_Lich: return "npc_dota_hero_lich";
		case Hero_Riki: return "npc_dota_hero_riki";
		case Hero_Enigma: return "npc_dota_hero_enigma";
		case Hero_Tinker: return "npc_dota_hero_tinker";
		case Hero_Sniper: return "npc_dota_hero_sniper";
		case Hero_Necrolyte: return "npc_dota_hero_necrolyte";
		case Hero_Warlock: return "npc_dota_hero_warlock";
		case Hero_BeastMaster: return "npc_dota_hero_beastmaster";
		case Hero_QueenOfPain: return "npc_dota_hero_queenofpain";
		case Hero_Venomancer: return "npc_dota_hero_venomancer";
		case Hero_FacelessVoid: return "npc_dota_hero_faceless_void";
		case Hero_SkeletonKing: return "npc_dota_hero_skeleton_king";
		case Hero_DeathProphet: return "npc_dota_hero_death_prophet";
		case Hero_PhantomAssassin: return "npc_dota_hero_phantom_assassin";
		case Hero_Pugna: return "npc_dota_hero_pugna";
		case Hero_TemplarAssassin: return "npc_dota_hero_templar_assassin";
		case Hero_Viper: return "npc_dota_hero_viper";
		case Hero_Luna: return "npc_dota_hero_luna";
		case Hero_DragonKnight: return "npc_dota_hero_dragon_knight";
		case Hero_Dazzle: return "npc_dota_hero_dazzle";
		case Hero_Clockwerk: return "npc_dota_hero_rattletrap";
		case Hero_Leshrac: return "npc_dota_hero_leshrac";
		case Hero_Furion: return "npc_dota_hero_furion";
		case Hero_Lifestealer: return "npc_dota_hero_life_stealer";
		case Hero_DarkSeer: return "npc_dota_hero_dark_seer";
		case Hero_Clinkz: return "npc_dota_hero_clinkz";
		case Hero_Omniknight: return "npc_dota_hero_omniknight";
		case Hero_Enchantress: return "npc_dota_hero_enchantress";
		case Hero_Huskar: return "npc_dota_hero_huskar";
		case Hero_NightStalker: return "npc_dota_hero_night_stalker";
		case Hero_Broodmother: return "npc_dota_hero_broodmother";
		case Hero_BountyHunter: return "npc_dota_hero_bounty_hunter";
		case Hero_Weaver: return "npc_dota_hero_weaver";
		case Hero_Jakiro: return "npc_dota_hero_jakiro";
		case Hero_Batrider: return "npc_dota_hero_batrider";
		case Hero_Chen: return "npc_dota_hero_chen";
		case Hero_Spectre: return "npc_dota_hero_spectre";
		case Hero_AncientApparition: return "npc_dota_hero_ancient_apparition";
		case Hero_Doom: return "npc_dota_hero_doom_bringer";
		case Hero_Ursa: return "npc_dota_hero_ursa";
		case Hero_SpiritBreaker: return "npc_dota_hero_spirit_breaker";
		case Hero_Gyrocopter: return "npc_dota_hero_gyrocopter";
		case Hero_Alchemist: return "npc_dota_hero_alchemist";
		case Hero_Invoker: return "npc_dota_hero_invoker";
		case Hero_Silencer: return "npc_dota_hero_silencer";
		case Hero_ObsidianDestroyer: return "npc_dota_hero_obsidian_destroyer";
		case Hero_Lycan: return "npc_dota_hero_lycan";
		case Hero_Brewmaster: return "npc_dota_hero_brewmaster";
		case Hero_ShadowDemon: return "npc_dota_hero_shadow_demon";
		case Hero_LoneDruid: return "npc_dota_hero_lone_druid";
		case Hero_ChaosKnight: return "npc_dota_hero_chaos_knight";
		case Hero_Meepo: return "npc_dota_hero_meepo";
		case Hero_TreantProtector: return "npc_dota_hero_treant";
		case Hero_OgreMagi: return "npc_dota_hero_ogre_magi";
		case Hero_Undying: return "npc_dota_hero_undying";
		case Hero_Rubick: return "npc_dota_hero_rubick";
		case Hero_Disruptor: return "npc_dota_hero_disruptor";
		case Hero_NyxAssassin: return "npc_dota_hero_nyx_assassin";
		case Hero_NagaSiren: return "npc_dota_hero_naga_siren";
		case Hero_KeeperOfTheLight: return "npc_dota_hero_keeper_of_the_light";
		case Hero_Wisp: return "npc_dota_hero_wisp";
		case Hero_Visage: return "npc_dota_hero_visage";
		case Hero_Slark: return "npc_dota_hero_slark";
		case Hero_Medusa: return "npc_dota_hero_medusa";
		case Hero_TrollWarlord: return "npc_dota_hero_troll_warlord";
		case Hero_CentaurWarchief: return "npc_dota_hero_centaur";
		case Hero_Magnus: return "npc_dota_hero_magnataur";
		case Hero_Timbersaw: return "npc_dota_hero_shredder";
		case Hero_Bristleback: return "npc_dota_hero_bristleback";
		case Hero_Tusk: return "npc_dota_hero_tusk";
		case Hero_SkywrathMage: return "npc_dota_hero_skywrath_mage";
		case Hero_Abaddon: return "npc_dota_hero_abaddon";
		case Hero_ElderTitan: return "npc_dota_hero_elder_titan";
		case Hero_LegionCommander: return "npc_dota_hero_legion_commander";
	}

	return NULL;
}

	