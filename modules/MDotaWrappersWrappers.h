#pragma once
#include <stdlib.h>
#include "MDotaWrappers.h"
#include "SMJS_SimpleWrapped.h"


class DMasterBuff : public CDOTA_Buff {
public:
	SMJS_Plugin *plugin;
	v8::Persistent<v8::Object> obj;
	v8::Persistent<v8::Object> ent;

	DMasterBuff(SMJS_Plugin *pl, CBaseEntity *target, v8::Handle<v8::Object> pobj) {
		plugin = pl;
		obj = v8::Persistent<v8::Object>::New(pobj);
		ent = v8::Persistent<v8::Object>::Cast(GetEntityWrapper(target)->GetWrapper(plugin));
	}

	bool IsPermanent(){ return true; }
	bool IsPurgable(){ return false; }
	bool ShouldSendToClients(){ return false; }
	bool ShouldSendToTeam(){ return false; }

#define USE_CALLBACK2(callbackName, function) { \
		auto func = obj->Get(v8::String::NewSymbol(#function)); \
		if(!func.IsEmpty() && func->IsFunction()){ \
			USE_CALLBACK(DMasterBuff, callbackName, function)\
		} \
	}

	void DeclareFunctions(){
		HandleScope handle_scope(plugin->GetIsolate());
		Context::Scope context_scope(plugin->GetContext());

		USE_CALLBACK2(GetPreAttack_BonusDamage, getPreAttackBonusDamage);
		USE_CALLBACK2(GetPreAttack_BonusDamage_PostCrit, getPreAttackBonusDamagePostCrit);
		USE_CALLBACK2(GetBaseAttack_BonusDamage, getBaseAttackBonusDamage);
		USE_CALLBACK2(GetProcAttack_BonusDamage_Physical, getProcAttackBonusDamagePhysical);
		USE_CALLBACK2(GetProcAttack_BonusDamage_Magical, getProcAttackBonusDamageMagical);
		USE_CALLBACK2(GetProcAttack_BonusDamage_Composite, getProcAttackBonusDamageComposite);
		USE_CALLBACK2(GetProcAttack_BonusDamage_Pure, getProcAttackBonusDamagePure);
		USE_CALLBACK2(GetProcAttack_Feedback, getProcAttackFeedback);
		USE_CALLBACK2(GetPostAttack, getPostAttack);
		USE_CALLBACK2(GetInvisibilityLevel, getInvisibilityLevel);
		USE_CALLBACK2(GetPersistentInvisibility, getPersistentInvisibility);
		USE_CALLBACK2(GetMoveSpeedBonus_Constant, getMoveSpeedBonusConstant);
		USE_CALLBACK2(GetMoveSpeedOverride, getMoveSpeedOverride);
		USE_CALLBACK2(GetMoveSpeedBonus_Percentage, getMoveSpeedBonusPercentage);
		USE_CALLBACK2(GetMoveSpeedBonus_PercentageUnique, getMoveSpeedBonusPercentageUnique);
		USE_CALLBACK2(GetMoveSpeedBonus_Special_Boots, getMoveSpeedBonusSpecialBoots);
		USE_CALLBACK2(GetMoveSpeed_Absolute, getMoveSpeedAbsolute);
		USE_CALLBACK2(GetMoveSpeed_Limit, getMoveSpeedLimit);
		USE_CALLBACK2(GetMoveSpeed_Max, getMoveSpeedMax);
		USE_CALLBACK2(GetAttackSpeedBonus_Constant, getAttackSpeedBonusConstant);
		USE_CALLBACK2(GetAttackSpeedBonus_Constant_PowerTreads, getAttackSpeedBonusConstantPowerTreads);
		USE_CALLBACK2(GetAttackSpeedBonus_Constant_Secondary, getAttackSpeedBonusConstantSecondary);
		USE_CALLBACK2(GetBaseAttackTimeConstant, getBaseAttackTimeConstant);
		USE_CALLBACK2(GetDamageOutgoing_Percentage, getDamageOutgoingPercentage);
		USE_CALLBACK2(GetDamageOutgoing_Percentage_Illusion, getDamageOutgoingPercentageIllusion);
		USE_CALLBACK2(GetBaseDamageOutgoing_Percentage, getBaseDamageOutgoingPercentage);
		USE_CALLBACK2(GetIncomingDamage_Percentage, getIncomingDamagePercentage);
		USE_CALLBACK2(GetIncomingPhysicalDamage_Percentage, getIncomingPhysicalDamagePercentage);
		USE_CALLBACK2(GetIncomingSpellDamage_Percentage, getIncomingSpellDamagePercentage);
		USE_CALLBACK2(GetEvasion_Constant, getEvasionConstant);
		USE_CALLBACK2(GetAvoid_Constant, getAvoidConstant);
		USE_CALLBACK2(GetAvoidSpell, getAvoidSpell);
		USE_CALLBACK2(GetMiss_Percentage, getMissPercentage);
		USE_CALLBACK2(GetPhysicalArmorBonus, getPhysicalArmorBonus);
		USE_CALLBACK2(GetPhysicalArmorBonusIllusions, getPhysicalArmorBonusIllusions);
		USE_CALLBACK2(GetPhysicalArmorBonus_Unique, getPhysicalArmorBonusUnique);
		USE_CALLBACK2(GetPhysicalArmorBonus_UniqueActive, getPhysicalArmorBonusUniqueActive);
		USE_CALLBACK2(GetMagicalResistanceBonus, getMagicalResistanceBonus);
		USE_CALLBACK2(GetMagicalResistanceItemUnique, getMagicalResistanceItemUnique);
		USE_CALLBACK2(GetMagicalResistanceDecrepifyUnique, getMagicalResistanceDecrepifyUnique);
		USE_CALLBACK2(GetBaseManaRegen, getBaseManaRegen);
		USE_CALLBACK2(GetConstantManaRegen, getConstantManaRegen);
		USE_CALLBACK2(GetConstantManaRegenUnique, getConstantManaRegenUnique);
		USE_CALLBACK2(GetPercentageManaRegen, getPercentageManaRegen);
		USE_CALLBACK2(GetTotalPercentageManaRegen, getTotalPercentageManaRegen);
		USE_CALLBACK2(GetConstantHealthRegen, getConstantHealthRegen);
		USE_CALLBACK2(GetPercentageHealthRegen, getPercentageHealthRegen);
		USE_CALLBACK2(GetHealthBonus, getHealthBonus);
		USE_CALLBACK2(GetManaBonus, getManaBonus);
		USE_CALLBACK2(GetExtraHealthBonus, getExtraHealthBonus);
		USE_CALLBACK2(GetBonusStats_Strength, getBonusStatsStrength);
		USE_CALLBACK2(GetBonusStats_Agility, getBonusStatsAgility);
		USE_CALLBACK2(GetBonusStats_Intellect, getBonusStatsIntellect);
		USE_CALLBACK2(GetAttackRangeBonus, getAttackRangeBonus);
		USE_CALLBACK2(GetReincarnation, getReincarnation);
		USE_CALLBACK2(GetRespawnTime, getRespawnTime);
		USE_CALLBACK2(GetDeathGoldCost, getDeathGoldCost);
		USE_CALLBACK2(GetPreAttack_CriticalStrike, getPreAttackCriticalStrike);
		USE_CALLBACK2(GetPhysical_ConstantBlock, getPhysicalConstantBlock);
		USE_CALLBACK2(GetTotal_ConstantBlock_UnavoidablePreArmor, getTotalConstantBlockUnavoidablePreArmor);
		USE_CALLBACK2(GetTotal_ConstantBlock, getTotalConstantBlock);
		USE_CALLBACK2(GetOverrideAnimation, getOverrideAnimation);
		USE_CALLBACK2(GetOverrideAnimationWeight, getOverrideAnimationWeight);
		USE_CALLBACK2(GetOverrideAnimationRate, getOverrideAnimationRate);
		USE_CALLBACK2(GetAbsorbSpell, getAbsorbSpell);
		USE_CALLBACK2(GetDisableAutoAttack, getDisableAutoAttack);
		USE_CALLBACK2(GetBonusDayVision, getBonusDayVision);
		USE_CALLBACK2(GetBonusNightVision, getBonusNightVision);
		USE_CALLBACK2(GetBonusVisionPercentage, getBonusVisionPercentage);
		USE_CALLBACK2(GetMinHealth, getMinHealth);
		USE_CALLBACK2(GetAbsoluteNoDamagePhysical, getAbsoluteNoDamagePhysical);
		USE_CALLBACK2(GetAbsoluteNoDamageMagical, getAbsoluteNoDamageMagical);
		USE_CALLBACK2(GetAbsoluteNoDamagePure, getAbsoluteNoDamagePure);
		USE_CALLBACK2(IsIllusion, isIllusion);
		USE_CALLBACK2(GetTurnRate_Percentage, getTurnRatePercentage);

		USE_CALLBACK2(Event_OnAttackStart, onAttackStart);
		USE_CALLBACK2(Event_OnAttack, onAttack);
		USE_CALLBACK2(Event_OnAttackLanded, onAttackLanded);
		USE_CALLBACK2(Event_OnAttackFail, onAttackFail);
		USE_CALLBACK2(Event_OnAttackAllied, onAttackAllied);
		USE_CALLBACK2(Event_OnProjectileDodge, onProjectileDodge);
		USE_CALLBACK2(Event_OnOrder, onOrder);
		USE_CALLBACK2(Event_OnUnitMoved, onUnitMoved);
		USE_CALLBACK2(Event_OnAbilityStart, onAbilityStart);
		USE_CALLBACK2(Event_OnAbilityExecuted, onAbilityExecuted);
		USE_CALLBACK2(Event_OnBreakInvisibility, onBreakInvisibility);
		USE_CALLBACK2(Event_OnAbilityEndChannel, onAbilityEndChannel);
		USE_CALLBACK2(Event_OnProcessUpgrade, onProcessUpgrade);
		USE_CALLBACK2(Event_OnRefresh, onRefresh);
		USE_CALLBACK2(Event_OnTakeDamage, onTakeDamage);
		USE_CALLBACK2(Event_OnStateChanged, onStateChanged);
		USE_CALLBACK2(Event_OnOrbEffect, onOrbEffect);
		USE_CALLBACK2(Event_OnAttacked, onAttacked);
		USE_CALLBACK2(Event_OnDeath, onDeath);
		USE_CALLBACK2(Event_OnRespawn, onRespawn);
		USE_CALLBACK2(Event_OnSpentMana, onSpentMana);
		USE_CALLBACK2(Event_OnTeleporting, onTeleporting);
		USE_CALLBACK2(Event_OnTeleported, onTeleported);
		USE_CALLBACK2(Event_OnHealthGained, onHealthGained);
		USE_CALLBACK2(Event_OnManaGained, onManaGained);
		USE_CALLBACK2(Event_OnTakeDamage_ReaperScythe, onTakeDamageReaperScythe);

		USE_CALLBACK2(GetTooltip, getTooltip);
		USE_CALLBACK2(GetModelChange, getModelChange);
		USE_CALLBACK2(GetModelScale, getModelScale);
		USE_CALLBACK2(IsScepter, isScepter);
		USE_CALLBACK2(GetActivityModifiers, getActivityModifiers);
		USE_CALLBACK2(GetActivityTranslationModifiers, getActivityTranslationModifiers);
		USE_CALLBACK2(GetAttackSound, getAttackSound);
		USE_CALLBACK2(GetUnitLifetimeFraction, getUnitLifetimeFraction);
		USE_CALLBACK2(GetProvidesFOWVision, getProvidesFOWVision);
	}

#define DEF_CALLBACK(name) \
	virtual CModifierCallbackResult& name(CModifierParams params){ \
		if(obj.IsEmpty()) return;  \
		params.result.Set(0.0f); \
		HandleScope handle_scope(plugin->GetIsolate()); \
		Context::Scope context_scope(plugin->GetContext()); \
		auto func = obj->Get(v8::String::NewSymbol(#name)); \
		if(!func.IsEmpty() && func->IsFunction()){ \
			auto res = v8::Handle<v8::Function>::Cast(func)->Call(obj, 0, NULL); \
			if(!res.IsEmpty()) { \
				if(res->IsNumber()){ \
					params.result.Set((float) res->NumberValue()); \
				} else if(res->IsString()){ \
					v8::String::Utf8Value str(res); \
					params.result.Set(*str); \
				} else if(res->IsBoolean()) { \
					params.result.Set(res->BooleanValue()); \
				} \
			} \
		} \
		return params.result; \
	}

	DEF_CALLBACK(getPreAttackBonusDamage);
	DEF_CALLBACK(getPreAttackBonusDamagePostCrit);
	DEF_CALLBACK(getBaseAttackBonusDamage);
	DEF_CALLBACK(getProcAttackBonusDamagePhysical);
	DEF_CALLBACK(getProcAttackBonusDamageMagical);
	DEF_CALLBACK(getProcAttackBonusDamageComposite);
	DEF_CALLBACK(getProcAttackBonusDamagePure);
	DEF_CALLBACK(getProcAttackFeedback);
	DEF_CALLBACK(getPostAttack);
	DEF_CALLBACK(getInvisibilityLevel);
	DEF_CALLBACK(getPersistentInvisibility);
	DEF_CALLBACK(getMoveSpeedBonusConstant);
	DEF_CALLBACK(getMoveSpeedOverride);
	DEF_CALLBACK(getMoveSpeedBonusPercentage);
	DEF_CALLBACK(getMoveSpeedBonusPercentageUnique);
	DEF_CALLBACK(getMoveSpeedBonusSpecialBoots);
	DEF_CALLBACK(getMoveSpeedAbsolute);
	DEF_CALLBACK(getMoveSpeedLimit);
	DEF_CALLBACK(getMoveSpeedMax);
	DEF_CALLBACK(getAttackSpeedBonusConstant);
	DEF_CALLBACK(getAttackSpeedBonusConstantPowerTreads);
	DEF_CALLBACK(getAttackSpeedBonusConstantSecondary);
	DEF_CALLBACK(getBaseAttackTimeConstant);
	DEF_CALLBACK(getDamageOutgoingPercentage);
	DEF_CALLBACK(getDamageOutgoingPercentageIllusion);
	DEF_CALLBACK(getBaseDamageOutgoingPercentage);
	DEF_CALLBACK(getIncomingDamagePercentage);
	DEF_CALLBACK(getIncomingPhysicalDamagePercentage);
	DEF_CALLBACK(getIncomingSpellDamagePercentage);
	DEF_CALLBACK(getEvasionConstant);
	DEF_CALLBACK(getAvoidConstant);
	DEF_CALLBACK(getAvoidSpell);
	DEF_CALLBACK(getMissPercentage);
	DEF_CALLBACK(getPhysicalArmorBonus);
	DEF_CALLBACK(getPhysicalArmorBonusIllusions);
	DEF_CALLBACK(getPhysicalArmorBonusUnique);
	DEF_CALLBACK(getPhysicalArmorBonusUniqueActive);
	DEF_CALLBACK(getMagicalResistanceBonus);
	DEF_CALLBACK(getMagicalResistanceItemUnique);
	DEF_CALLBACK(getMagicalResistanceDecrepifyUnique);
	DEF_CALLBACK(getBaseManaRegen);
	DEF_CALLBACK(getConstantManaRegen);
	DEF_CALLBACK(getConstantManaRegenUnique);
	DEF_CALLBACK(getPercentageManaRegen);
	DEF_CALLBACK(getTotalPercentageManaRegen);
	DEF_CALLBACK(getConstantHealthRegen);
	DEF_CALLBACK(getPercentageHealthRegen);
	DEF_CALLBACK(getHealthBonus);
	DEF_CALLBACK(getManaBonus);
	DEF_CALLBACK(getExtraHealthBonus);
	DEF_CALLBACK(getBonusStatsStrength);
	DEF_CALLBACK(getBonusStatsAgility);
	DEF_CALLBACK(getBonusStatsIntellect);
	DEF_CALLBACK(getAttackRangeBonus);
	DEF_CALLBACK(getReincarnation);
	DEF_CALLBACK(getRespawnTime);
	DEF_CALLBACK(getDeathGoldCost);
	DEF_CALLBACK(getPreAttackCriticalStrike);
	DEF_CALLBACK(getPhysicalConstantBlock);
	DEF_CALLBACK(getTotalConstantBlockUnavoidablePreArmor);
	DEF_CALLBACK(getTotalConstantBlock);
	DEF_CALLBACK(getOverrideAnimation);
	DEF_CALLBACK(getOverrideAnimationWeight);
	DEF_CALLBACK(getOverrideAnimationRate);
	DEF_CALLBACK(getAbsorbSpell);
	DEF_CALLBACK(getDisableAutoAttack);
	DEF_CALLBACK(getBonusDayVision);
	DEF_CALLBACK(getBonusNightVision);
	DEF_CALLBACK(getBonusVisionPercentage);
	DEF_CALLBACK(getMinHealth);
	DEF_CALLBACK(getAbsoluteNoDamagePhysical);
	DEF_CALLBACK(getAbsoluteNoDamageMagical);
	DEF_CALLBACK(getAbsoluteNoDamagePure);
	DEF_CALLBACK(isIllusion);
	DEF_CALLBACK(getTurnRatePercentage);

	DEF_CALLBACK(onAttackStart);
	DEF_CALLBACK(onAttack);
	DEF_CALLBACK(onAttackLanded);
	DEF_CALLBACK(onAttackFail);
	DEF_CALLBACK(onAttackAllied);
	DEF_CALLBACK(onProjectileDodge);
	DEF_CALLBACK(onOrder);
	DEF_CALLBACK(onUnitMoved);
	DEF_CALLBACK(onAbilityStart);
	DEF_CALLBACK(onAbilityExecuted);
	DEF_CALLBACK(onBreakInvisibility);
	DEF_CALLBACK(onAbilityEndChannel);
	DEF_CALLBACK(onProcessUpgrade);
	DEF_CALLBACK(onRefresh);
	DEF_CALLBACK(onTakeDamage);
	DEF_CALLBACK(onStateChanged);
	DEF_CALLBACK(onOrbEffect);
	DEF_CALLBACK(onAttacked);
	DEF_CALLBACK(onDeath);
	DEF_CALLBACK(onRespawn);
	DEF_CALLBACK(onSpentMana);
	DEF_CALLBACK(onTeleporting);
	DEF_CALLBACK(onTeleported);
	DEF_CALLBACK(onHealthGained);
	DEF_CALLBACK(onManaGained);
	DEF_CALLBACK(onTakeDamageReaperScythe);

	DEF_CALLBACK(getTooltip);
	DEF_CALLBACK(getModelChange);
	DEF_CALLBACK(getModelScale);
	DEF_CALLBACK(isScepter);
	DEF_CALLBACK(getActivityModifiers);
	DEF_CALLBACK(getActivityTranslationModifiers);
	DEF_CALLBACK(getAttackSound);
	DEF_CALLBACK(getUnitLifetimeFraction);
	DEF_CALLBACK(getProvidesFOWVision);

private:
	DMasterBuff();
};
