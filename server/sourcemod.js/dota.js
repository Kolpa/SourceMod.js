(function(){

var playerManager = null;

dota.MAX_PLAYERS = 24;

dota.STATE_INIT = 0;
dota.STATE_WAIT_FOR_PLAYERS_TO_LOAD = 1;
dota.STATE_HERO_SELECTION = 2;
dota.STATE_STRATEGY_TIME = 3;
dota.STATE_PRE_GAME = 4;
dota.STATE_GAME_IN_PROGRESS = 5;
dota.STATE_POST_GAME = 6;
dota.STATE_DISCONNECT = 7;

dota.TEAM_NONE = 0;
dota.TEAM_SPEC = 1;
dota.TEAM_RADIANT = 2;
dota.TEAM_DIRE = 3;
dota.TEAM_NEUTRAL = 4;


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
// ethereal: 1 + 3
// manta: 6 + 9 + 10 + 15 + 23
// brewmaster during ult: 6 + 9 + 15 + 18 + 23 + 31
// hex: 2 + 4 + 5 + 7 + 13 + 14
// void's ult: 6 + 18 + 28
// jugger during ult: 0 + 9 + 10 + 15 + 23
// OD's banish: 6 + 9 + 15 + 18 + 23 + 31
// lifestealer during ult: 0 + 2 + 3 + 8 + 9 + 10 + 15 + 19 + 23 + 25 + 31
// naga's ult: 0 + 2 + 6 + 9
// bane nightmare, first second: 0 + 2 + 6 + 9 + 12 + 23
// bane nightmare: 0 + 2 + 6 + 12
// bh invis: 2 + 3 + 6 + 8 + 9 + 12 + 13 + 16 + 17 + 20 + 21
//
// 1 and 2 prevent the unit from auto attacking
// 3 prevents units from attacking it
// 15 no health bar, cannot be clicked but can still be attacked
// 23 no health bar
// 28 gyro's Q only works if this is 0
// 30 makes the unit not give vision
/// Unit can still turn around and attack
dota.UNIT_STATE_ROOTED = 0;
// 1 +
dota.UNIT_STATE_NO_AUTOATTACKS = 2;
// 3 +
dota.UNIT_STATE_SILENCED = 4;
dota.UNIT_STATE_HEXED = 5;
dota.UNIT_STATE_STUNNED = 6;
// 7 +
dota.UNIT_STATE_INVISIBLE = 8;
dota.UNIT_STATE_INVULNERABLE = 9;
dota.UNIT_STATE_MAGIC_IMMUNE = 10;
dota.UNIT_STATE_REVEALED = 11;
// 12 sleeping? (bane)
// 13 +
// 14 +
// 15 +
// 16 +
// 17 +
dota.UNIT_STATE_PAUSED = 18;
dota.UNIT_STATE_CANT_ACT = 19;
// 20 +
// 21 +
// 22
dota.UNIT_STATE_NO_HEALTHBAR = 23;
dota.UNIT_STATE_FLYING = 24;
dota.UNIT_STATE_PHASE = 25;
// 26
// 27
// 28 +
dota.UNIT_STATE_DOMINATED = 29;
dota.UNIT_STATE_NO_VISION = 30;
dota.UNIT_STATE_BANISHED = 31;

/////////////// Unit target flags ///////////////

//dota.UNIT_TARGET_STATE_FLAG_ =                        1 <<  0;
dota.UNIT_TARGET_STATE_FLAG_RANGED =                    1 <<  1;
dota.UNIT_TARGET_STATE_FLAG_NOT_RANGED =                1 <<  2;
dota.UNIT_TARGET_STATE_FLAG_DEAD =                      1 <<  3;
dota.UNIT_TARGET_STATE_FLAG_MAGIC_IMMUNE_ENEMIES =      1 <<  4;
dota.UNIT_TARGET_STATE_FLAG_NOT_MAGIC_IMMUNE_ALLIES =   1 <<  5;
dota.UNIT_TARGET_STATE_FLAG_INVULNERABLE =              1 <<  6;
dota.UNIT_TARGET_STATE_FLAG_FOW_VISIBLE =               1 <<  7;
dota.UNIT_TARGET_STATE_FLAG_VISIBLE =                   1 <<  8;
dota.UNIT_TARGET_STATE_FLAG_NOT_ANCIENTS =              1 <<  9;
dota.UNIT_TARGET_STATE_FLAG_CONTROLLABLE_BY_PLAYERS =   1 << 10;
dota.UNIT_TARGET_STATE_FLAG_NOT_DOMINATED =             1 << 11;
dota.UNIT_TARGET_STATE_FLAG_NOT_SUMMONED =              1 << 12;
dota.UNIT_TARGET_STATE_FLAG_NOT_ILLUSIONS =             1 << 13;
dota.UNIT_TARGET_STATE_FLAG_ETHEREAL =                  1 << 14;
dota.UNIT_TARGET_STATE_FLAG_HAS_MANA =                  1 << 15;
//dota.UNIT_TARGET_STATE_FLAG_ =                        1 << 16;
dota.UNIT_TARGET_STATE_FLAG_NOT_CREEP_HERO =            1 << 17;
dota.UNIT_TARGET_STATE_FLAG_BANISHED =                  1 << 18; // filters also matches banished units
//dota.UNIT_TARGET_STATE_FLAG_ =                        1 << 19; // has unit state 12

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
dota.UNIT_TARGET_TYPE_MECHANICAL = 1 << 0;
dota.UNIT_TARGET_TYPE_GENERAL =    1 << 1;
dota.UNIT_TARGET_TYPE_BUILDINGS =  1 << 2;
dota.UNIT_TARGET_TYPE_SIEGES =     1 << 3;
dota.UNIT_TARGET_TYPE_COURIERS =   1 << 4;
dota.UNIT_TARGET_TYPE_WARDS =      1 << 5;

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
/*
game.hook("OnMapStart", function(){
	playerManager = game.findEntityByClassname(-1, "dota_player_manager");
});*/

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

Entity.prototype.isHero = function(){
	if(typeof this._isHero != 'undefined') return this._isHero;
	
	this._isHero = (dota.getUnitType(this) & dota.UNIT_TYPE_FLAG_HERO) == dota.UNIT_TYPE_FLAG_HERO;
	
	return this._isHero;
}

dota.removeAll = function(type){
	game.findEntitiesByClassname(type).forEach(function(ent){
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
}

dota.setUnitControllableByPlayer = function(ent, playerId, value){
	if(value){
		ent.netprops.m_iIsControllableByPlayer |= 1 << playerId;
	}else{
		ent.netprops.m_iIsControllableByPlayer &= ~(1 << playerId);
	}
}

})();
