#pragma once
#include <stdlib.h>
#include "mathlib\vector.h"

// Jumps to the vtable function at the given offset
#define REDIRECT_TO_VTABLE(vtable, offset) { \
		__asm { mov eax, [vtable] }; \
		__asm { mov eax, [eax + 4 * offset] }; \
		__asm { jmp eax }; \
	}

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

struct AvoidLocation {
	int dunno;
	int dunno2;
	Vector location;
};


struct ModifierCallbackResult {
	int type;
	float fValue;
	const char *szValue;

	// I know, this is stupid, blame volvo and tell them to learn how to use unions
	void Set(bool value) { type = 1; fValue = (float) value; szValue = NULL; }
	void Set(int value) { type = 1; fValue = (float) value; szValue = NULL; }
	void Set(float value) { type = 1; fValue = value; szValue = NULL; }
	void Set(const char *value) { type = 2; fValue = 0.0f; szValue = value; }
};

struct CModifierParams {
	ModifierCallbackResult *result;
	void *param;
};

// Just a base class that will redirect any non-complex virtual to the real vtable
// Simple ones are inlined here for perfomance.
class CDOTA_Buff {
public:
	CDOTA_Buff() CALL_REAL_CONSTRUCTOR(CDOTA_Buff_Constructor);
	virtual ~CDOTA_Buff() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 0);

	// Seems to be called for each state to check if it should be applied (1), removed (0), or ignored(-1)
	virtual int  CheckState(int stateID) { return -1; }

	// Probably the priority for the CheckState function
	virtual int  GetPriority(){ return 1; }

	virtual int  GetAttributes() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 3);
	virtual bool ShouldSendToClients() { return true; }
	virtual bool ShouldSendToTeam(int team) { return true; }
	virtual void AddCustomTransmiterData(CDOTAModifierBuffTableEntry &) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 6);
	virtual bool IsHidden() { return true ; }
	virtual bool IsDebuff() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 8);
	virtual bool CanParentBeAutoAttacked(){ return true; }
	virtual bool IsPurgable() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 10);
	virtual bool IsPurgeException() { return false; }
	virtual bool IsStunDebuff() { return false; }
	virtual bool AllowIllusionDuplicate() { return false; }
	virtual CDOTA_Buff* DuplicateForIllusion(CDOTA_BaseNPC*) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 14);
	virtual bool  IsPermanent() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 15);
	virtual int   RefreshTexture(){ return 0; }
	virtual void  IncrementStackCount() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 17);
	virtual void  DecrementStackCount() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 18);
	virtual int   GetStackCount() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 19);
	virtual void  SetStackCount(int) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 20);
	virtual void  OnStackCountChanged(int){};
	virtual float GetDieTime() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 22);
	virtual float GetDuration() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 23);
	virtual void* GetTexture() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 24);
	virtual float GetRemainingTime() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 25);
	virtual float GetElapsedTime() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 26);
	virtual bool  IsAura() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 27);
	virtual const char * GetModifierAura() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 28);
	virtual int   GetAuraSearchTeam() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 29);
	virtual int   GetAuraSearchType() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 30);
	virtual int   GetAuraSearchFlags() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 31);
	virtual float GetAuraRadius() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 32);
	virtual bool  GetAuraEntityReject(CDOTA_BaseNPC *){ return false; }
	virtual void  SetAuraCreateData(KeyValues *) {};
	virtual bool  IsHealingAura() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 35);
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
	virtual void SetDuration(float newDuration, bool sendBuffRefresh) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 48);
	virtual void PerformDOT(float timeElapsed) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 49);

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
	virtual void GetAvoidLocation(AvoidLocation &dest) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 59); // probably incorrect
	virtual int AvoidFilter(CDOTA_BaseNPC *){ return 1; }
	virtual float AvoidStrength(CDOTA_Bot *){ return 0.0f; }
	virtual float GetAvoidRadius(int){ return 0.0f; };
	virtual bool IsAvoidMagical() { return false; }
	virtual bool IsAvoidPhysical() { return false; }
	virtual bool IsPersistentAvoidance() { return false; }
	virtual bool IsCastAttack(){ return false; }
	virtual float GetAuraDuration() { return 0.5f; }
	virtual void DoCreate(KeyValues *) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 68);

