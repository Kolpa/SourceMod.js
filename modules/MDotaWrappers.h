#pragma once
#include <stdlib.h>
#include "mathlib\vector.h"
#include "sourcehook.h"
#include "sh_memfuncinfo.h"
#include "MEntities.h"

#define CALL_REAL_CONSTRUCTOR(ctor) { \
		void *self = this; \
		void *myVTable = *(void**)self; \
		__asm { mov  esi, self } \
		__asm {	call ctor } \
		*(void**)self = myVTable; \
	}

extern void *CDOTA_Buff_VTable;
extern void *CDOTA_Buff_Constructor;

class KeyValues;
class CDOTA_BaseNPC;
class CDOTAModifierBuffTableEntry;
class CDOTA_AttackRecord;
class CDOTA_Bot;
class CDOTA_Orb;

struct AvoidLocation {
	int dunno;
	int dunno2;
	Vector location;
};


struct CModifierCallbackResult {
	// I know, this is stupid, blame volvo and tell them to learn how to use unions
	CModifierCallbackResult(bool value) { type = 1; fValue = (float) value; szValue = NULL; }
	CModifierCallbackResult(int value) { type = 1; fValue = (float) value; szValue = NULL; }
	CModifierCallbackResult(float value) { type = 1; fValue = value; szValue = NULL; }
	CModifierCallbackResult(double value) { type = 1; fValue = value; szValue = NULL; }
	CModifierCallbackResult(const char *value) { type = 2; fValue = 0.0f; szValue = value; }

private:
	int type;
	float fValue;
	const char *szValue;
};

struct CModifierParams {
#ifdef WIN32
private:
	uint8 padding;
public:
#endif
	// Offsets in the mac bin, in windows there's an extra 4 byte offset

	char padding0[60];

	CDOTA_Orb *m_Orb; // 60
	char padding1[48];
	CBaseHandle m_Victim; // 112
};

struct CTakeDamageInfo {
	char padding0[8];
	float unknown8; // 8
	char padding1[8];
	float unknown20; // 20
	char padding2[8];
	float unknown32; // 32
	CBaseHandle m_Ability; // 36
	CBaseHandle m_Killer; // 40
	char padding3[20];
	int flags; // 64
};

// Just a base class that will redirect any non-complex virtual to the real vtable
// Simple ones are inlined here for perfomance (to avoid an extra jump to the real vtable).
class __declspec(novtable) CDOTA_Buff {
public:
	CDOTA_Buff() CALL_REAL_CONSTRUCTOR(CDOTA_Buff_Constructor);

	virtual void Destructor(bool unallocate);

	// Seems to be called for each state to check if it should be applied (1), removed (0), or ignored(-1)
	virtual int  CheckState(int stateID) { return -1; }

	// Probably the priority for the CheckState function
	virtual int  GetPriority(){ return 1; }

	virtual int  GetAttributes();
	virtual bool ShouldSendToClients() { return true; }
	virtual bool ShouldSendToTeam(int team) { return true; }
	virtual void AddCustomTransmiterData(CDOTAModifierBuffTableEntry &);
	virtual bool IsHidden() { return true; }
	virtual bool IsDebuff();
	virtual bool CanParentBeAutoAttacked(){ return true; }
	virtual bool IsPurgable();
	virtual bool IsPurgeException() { return false; }
	virtual bool IsStunDebuff() { return false; }
	virtual bool AllowIllusionDuplicate() { return false; }
	virtual CDOTA_Buff* DuplicateForIllusion(CDOTA_BaseNPC*);
	virtual bool  IsPermanent();
	virtual int   RefreshTexture(){ return 0; }
	virtual void  IncrementStackCount();
	virtual void  DecrementStackCount();
	virtual int   GetStackCount();
	virtual void  SetStackCount(int);
	virtual void  OnStackCountChanged(int){};
	virtual float GetDieTime();
	virtual float GetDuration();
	virtual void* GetTexture();
	virtual float GetRemainingTime();
	virtual float GetElapsedTime();
	virtual bool  IsAura();
	virtual const char * GetModifierAura();
	virtual int   GetAuraSearchTeam();
	virtual int   GetAuraSearchType();
	virtual int   GetAuraSearchFlags();
	virtual float GetAuraRadius();
	virtual bool  GetAuraEntityReject(CDOTA_BaseNPC *){ return false; }
	virtual void  SetAuraCreateData(KeyValues *) {};
	virtual bool  IsHealingAura();
	virtual int   GetCustomOrb(CDOTA_AttackRecord *) { return 0; }
	virtual int   GetOrbPriority(CDOTA_AttackRecord *) { return 1; }
	virtual void* GetOrbLabel(CDOTA_AttackRecord *) { return 0; }
	virtual int GetBuffParticle() { return 0; } // Could also be a pointer, instead of an int, not sure
	virtual bool RemoveOnDeath() { return true; }
	virtual void OnDurationExpired() {};
	virtual void OnIntervalThink() {};
	virtual void OnCreated(KeyValues *){};
	virtual void OnRefresh(KeyValues *){};
	virtual void OnDestroy(){};
	virtual bool DestroyOnExpire() { return true; }
	virtual void OnThinkInternal() {};
	virtual void SetDuration(float newDuration, bool sendBuffRefresh);
	virtual void PerformDOT(float timeElapsed);

