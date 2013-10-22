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
		__asm { \
			mov  esi, self \
			call CDOTA_Buff_Constructor \
		} \
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

// Just a base class that will redirect any non-complex virtual to the real vtable
// Simple ones are inlined here for perfomance.
class CDOTA_Buff {
public:
	CDOTA_Buff() CALL_REAL_CONSTRUCTOR(CDOTA_Buff_Constructor);
	virtual ~CDOTA_Buff() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 0)
	virtual int  CheckState(int unknown) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 1)
	virtual int  GetPriority(){ return 1; }
	virtual int  GetAttributes() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 3)
	virtual bool ShouldSendToClients() { return true; }
	virtual bool ShouldSendToTeam(int team) { return true; }
	virtual void AddCustomTransmiterData(CDOTAModifierBuffTableEntry &) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 6)
	virtual bool IsHidden() { return true ; }
	virtual bool IsDebuff() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 8)
	virtual bool CanParentBeAutoAttacked(){ return true; }
	virtual bool IsPurgable() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 10)
	virtual bool IsPurgeException() { return false; }
	virtual bool IsStunDebuff() { return false; }
	virtual bool AllowIllusionDuplicate() { return false; }
	virtual CDOTA_Buff* DuplicateForIllusion(CDOTA_BaseNPC*) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 14)
	virtual bool  IsPermanent() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 15)
	virtual int   RefreshTexture(){ return 0; }
	virtual void  IncrementStackCount() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 17)
	virtual void  DecrementStackCount() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 18)
	virtual int   GetStackCount() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 19)
	virtual void  SetStackCount(int) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 20)
	virtual void  OnStackCountChanged(int){};
	virtual float GetDieTime() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 22)
	virtual float GetDuration() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 23)
	virtual void* GetTexture() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 24)
	virtual float GetRemainingTime() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 25)
	virtual float GetElapsedTime() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 26)
	virtual bool  IsAura() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 27)
	virtual const char * GetModifierAura() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 28)
	virtual int   GetAuraSearchTeam() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 29)
	virtual int   GetAuraSearchType() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 30)
	virtual int   GetAuraSearchFlags() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 31)
	virtual float GetAuraRadius() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 32)
	virtual bool  GetAuraEntityReject(CDOTA_BaseNPC *){ return false; }
	virtual void  SetAuraCreateData(KeyValues *) {};
	virtual bool  IsHealingAura() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 35)
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
	virtual void SetDuration(float newDuration, bool sendBuffRefresh) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 48)
	virtual void PerformDOT(float timeElapsed) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 49)
	virtual void DeclareFunctions(){};
	virtual const char *GetEffectName() { return NULL; }
	virtual int GetEffectAttachType() { return 1; }
	virtual bool ShouldUseOverheadOffset() { return false; }
	virtual const char *GetStatusEffectName() { return NULL; }
	virtual int StatusEffectPriority() { return -1; }
	virtual const char *GetHeroEffectName() { return NULL; };
	virtual int HeroEffectPriority() { return -1; }
	virtual int GetNumAvoidLocations() { return 0; }
	virtual void GetAvoidLocation(AvoidLocation &dest) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 59) // probably incorrect
	virtual int AvoidFilter(CDOTA_BaseNPC *){ return 1; }
	virtual float AvoidStrength(CDOTA_Bot *){ return 0.0f; }
	virtual float GetAvoidRadius(int){ return 0.0f; };
	virtual bool IsAvoidMagical() { return false; }
	virtual bool IsAvoidPhysical() { return false; }
	virtual bool IsPersistentAvoidance() { return false; }
	virtual bool IsCastAttack(){ return false; }
	virtual float GetAuraDuration() { return 0.5f; }
	virtual void DoCreate(KeyValues *) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 68)

private:
	char unknown[608];

	void *GetModifierBonusStats_Intellect;
	bool m_bUnknown;

	char unknown2[300];
};