protected:
	#define CONCAT_3_( a, b ) a##b
	#define CONCAT_2_( a, b ) CONCAT_3_( a, b )
	#define CONCAT( a, b ) CONCAT_2_( a, b )
	#define PADDING_HELPER(len, n) char CONCAT(padding, n)[len];
	#define PADDING(len) PADDING_HELPER(len, __COUNTER__)

	#define MODIFIER_CALLBACK(name) void (__thiscall *name)(const CModifierParams&)

	PADDING(176);

	// Add auto attack bonus damage
	// 176 CDOTA_Modifier_Bloodseeker_Thirst_Speed
	MODIFIER_CALLBACK(GetModifierPreAttack_BonusDamage);
	// 180
	PADDING(4);
	// 184
	MODIFIER_CALLBACK(GetModifierBaseAttack_BonusDamage);
	MODIFIER_CALLBACK(GetModifierProcAttack_BonusDamage_Physical);
	MODIFIER_CALLBACK(GetModifierProcAttack_BonusDamage_Magical);
	// 196
	PADDING(8);
	// When you auto attack something, used to do stuff like AntiMage's mana break
	// Param is target (CDOTA_BaseNPC)
	// 204 
	MODIFIER_CALLBACK(GetModifierProcAttack_Feedback);
	// 208
	PADDING(4);
	// 212
	MODIFIER_CALLBACK(GetModifierInvisibilityLevel);
	// 216
	PADDING(4);
	// 220
	MODIFIER_CALLBACK(GetModifierMoveSpeedBonus_Constant);
	MODIFIER_CALLBACK(GetModifierMoveSpeedOverride);
	MODIFIER_CALLBACK(GetModifierMoveSpeedBonus_Percentage);
	// 232
	PADDING(4);
	// 236
	MODIFIER_CALLBACK(GetModifierMoveSpeedBonus_Special_Boots);
	// 240
	PADDING(8);
	// Increase the maximum move speed
	// 248 CDOTA_Modifier_Bloodseeker_Thirst_Speed
	MODIFIER_CALLBACK(GetModifierMoveSpeed_Max);
	MODIFIER_CALLBACK(GetModifierAttackSpeedBonus_Constant);
	MODIFIER_CALLBACK(GetModifierAttackSpeedBonus_Constant_PowerTreads);
	MODIFIER_CALLBACK(GetModifierAttackSpeedBonus_Constant_Secondary);
	// 260
	PADDING(8);
	// 268
	MODIFIER_CALLBACK(GetModifierDamageOutgoing_Percentage);
	// 272
	PADDING(8);
	// 280
	MODIFIER_CALLBACK(GetModifierIncomingDamage_Percentage);
	// 288
	PADDING(4);
	// 292
	MODIFIER_CALLBACK(GetModifierEvasion_Constant);
	// 296
	PADDING(8);
	// 304
	MODIFIER_CALLBACK(GetModifierMiss_Percentage);
	MODIFIER_CALLBACK(GetModifierPhysicalArmorBonus);
	MODIFIER_CALLBACK(GetModifierPhysicalArmorBonusIllusions); // Also affects illusions
	// 316
	PADDING(8);
	// 324
	MODIFIER_CALLBACK(GetModifierMagicalResistanceBonus);
	// 328
	PADDING(4);
	// 332
	MODIFIER_CALLBACK(GetModifierMagicalResistanceDecrepifyUnique);
	// 336
	PADDING(4);
	// 340
	MODIFIER_CALLBACK(GetModifierConstantManaRegen);
	// 344
	PADDING(4);
	// 348
	MODIFIER_CALLBACK(GetModifierPercentageManaRegen);
	// 352
	PADDING(4);
	// 356
	MODIFIER_CALLBACK(GetModifierConstantHealthRegen);
	// 360
	PADDING(4);
	// 364
	MODIFIER_CALLBACK(GetModifierHealthBonus);
	MODIFIER_CALLBACK(GetModifierManaBonus);
	// 372
	PADDING(4);
	// 376
	MODIFIER_CALLBACK(GetModifierExtraHealthBonus);
	MODIFIER_CALLBACK(GetModifierBonusStats_Strength);
	MODIFIER_CALLBACK(GetModifierBonusStats_Agility);
	MODIFIER_CALLBACK(GetModifierBonusStats_Intellect);
	// 392
	PADDING(4);
	// 396
	MODIFIER_CALLBACK(GetModifierAttackRangeBonus);
	// 400
	PADDING(12);
	// 412
	MODIFIER_CALLBACK(GetModifierPreAttack_CriticalStrike);
	MODIFIER_CALLBACK(GetModifierPhysical_ConstantBlock); // Shields
	// 420
	PADDING(8);
	// 428
	MODIFIER_CALLBACK(GetOverrideAnimation);
	MODIFIER_CALLBACK(GetOverrideAnimationWeight);
	// 436
	PADDING(16);
	// 452
	MODIFIER_CALLBACK(GetBonusNightVision);
	MODIFIER_CALLBACK(GetBonusVisionPercentage);
	MODIFIER_CALLBACK(GetMinHealth);
	MODIFIER_CALLBACK(GetAbsoluteNoDamagePhysical);
	MODIFIER_CALLBACK(GetAbsoluteNoDamageMagical);
	// 472
	PADDING(4);
	// 476
	MODIFIER_CALLBACK(IsIllusion);
	MODIFIER_CALLBACK(GetModifierTurnRate_Percentage);
	MODIFIER_CALLBACK(OnAttackStart);
	MODIFIER_CALLBACK(OnAttack);
	MODIFIER_CALLBACK(OnAttackLanded);
	// 496
	PADDING(16);
	// 512
	MODIFIER_CALLBACK(OnOrder);
	// 516
	PADDING(4);
	// 520
	MODIFIER_CALLBACK(OnAbilityStart);
	MODIFIER_CALLBACK(OnAbilityExecuted); // Called when the hero casts any ability
	MODIFIER_CALLBACK(OnBreakInvisibility);
	// 528
	PADDING(16);
	// 544
	MODIFIER_CALLBACK(OnTakeDamage);
	MODIFIER_CALLBACK(OnStateChanged);
	// 552
	PADDING(4);
	// 556
	MODIFIER_CALLBACK(OnAttacked);
	MODIFIER_CALLBACK(OnDeath); // There is a param, not sure what it is, could be damage record
	// 564
	PADDING(28);
	// 592
	MODIFIER_CALLBACK(OnTooltip); // This seems to be the number that floats above an unit
	MODIFIER_CALLBACK(GetModifierModelChange);
	MODIFIER_CALLBACK(ModelScale);
	// 604
	PADDING(4);
	// 608
	MODIFIER_CALLBACK(GetActivityModifiers);
	MODIFIER_CALLBACK(GetActivityTranslationModifiers);
	MODIFIER_CALLBACK(GetAttackSound); // Param is the victim CDOTA_BaseNPC
	MODIFIER_CALLBACK(GetModifierProvidesFOWVision);

	// 616 also points to GetUnitLifetimeFraction (wards) ???

	PADDING(300);
#undef MODIFIER_CALLBACK
#undef PADDING

private:
};