	// Declare the GetModifier* callbacks here
	virtual void DeclareFunctions(){};

	virtual const char *GetEffectName() { return NULL; }
	virtual int GetEffectAttachType() { return 1; }
	virtual bool ShouldUseOverheadOffset() { return false; }
	virtual const char *GetStatusEffectName() { return NULL; }
	virtual int StatusEffectPriority() { return -1; }
	virtual const char *GetHeroEffectName() { return NULL; };
	virtual int HeroEffectPriority() { return -1; }
	virtual int GetNumAvoidLocations() { return 0; }
	virtual void GetAvoidLocation(AvoidLocation &dest); // probably incorrect
	virtual int AvoidFilter(CDOTA_BaseNPC *){ return 1; }
	virtual float AvoidStrength(CDOTA_Bot *){ return 0.0f; }
	virtual float GetAvoidRadius(int){ return 0.0f; };
	virtual bool IsAvoidMagical() { return false; }
	virtual bool IsAvoidPhysical() { return false; }
	virtual bool IsPersistentAvoidance() { return false; }
	virtual bool IsCastAttack(){ return false; }
	virtual float GetAuraDuration() { return 0.5f; }
	virtual void DoCreate(KeyValues *);

	inline CDOTA_BaseNPC *GetCaster(){
		extern void *GetBuffCaster;
		CDOTA_BaseNPC *result;
		CDOTA_Buff *self = this;
		__asm {
			mov eax, self
			call GetBuffCaster
			mov result, eax
		}
		return result;
	}

protected:

	#define CONCAT_3_( a, b ) a##b
	#define CONCAT_2_( a, b ) CONCAT_3_( a, b )
	#define CONCAT( a, b ) CONCAT_2_( a, b )
	#define PADDING_HELPER(len, n) char CONCAT(padding, n)[len];
	#define PADDING(len) PADDING_HELPER(len, __COUNTER__)

	#define MODIFIER_CALLBACK(name) CModifierCallbackResult (*name)(const CModifierParams &);

	// Use this macro to use a callback
	
