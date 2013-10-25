(function(){

MAX_PLAYERS = dota.MAX_PLAYERS = 24;

dota.STATE_INIT = 0;
dota.STATE_WAIT_FOR_PLAYERS_TO_LOAD = 1;
dota.STATE_HERO_SELECTION = 2;
dota.STATE_STRATEGY_TIME = 3;
dota.STATE_PRE_GAME = 4;
dota.STATE_GAME_IN_PROGRESS = 5;
dota.STATE_POST_GAME = 6;
dota.STATE_DISCONNECT = 7;

TEAM_NONE = dota.TEAM_NONE = 0;
TEAM_SPEC = dota.TEAM_SPEC = 1;
TEAM_RADIANT = dota.TEAM_RADIANT = 2;
TEAM_DIRE = dota.TEAM_DIRE = 3;
TEAM_NEUTRAL = dota.TEAM_NEUTRAL = 4;


dota.DAMAGE_TYPE_PHYSICAL =   1 << 0;
dota.DAMAGE_TYPE_MAGICAL =    1 << 1;
dota.DAMAGE_TYPE_COMPOSITE =  1 << 2;
dota.DAMAGE_TYPE_PURE =       1 << 3;
dota.DAMAGE_TYPE_HP_REMOVAL = 1 << 4;

dota.UNIT_CAP_NO_ATTACK = 0;
dota.UNIT_CAP_MELEE_ATTACK = 1;
dota.UNIT_CAP_RANGED_ATTACK = 2;

dota.UNIT_CAP_MOVE_NONE = 0;
dota.UNIT_CAP_MOVE_GROUND = 1;
dota.UNIT_CAP_MOVE_FLY = 2;

dota.COMBAT_CLASS_ATTACK_LIGHT = 0;
dota.COMBAT_CLASS_ATTACK_HERO = 1;
dota.COMBAT_CLASS_ATTACK_BASIC = 2;
dota.COMBAT_CLASS_ATTACK_PIERCE = 3;
dota.COMBAT_CLASS_ATTACK_SIEGE = 4;

dota.COMBAT_CLASS_DEFEND_WEAK = 0;
dota.COMBAT_CLASS_DEFEND_BASIC = 1;
dota.COMBAT_CLASS_DEFEND_STRONG = 2;
dota.COMBAT_CLASS_DEFEND_STRUCTURE = 3;
dota.COMBAT_CLASS_DEFEND_HERO = 4;
dota.COMBAT_CLASS_DEFEND_SOFT = 5;

dota.UNIT_TARGET_TEAM_FRIENDLY = 1;
dota.UNIT_TARGET_TEAM_ENEMY = 2;
dota.UNIT_TARGET_TEAM_BOTH = 3;
dota.UNIT_TARGET_TEAM_CUSTOM = 4;

/////////////// Unit states ///////////////
// Use the ones on the left, the right ones are obsolete names
dota.UNIT_STATE_ROOTED = 0;
dota.UNIT_STATE_SOFT_DISARMED = 1;
dota.UNIT_STATE_DISARMED = dota.UNIT_STATE_NO_AUTOATTACKS = 2;
dota.UNIT_STATE_ATTACK_IMMUNE = dota.UNIT_STATE_CANNOT_BE_ATTACKED = 3;
dota.UNIT_STATE_SILENCED = 4;
dota.UNIT_STATE_MUTED = 5;
dota.UNIT_STATE_STUNNED = 6;
dota.UNIT_STATE_HEXED = 7;
dota.UNIT_STATE_INVISIBLE = 8;
dota.UNIT_STATE_INVULNERABLE = 9;
dota.UNIT_STATE_MAGIC_IMMUNE = 10;
dota.UNIT_STATE_PROVIDES_VISION = dota.UNIT_STATE_REVEALED = 11;
dota.UNIT_STATE_NIGHTMARED = 12;
dota.UNIT_STATE_BLOCK_DISABLED = 13;
dota.UNIT_STATE_EVADE_DISABLED = 14;
dota.UNIT_STATE_UNSELECTABLE = 15;
dota.UNIT_STATE_CANNOT_MISS = 16;
dota.UNIT_STATE_SPECIALLY_DENIABLE = 17;
dota.UNIT_STATE_FROZEN = dota.UNIT_STATE_PAUSED = 18;
dota.UNIT_STATE_COMMAND_RESTRICTED = dota.UNIT_STATE_CANT_ACT = 19;
dota.UNIT_STATE_NOT_ON_MINIMAP_FOR_ENEMIES = 20;
dota.UNIT_STATE_NOT_ON_MINIMAP = 21;
dota.UNIT_STATE_LOW_ATTACK_PRIORITY = 21;
dota.UNIT_STATE_NO_HEALTHBAR = 23;
dota.UNIT_STATE_FLYING = 24;
dota.UNIT_STATE_NO_UNIT_COLLISION = dota.UNIT_STATE_PHASE = 25;
dota.UNIT_STATE_NO_TEAM_MOVE_TO = 26;
dota.UNIT_STATE_NO_TEAM_SELECT = 27;
dota.UNIT_STATE_PASSIVES_DISABLED = 28;
dota.UNIT_STATE_DOMINATED = 29;
dota.UNIT_STATE_BLIND = dota.UNIT_STATE_NO_VISION = 30;
dota.UNIT_STATE_OUT_OF_GAME = dota.UNIT_STATE_BANISHED = 31;

/////////////// Unit target flags ///////////////

//dota.UNIT_TARGET_STATE_FLAG_ =                        1 <<  0;
dota.UNIT_TARGET_STATE_FLAG_RANGED_ONLY = dota.UNIT_TARGET_STATE_FLAG_RANGED = 1 << 1;
dota.UNIT_TARGET_STATE_FLAG_MELEE_ONLY = dota.UNIT_TARGET_STATE_FLAG_NOT_RANGED = 1 << 2;
dota.UNIT_TARGET_STATE_FLAG_DEAD = 1 << 3;
dota.UNIT_TARGET_STATE_FLAG_MAGIC_IMMUNE_ENEMIES = 1 << 4;
dota.UNIT_TARGET_STATE_FLAG_NOT_MAGIC_IMMUNE_ALLIES = 1 << 5;
dota.UNIT_TARGET_STATE_FLAG_INVULNERABLE = 1 << 6;
dota.UNIT_TARGET_STATE_FLAG_FOW_VISIBLE = 1 << 7;
dota.UNIT_TARGET_STATE_NO_INVIS = dota.UNIT_TARGET_STATE_FLAG_VISIBLE = 1 << 8;
dota.UNIT_TARGET_STATE_FLAG_NOT_ANCIENTS = 1 << 9;
dota.UNIT_TARGET_STATE_FLAG_PLAYER_CONTROLLED = dota.UNIT_TARGET_STATE_FLAG_CONTROLLABLE_BY_PLAYERS = 1 << 10;
dota.UNIT_TARGET_STATE_FLAG_NOT_DOMINATED = 1 << 11;
dota.UNIT_TARGET_STATE_FLAG_NOT_SUMMONED = 1 << 12;
dota.UNIT_TARGET_STATE_FLAG_NOT_ILLUSION = dota.UNIT_TARGET_STATE_FLAG_NOT_ILLUSIONS = 1 << 13;
dota.UNIT_TARGET_STATE_FLAG_NOT_ATTACK_IMMUNE = dota.UNIT_TARGET_STATE_FLAG_ETHEREAL = 1 << 14;
dota.UNIT_TARGET_STATE_FLAG_MANA_ONLY = dota.UNIT_TARGET_STATE_FLAG_HAS_MANA = 1 << 15;
dota.UNIT_TARGET_STATE_FLAG_CHECK_DISABLE_HELP = 1 << 16;
dota.UNIT_TARGET_STATE_FLAG_NOT_CREEP_HERO = 1 << 17;
dota.UNIT_TARGET_STATE_FLAG_OUT_OF_WORLD = dota.UNIT_TARGET_STATE_FLAG_BANISHED = 1 << 18; // filters also matches banished units
dota.UNIT_TARGET_STATE_FLAG_NOT_NIGHTMARED = 1 << 19;

/////////////// Unit type flags ///////////////
// Couriers = 0
// Heroes = 1
// Roshan is a creep and roshan
// Lane creeps are lane creeps and creeps
// Neutral creeps are only creeps
// Siege creeps = 8
// Towers = 16 + 4
// Barracks = 64 + 16
// Building = 16
// Ancient = 32 + 16

dota.UNIT_TYPE_FLAG_HERO =       1 <<  0;
dota.UNIT_TYPE_FLAG_MECHANICAL = 1 <<  1; // Seems unused
dota.UNIT_TYPE_FLAG_TOWER =      1 <<  2;
dota.UNIT_TYPE_FLAG_SIEGE =      1 <<  3;
dota.UNIT_TYPE_FLAG_BUILDING =   1 <<  4;
dota.UNIT_TYPE_FLAG_ANCIENT =    1 <<  5;
dota.UNIT_TYPE_FLAG_BARRACKS =   1 <<  6;
dota.UNIT_TYPE_FLAG_CREEP =      1 <<  7;
dota.UNIT_TYPE_FLAG_COURIER =    1 <<  8;
dota.UNIT_TYPE_FLAG_WARD =       1 <<  9;
dota.UNIT_TYPE_FLAG_LANE_CREEP = 1 << 10;
dota.UNIT_TYPE_FLAG_ROSHAN =     1 << 11;


/////////////// Unit target type flags ///////////////
dota.UNIT_TARGET_TYPE_HERO =       1 << 0;
dota.UNIT_TARGET_TYPE_CREEP =      1 << 1;
dota.UNIT_TARGET_TYPE_BUILDING =   1 << 2;
dota.UNIT_TARGET_TYPE_MECHANICAL = 1 << 3;
dota.UNIT_TARGET_TYPE_COURIER =    1 << 4;
dota.UNIT_TARGET_TYPE_OTHER =      1 << 5;
dota.UNIT_TARGET_TYPE_TREE =       1 << 6;
dota.UNIT_TARGET_TYPE_CUSTOM =     1 << 7;
dota.UNIT_TARGET_TYPE_BASIC =      1 << 8;

/////////////// executeOrders order type /////////////
dota.ORDER_TYPE_MOVE_TO_LOCATION = 1;
dota.ORDER_TYPE_MOVE_TO_UNIT = 2;
dota.ORDER_TYPE_ATTACK_MOVE = 3;
dota.ORDER_TYPE_ATTACK = 4;
dota.ORDER_TYPE_CAST_ABILITY_ON_LOCATION = 5;
dota.ORDER_TYPE_CAST_ABILITY_ON_UNIT = 6;
dota.ORDER_TYPE_CAST_ABILITY_ON_TREE = 7;
dota.ORDER_TYPE_CAST_ABILITY_NO_TARGET = 8;
dota.ORDER_TYPE_AUTO_CAST_ABILITY = 9;
dota.ORDER_TYPE_HOLD = 10;
dota.ORDER_TYPE_DROP_ITEM = 12;
dota.ORDER_TYPE_GIVE_ITEM = 13;
dota.ORDER_TYPE_PICK_UP_ITEM = 14;
dota.ORDER_TYPE_PICK_UP_RUNE = 15;
dota.ORDER_TYPE_BUY_ITEM = 16;
dota.ORDER_TYPE_AUTO_CAST_TOGGLE = 20;
dota.ORDER_TYPE_STOP = 21;

/////////////// rune types /////////////////////////
dota.RUNE_TYPE_DOUBLE_DAMAGE = 0;
dota.RUNE_TYPE_HASTE = 1;
dota.RUNE_TYPE_ILLUSION = 2;
dota.RUNE_TYPE_INVIS = 3;
dota.RUNE_TYPE_REGENERATION = 4;
			
dota.ENT_HOOK_ON_SPELL_START = 0;
dota.ENT_HOOK_ON_SPELL_START_POST = 1;
dota.ENT_HOOK_GET_MANA_COST = 2;
dota.ENT_HOOK_IS_STEALABLE = 3;
dota.ENT_HOOK_GET_CHANNEL_TIME = 4;
dota.ENT_HOOK_GET_CAST_RANGE = 5;
dota.ENT_HOOK_GET_CAST_POINT = 6;
dota.ENT_HOOK_GET_COOLDOWN = 7;
dota.ENT_HOOK_GET_ABILITY_DAMAGE = 8;
dota.ENT_HOOK_ON_ABILITY_PHASE_START = 9;
dota.ENT_HOOK_ON_ABILITY_PHASE_INTERRUPTED = 10;
dota.ENT_HOOK_ON_CHANNEL_FINISH = 11;
dota.ENT_HOOK_ON_TOGGLE = 12;
dota.ENT_HOOK_ON_STOLEN = 16;

//dota.ENT_HOOK_ON_PROJECTILE_THINK_WITH_VECTOR = 14;
//dota.ENT_HOOK_ON_PROJECTILE_THINK_WITH_INT = 15;



// The actual offset is around 0x2770, but it may change, so we store it as a relative
// offset from the closest prop we know
// This is found in the dota scripted spawner functions

var waypointOffset = game.getPropOffset("CDOTA_BaseNPC", "m_iDamageBonus") + 0x0014;
dota.setUnitWaypoint = function(unit, waypoint){
	unit.setDataEnt(waypointOffset, waypoint);
	dota._unitInvade(unit);
}

var moveCapabilitiesOffset = game.getPropOffset("CDOTA_BaseNPC", "m_iAttackCapabilities") + 0x0004;
dota.setMoveCapabilities = function(unit, cap){
	unit.setData(moveCapabilitiesOffset, 4, cap);
}

var unitTypeOffset = game.getPropOffset("CDOTA_BaseNPC", "m_iCurrentLevel") - 0x0010;
dota.getUnitType = function(unit){
	return unit.getData(unitTypeOffset, 4);
}

dota.setHeroLevel = function(hero, level){
	var levelDiff = level - hero.netprops.m_iCurrentLevel;
	if(levelDiff == 0) return;
	
	if(levelDiff > 0){
		for(var i = 0; i < levelDiff; ++i){
			dota.levelUpHero(hero, i == 0 /* playEffects only once */);
		}
	}else{
		// Deleveling a hero
		// This may be really buggy
		
		if(levelDiff != 0){
			hero.netprops.m_iCurrentLevel = level;
			hero.netprops.m_flStrength += hero.netprops.m_flStrengthGain * levelDiff;
			hero.netprops.m_flAgility += hero.netprops.m_flAgilityGain * levelDiff;
			hero.netprops.m_flIntellect += hero.netprops.m_flIntellectGain * levelDiff;
			
			hero.netprops.m_iAbilityPoints = Math.max(0, hero.netprops.m_iAbilityPoints + levelDiff);
		}
		
		var expRequired = dota.getTotalExpRequiredForLevel(level);
		if(hero.netprops.m_iCurrentXP < expRequired){
			hero.netprops.m_iCurrentXP = expRequired;
		}
	}
}

dota.findClientByPlayerID = function(playerId){
	for(var i = 0; i < server.clients.length; ++i){
		var c = server.clients[i];
		if(c == null) continue;
		if(!c.isInGame()) continue;
		if(c.netprops.m_iPlayerID === playerId) return c;
	}
	return null;
}

Client.prototype.getPlayerID = function(){
	if(this._cachedPlayerID && this._cachedPlayerID != -1) return this._cachedPlayerID;
	return this._cachedPlayerID = this.netprops.m_iPlayerID;
}

Client.prototype.getHeroes = function(){
	var hero = this.netprops.m_hAssignedHero;
	if(hero == null){
		return [];
	}
	
	// Cache the check if this client has meepo
	if(typeof this._isMeepo == 'undefined'){
		this._isMeepo = (hero.getClassname() == 'npc_dota_hero_meepo');
	}
	
	if(this._isMeepo){
		return game.findEntitiesByClassname('npc_dota_hero_meepo');
	}else{
		return [hero];
	}
}

Client.prototype.forEachHero = function(func){
	if(typeof func != "function") throw new Error("Argument must be a function");
	
	var hero = this.netprops.m_hAssignedHero;
	if(hero == null){
		return;
	}
	
	// Cache the check if this client has meepo
	if(typeof this._isMeepo == 'undefined'){
		this._isMeepo = (hero.getClassname() == 'npc_dota_hero_meepo');
	}
	
	if(this._isMeepo){
		return game.findEntitiesByClassname('npc_dota_hero_meepo').forEach(func);
	}else{
		return func(hero);
	}
}

Entity.prototype.isHero = function(){
	if(typeof this._isHero != 'undefined') return this._isHero;
	
	this._isHero = (dota.getUnitType(this) & dota.UNIT_TYPE_FLAG_HERO) == dota.UNIT_TYPE_FLAG_HERO;
	
	return this._isHero;
}

/*
// This function (getDataString) could be exploited to read anything from any part of the memory,
// we can't add it.
Entity.prototype.getUnitName = function(){
	return this.getDataString(this, 11936);
}*/

dota.removeAll = function(type){
	game.findEntitiesByClassname(type).forEach(function(ent){
		if(!ent.isValid()) return;
		dota.remove(ent);
	});
}

dota.clearMap = function(){
	dota.removeAll("ent_dota_fountain*");
	dota.removeAll("ent_dota_shop*");
	dota.removeAll("npc_dota_tower*");
	dota.removeAll("npc_dota_fort*");
	dota.removeAll("npc_dota_barracks*");
	dota.removeAll("npc_dota_creep*");
	dota.removeAll("npc_dota_building*");
	dota.removeAll("npc_dota_neutral_spawner*");
	dota.removeAll("npc_dota_roshan_spawner*");
	dota.removeAll("npc_dota_scripted_spawner*");
	dota.removeAll("npc_dota_spawner*");
	dota.removeAll("npc_dota_roshan*");
	dota.removeAll("trigger_shop*");
}

dota.setUnitControllableByPlayer = function(ent, playerId, value){
	if(value){
		ent.netprops.m_iIsControllableByPlayer |= 1 << playerId;
	}else{
		ent.netprops.m_iIsControllableByPlayer &= ~(1 << playerId);
	}
}

dota.executeOrdersEx = function(type, units, target, ability, loc){
	var oldMask = new Array(units.length);
	for (var i = 0; i < units.length; i++) {
		oldMask[i] = units[i].netprops.m_iIsControllableByPlayer;
		units[i].netprops.m_iIsControllableByPlayer = 1;
	}
	
	dota.executeOrders(0, type, units, target, ability, false, loc);
	
	for (var i = 0; i < units.length; i++) {
		units[i].netprops.m_iIsControllableByPlayer = oldMask[i];
	}
}


; // Submodules (this semicolon is required)

(function(){
	// Custom unit creator
	var hasUnitParsedHook = false;
	var creatingCustomUnit = false;
	var customUnitKV = null;
	dota.initCustomUnitHook = function(){
		if(hasUnitParsedHook) return;
		hasUnitParsedHook = true;
		
		game.hook("Dota_OnUnitParsed", customUnitCreatorHook);
	}

	dota.createCustomUnit = function(baseUnit, team, kv){
		if(!hasUnitParsedHook) throw new Error("You must initialize the cleanup hoo kwith dota.initCustomUnitHook");
		
		creatingCustomUnit = true;
		customUnitKV = kv;
		
		var unit = dota.createUnit(baseUnit, team);
		
		creatingCustomUnit = false;
		customUnitKV = null;
		
		return unit;
	}

	function customUnitCreatorHook(unit, keyvalues){
		if(creatingCustomUnit){
			for(i in customUnitKV){
				if(customUnitKV.hasOwnProperty(i)){
					if(typeof customUnitKV[i] == 'object') continue;
					
					keyvalues[i] = customUnitKV[i];
				}
			}
		}
	}
})();

(function(){
	// Unit cleanup
	
	Entity.prototype.__automaticCleanup = false;
	
	var hasCleanupHook = false;
	dota.initCleanupHook = function(){
		if(hasCleanupHook) return;
		hasCleanupHook = true;
		
		game.hook("Dota_OnUnitThink", onUnitThinkCleanup);
	}
	
	dota.autoRemoveUnit = function(ent, func){
		if(!hasCleanupHook) throw new Error("You must initialize the cleanup hoo kwith dota.initCleanupHook");
		
		ent.__automaticCleanup = true;
		ent.__automaticCleanupDelay = 30;
		ent.__automaticCleanupFunc = func;
	}
	
	function onUnitThinkCleanup(unit){
		if(unit.__automaticCleanup){
			if(unit.isValid() && unit.netprops.m_iHealth <= 0 && --unit.__automaticCleanupDelay == 0){
				dota.remove(unit);
				if(unit.__automaticCleanupFunc){
					unit.__automaticCleanupFunc();
				}
			}
		}
	}
})();

})();
