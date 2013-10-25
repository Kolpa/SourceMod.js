#include "MDotaWrappers.h"
#include "MDota.h"

// Jumps to the vtable function at the given offset
// The __declspec(naked) is required or the stack will be unbalanced
#define redirector __declspec(naked)

#define REDIRECT_TO_VTABLE(vtable, offset) { \
		__asm { mov eax, [vtable] }; \
		__asm { mov eax, [eax + 4 * offset] }; \
		__asm { jmp eax }; \
	}


// Those variables are initialized in MDota.cpp
void *CDOTA_Buff_VTable;
void *CDOTA_Buff_Constructor;


void CDOTA_Buff::Destructor(bool unallocate) {

	__asm {
		push 0
		mov eax, [CDOTA_Buff_VTable]
		mov eax, [eax]
		call eax
	}

	free(this);
}
redirector int  CDOTA_Buff::GetAttributes() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 3);
redirector void CDOTA_Buff::AddCustomTransmiterData(CDOTAModifierBuffTableEntry &) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 6);
redirector bool CDOTA_Buff::IsDebuff() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 8);
redirector bool CDOTA_Buff::IsPurgable() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 10);
redirector CDOTA_Buff* CDOTA_Buff::DuplicateForIllusion(CDOTA_BaseNPC*) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 14);
redirector bool  CDOTA_Buff::IsPermanent() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 15);
redirector void  CDOTA_Buff::IncrementStackCount() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 17);
redirector void  CDOTA_Buff::DecrementStackCount() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 18);
redirector int   CDOTA_Buff::GetStackCount() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 19);
redirector void  CDOTA_Buff::SetStackCount(int) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 20);
redirector float CDOTA_Buff::GetDieTime() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 22);
redirector float CDOTA_Buff::GetDuration() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 23);
redirector void* CDOTA_Buff::GetTexture() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 24);
redirector float CDOTA_Buff::GetRemainingTime() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 25);
redirector float CDOTA_Buff::GetElapsedTime() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 26);
redirector bool  CDOTA_Buff::IsAura() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 27);
redirector const char * CDOTA_Buff::GetModifierAura() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 28);
redirector int   CDOTA_Buff::GetAuraSearchTeam() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 29);
redirector int   CDOTA_Buff::GetAuraSearchType() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 30);
redirector int   CDOTA_Buff::GetAuraSearchFlags() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 31);
redirector float CDOTA_Buff::GetAuraRadius() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 32);
redirector bool  CDOTA_Buff::IsHealingAura() REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 35);
redirector void CDOTA_Buff::SetDuration(float newDuration, bool sendBuffRefresh) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 48);
redirector void CDOTA_Buff::PerformDOT(float timeElapsed) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 49);
redirector void CDOTA_Buff::GetAvoidLocation(AvoidLocation &dest) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 59); // probably incorrect
redirector void CDOTA_Buff::DoCreate(KeyValues *) REDIRECT_TO_VTABLE(CDOTA_Buff_VTable, 68);