	#define USE_CALLBACK(cls, callbackName, function) { \
		SourceHook::MemFuncInfo info; \
		SourceHook::GetFuncInfo<cls, cls, CModifierCallbackResult, const CModifierParams&>(this, &cls::function, info); \
		*(void**)&callbackName = (*(void***)this)[info.vtblindex]; \
	}
	
	/*
	#define USE_CALLBACK(cls, callbackName, function) { \
		callbackName = [](CModifierParams params) -> CModifierCallbackResult& { \
			cls *self; \
			__asm { mov self, ecx }; \
			return self->function(params); \
		}; \
	}*/

	PADDING(176);

	// Add auto attack bonus damage
	// 176 CDOTA_Modifier_Bloodseeker_Thirst_Speed
	MODIFIER_CALLBACK(GetPreAttack_BonusDamage);
	MODIFIER_CALLBACK(GetPreAttack_BonusDamage_PostCrit);
	MODIFIER_CALLBACK(GetBaseAttack_BonusDamage);
	MODIFIER_CALLBACK(GetProcAttack_BonusDamage_Physical);
	MODIFIER_CALLBACK(GetProcAttack_BonusDamage_Magical);
	MODIFIER_CALLBACK(GetProcAttack_BonusDamage_Composite);
	MODIFIER_CALLBACK(GetProcAttack_BonusDamage_Pure);
	// When you auto attack something, used to do stuff like AntiMage's mana break
	// Param is target (CDOTA_BaseNPC)
	MODIFIER_CALLBACK(GetProcAttack_Feedback);
	MODIFIER_CALLBACK(GetPostAttack);
	MODIFIER_CALLBACK(GetInvisibilityLevel);
	MODIFIER_CALLBACK(GetPersistentInvisibility);
	MODIFIER_CALLBACK(GetMoveSpeedBonus_Constant);
	MODIFIER_CALLBACK(GetMoveSpeedOverride);
	MODIFIER_CALLBACK(GetMoveSpeedBonus_Percentage);
	MODIFIER_CALLBACK(GetMoveSpeedBonus_PercentageUnique);
	MODIFIER_CALLBACK(GetMoveSpeedBonus_Special_Boots);
	MODIFIER_CALLBACK(GetMoveSpeed_Absolute);
	MODIFIER_CALLBACK(GetMoveSpeed_Limit);
	// 248 CDOTA_Modifier_Bloodseeker_Thirst_Speed
	MODIFIER_CALLBACK(GetMoveSpeed_Max);
	MODIFIER_CALLBACK(GetAttackSpeedBonus_Constant);
	MODIFIER_CALLBACK(GetAttackSpeedBonus_Constant_PowerTreads);
	MODIFIER_CALLBACK(GetAttackSpeedBonus_Constant_Secondary);
	MODIFIER_CALLBACK(GetBaseAttackTimeConstant);
	MODIFIER_CALLBACK(GetDamageOutgoing_Percentage);
	MODIFIER_CALLBACK(GetDamageOutgoing_Percentage_Illusion);
	MODIFIER_CALLBACK(GetBaseDamageOutgoing_Percentage);
	MODIFIER_CALLBACK(GetIncomingDamage_Percentage); // 280
	MODIFIER_CALLBACK(GetIncomingPhysicalDamage_Percentage); // 288
	MODIFIER_CALLBACK(GetIncomingSpellDamage_Percentage); // 292
	MODIFIER_CALLBACK(GetEvasion_Constant); // 296
	MODIFIER_CALLBACK(GetAvoid_Constant); // 300
	MODIFIER_CALLBACK(GetAvoidSpell); // 304
	MODIFIER_CALLBACK(GetMiss_Percentage); // 308
	MODIFIER_CALLBACK(GetPhysicalArmorBonus); // 312
	MODIFIER_CALLBACK(GetPhysicalArmorBonusIllusions); // 316 Also affects illusions
	MODIFIER_CALLBACK(GetPhysicalArmorBonus_Unique); // 320
	MODIFIER_CALLBACK(GetPhysicalArmorBonus_UniqueActive); // 324
	MODIFIER_CALLBACK(GetMagicalResistanceBonus); // 328
	MODIFIER_CALLBACK(GetMagicalResistanceItemUnique); // 332
	MODIFIER_CALLBACK(GetMagicalResistanceDecrepifyUnique); // 336
	MODIFIER_CALLBACK(GetBaseManaRegen); // 340
	MODIFIER_CALLBACK(GetConstantManaRegen); // 344
	MODIFIER_CALLBACK(GetConstantManaRegenUnique); // 348
	MODIFIER_CALLBACK(GetPercentageManaRegen); // 352
	MODIFIER_CALLBACK(GetTotalPercentageManaRegen); // 356
	MODIFIER_CALLBACK(GetConstantHealthRegen); // 360
	MODIFIER_CALLBACK(GetPercentageHealthRegen); // 364
	MODIFIER_CALLBACK(GetHealthBonus); // 368
	MODIFIER_CALLBACK(GetManaBonus); // 372
	MODIFIER_CALLBACK(GetExtraHealthBonus); // 376
	MODIFIER_CALLBACK(GetBonusStats_Strength); // 380
	MODIFIER_CALLBACK(GetBonusStats_Agility); // 384
	MODIFIER_CALLBACK(GetBonusStats_Intellect); // 388
	MODIFIER_CALLBACK(GetAttackRangeBonus);
	MODIFIER_CALLBACK(GetReincarnation);
	MODIFIER_CALLBACK(GetRespawnTime);
	MODIFIER_CALLBACK(GetDeathGoldCost);
	MODIFIER_CALLBACK(GetPreAttack_CriticalStrike);
	MODIFIER_CALLBACK(GetPhysical_ConstantBlock); // Shields
	MODIFIER_CALLBACK(GetTotal_ConstantBlock_UnavoidablePreArmor);
	MODIFIER_CALLBACK(GetTotal_ConstantBlock);
	MODIFIER_CALLBACK(GetOverrideAnimation);
	MODIFIER_CALLBACK(GetOverrideAnimationWeight);
	MODIFIER_CALLBACK(GetOverrideAnimationRate);
	MODIFIER_CALLBACK(GetAbsorbSpell);
	MODIFIER_CALLBACK(GetDisableAutoAttack);
	MODIFIER_CALLBACK(GetBonusDayVision);
	MODIFIER_CALLBACK(GetBonusNightVision);
	MODIFIER_CALLBACK(GetBonusVisionPercentage);
	MODIFIER_CALLBACK(GetMinHealth);
	MODIFIER_CALLBACK(GetAbsoluteNoDamagePhysical);
	MODIFIER_CALLBACK(GetAbsoluteNoDamageMagical);
	MODIFIER_CALLBACK(GetAbsoluteNoDamagePure);
	MODIFIER_CALLBACK(IsIllusion);
	MODIFIER_CALLBACK(GetTurnRate_Percentage);


	MODIFIER_CALLBACK(Event_OnAttackStart);
	MODIFIER_CALLBACK(Event_OnAttack);
	MODIFIER_CALLBACK(Event_OnAttackLanded);
	MODIFIER_CALLBACK(Event_OnAttackFail);
	MODIFIER_CALLBACK(Event_OnAttackAllied);
	MODIFIER_CALLBACK(Event_OnProjectileDodge);
	MODIFIER_CALLBACK(Event_OnOrder);
	MODIFIER_CALLBACK(Event_OnUnitMoved);
	MODIFIER_CALLBACK(Event_OnAbilityStart);
	MODIFIER_CALLBACK(Event_OnAbilityExecuted); // Called when the hero casts any ability
	MODIFIER_CALLBACK(Event_OnBreakInvisibility);
	MODIFIER_CALLBACK(Event_OnAbilityEndChannel);
	MODIFIER_CALLBACK(Event_OnProcessUpgrade);
	MODIFIER_CALLBACK(Event_OnRefresh);
	MODIFIER_CALLBACK(Event_OnTakeDamage);
	MODIFIER_CALLBACK(Event_OnStateChanged);
	MODIFIER_CALLBACK(Event_OnOrbEffect);
	MODIFIER_CALLBACK(Event_OnAttacked);
	MODIFIER_CALLBACK(Event_OnDeath); // There is a param, not sure what it is, could be damage record
	MODIFIER_CALLBACK(Event_OnRespawn);
	MODIFIER_CALLBACK(Event_OnSpentMana);
	MODIFIER_CALLBACK(Event_OnTeleporting);
	MODIFIER_CALLBACK(Event_OnTeleported);
	MODIFIER_CALLBACK(Event_OnHealthGained);
	MODIFIER_CALLBACK(Event_OnManaGained);
	MODIFIER_CALLBACK(Event_OnTakeDamage_ReaperScythe);

	MODIFIER_CALLBACK(GetTooltip); // This seems to be the number that floats above an unit
	MODIFIER_CALLBACK(GetModelChange);
	MODIFIER_CALLBACK(GetModelScale);
	MODIFIER_CALLBACK(IsScepter);
	MODIFIER_CALLBACK(GetActivityModifiers);
	MODIFIER_CALLBACK(GetActivityTranslationModifiers);
	MODIFIER_CALLBACK(GetAttackSound); // Param is the victim CDOTA_BaseNPC
	MODIFIER_CALLBACK(GetUnitLifetimeFraction);
	MODIFIER_CALLBACK(GetProvidesFOWVision);

	PADDING(300);
#undef MODIFIER_CALLBACK
#undef PADDING

private:
};


#undef redirector