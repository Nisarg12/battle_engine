#include <pokeagb/pokeagb.h>
#include "battle_data/pkmn_bank.h"
#include "battle_data/battle_state.h"
#include "moves/moves.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "battle_abilities.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern u16 rand_range(u16, u16);
extern bool b_pkmn_has_type(u8 bank, enum PokemonType type);
extern void stat_boost(u8 bank, u8 high_stat, s8 amount);
extern void set_status(u8 bank, u8 source, enum Effect status);
extern void set_ability(u8 bank, u8 source, u8 ability);

struct b_ability empty = {
};


// Stench
void stench_on_modify_move(u8 bank, u8 tbank, u16 move)
{
    if (M_FLINCH(move)) {
        return;
    } else {
        battle_master->b_moves[(bank)].flinch = 10;
    }
}

struct b_ability b_stench = {
    .on_modify_move = stench_on_modify_move,
};


// Drizzle
void drizzle_on_switch(u8 bank)
{
    // if weather can't be changed, return
    /*
    if ((battle_master->field_state.is_raining) ||
        (battle_master->field_state.is_primordial_sea) ||
        (battle_master->field_state.is_desolate_land)) {
        return;
    }
    // set new weather
    battle_master->field_state.is_raining = 1;*/
    enqueue_message(0, 0, STRING_RAINING, 0);
}

struct b_ability b_drizzle = {
    .on_switch = drizzle_on_switch,
};


// Speed Boost
void speed_boost_on_residual(u8 bank)
{
    if (p_bank[bank]->b_data.speed < 6) {
        p_bank[bank]->b_data.speed++;
        enqueue_message(0, bank, STRING_RAINING, 0);
    }
}

struct b_ability b_speed_boost = {
    .on_residual = speed_boost_on_residual,
};


// Battle Armor
u16 battle_armor_on_critchance(u8 bank, u16 amount)
{
    if (BANK_ABILITY(TARGET_OF(bank)) == ABILITY_BATTLE_ARMOR)
        return 0;
    return amount;
}

struct b_ability b_battle_armor = {
    .on_critchance = battle_armor_on_critchance,
};


// Sturdy
u8 sturdy_tryhit(u8 bank, u8 target, u16 move)
{
    switch (move) {
        case MOVE_FISSURE:
        case MOVE_HORN_DRILL:
        case MOVE_GUILLOTINE:
        case MOVE_SHEER_COLD:
            enqueue_message(move, bank, STRING_STURDY_IMMUNE, 0);
            return false;
            break;
        default:
            break;
    };
    return true;
}

u16 sturdy_on_damage(u8 bank, u8 tbank, u16 move, u16 damage, u8 ability, u16 item)
{
    if ((B_CURRENT_HP(bank) == TOTAL_HP(bank)) &&
        (damage >= B_CURRENT_HP(bank))) {
        return 1;
    }
    return damage;
}

struct b_ability b_sturdy = {
    .on_tryhit = sturdy_tryhit,
    .on_damage = sturdy_on_damage,
};


// Damp
u8 damp_tryhit(u8 bank, u8 target, u16 move)
{
    if ((move == MOVE_SELFDESTRUCT) || (move == MOVE_EXPLOSION)) {
        enqueue_message(move, bank, STRING_DAMP_BLOCKED, 0);
        return false;
    }
    return true;
}

u16 damp_on_damage(u8 bank, u8 tbank, u16 move, u16 damage, u8 ability, u16 item)
{
    if (ability == ABILITY_AFTERMATH)
        return 0;
    return damage;
}

struct b_ability b_damp = {
    .on_tryhit = damp_tryhit,
    .on_damage = damp_on_damage,
};


// Limber
void limber_on_update(u8 bank)
{
    if (B_STATUS(bank) == AILMENT_PARALYZE) {
       p_bank[bank]->b_data.status = AILMENT_NONE;
       enqueue_message(0, bank, STRING_STATUS_CURED, 0);
    }
}

bool limber_on_set_status(u8 bank, u8 source, enum Effect effect, bool settable)
{
    if (bank == source)
        return false;
    if ((effect == EFFECT_PARALYZE) && (settable)) {
       p_bank[bank]->b_data.status = AILMENT_NONE;
       enqueue_message(0, bank, STRING_IMMUNE_ABILITY, ABILITY_LIMBER);
       return true;
    }
    return false;
}

struct b_ability b_limber = {
    .on_update = limber_on_update,
    .on_set_status = limber_on_set_status,
};


// Sand Veil
u16 sand_veil_on_evasion(u8 bank, u16 stat) {
//    if (battle_master->field_state.is_sandstorm)
        return ((stat * 25) / 100);
    return stat;
}

bool sand_veil_on_immunity(u8 bank, enum Effect effect)
{
    //if (effect == EFFECT_SANDSTORM)
    //    return true;
    return false;
}

struct b_ability b_sand_veil = {
    .on_evasion = sand_veil_on_evasion,
    .on_immunity = sand_veil_on_immunity,
};




// Static
void static_on_after_damage(u8 bank, u8 target, u16 move, u16 dmg, u8 ability, u16 item)
{
    if (MAKES_CONTACT(move, target) && (target != bank)) {
        if (rand_range(0, 100) < 30) {
            p_bank[TARGET_OF(bank)]->b_data.status = AILMENT_PARALYZE;
        }
    }
}

struct b_ability b_static = {
    .on_after_damage = static_on_after_damage,
};


// Volt Absorb
u8 volt_absorb_tryhit(u8 bank, u8 target, u16 move)
{
    if ((target != bank) &&  (MOVE_TYPE(move) == MTYPE_ELECTRIC)) {
        enqueue_message(0, bank, STRING_IMMUNE_ABILITY, ABILITY_VOLT_ABSORB);
        p_bank[target]->b_data.current_hp += (TOTAL_HP(target) / 4);
        if (B_CURRENT_HP(target) > TOTAL_HP(target))
            p_bank[target]->b_data.current_hp = TOTAL_HP(target);
        return false;
    }
    return true;
}

struct b_ability b_volt_absorb = {
    .on_tryhit = volt_absorb_tryhit,
};


// Water Absorb
u8 water_absorb_tryhit(u8 bank, u8 target, u16 move)
{
    if ((target != bank) &&  (MOVE_TYPE(move) == MTYPE_WATER)) {
        enqueue_message(0, bank, STRING_IMMUNE_ABILITY, ABILITY_WATER_ABSORB);
        p_bank[target]->b_data.current_hp += (TOTAL_HP(target) / 4);
        if (B_CURRENT_HP(target) > TOTAL_HP(target))
            p_bank[target]->b_data.current_hp = TOTAL_HP(target);
        return false;
    }
    return true;
}

struct b_ability b_water_absorb = {
    .on_tryhit = water_absorb_tryhit,
};


// Oblivious
/* Attract and taunt immunity updated in the move's tryhits */
void oblivious_on_update(u8 bank)
{

}

struct b_ability b_oblivious = {
    .on_update = oblivious_on_update,
};


// Cloud Nine
void cloud_nine_on_switch(u8 bank)
{
    /*
    battle_master->field_state.is_sunny = false;
    battle_master->field_state.is_raining = false;
    battle_master->field_state.is_sandstorm = false;
    battle_master->field_state.is_hail = false;
    battle_master->field_state.is_desolate_land = false;
    battle_master->field_state.is_primordial_sea = false;
    battle_master->field_state.suppress_weather = true;
    enqueue_message(0, 0, STRING_WEATHER_GONE, 0);
    */
}

struct b_ability b_cloud_nine = {
    .on_switch = cloud_nine_on_switch,
};


// Compound Eyes
u16 compount_eyes_on_accuracy(u8 bank, u16 stat)
{
    return (stat + ((stat * 30) / 100));
}

struct b_ability b_compound_eyes = {
    .on_accuracy = compount_eyes_on_accuracy,
};


// Insomnia
void insomnia_on_update(u8 bank)
{
    if (B_STATUS(bank) == AILMENT_SLEEP) {
       p_bank[bank]->b_data.status = AILMENT_NONE;
       enqueue_message(0, bank, STRING_STATUS_CURED, 0);
    }
}

bool insomnia_on_set_status(u8 bank, u8 source, enum Effect effect, bool settable)
{
    if (bank == source)
        return false;
    if ((effect == EFFECT_SLEEP) && (settable)) {
       p_bank[bank]->b_data.status = AILMENT_NONE;
       enqueue_message(0, bank, STRING_IMMUNE_ABILITY, ABILITY_INSOMNIA);
       return true;
    }
    return false;
}

struct b_ability b_insomnia = {
    .on_update = insomnia_on_update,
    .on_set_status = insomnia_on_set_status,
};


// Color Change
void color_change_after_move_secondary(u8 bank, u8 target, u16 move, u8 ability, u16 item)
{
    // target exists
    if (B_CURRENT_HP(TARGET_OF(bank)) && (bank != target)) {
        if (IS_MOVE_STATUS(move)) {
            return;

        // target already has this type
        } else if((B_PKMN_TYPE(bank, 0) == MOVE_TYPE(move)) ||
            (B_PKMN_TYPE(bank, 1) == MOVE_TYPE(move)) ||
            (B_PKMN_TYPE(bank, 2) == MOVE_TYPE(move))) {
            return;
        }
        // set type if unique to target and non-status move
            B_PKMN_TYPE(bank, 0) = MOVE_TYPE(move);
            enqueue_message(move, bank, STRING_GAINED_TYPE, 0);
            return;
    }
}

struct b_ability b_color_change = {
    .on_after_move_secondary = color_change_after_move_secondary,
};


// Immunity
void immunity_on_update(u8 bank)
{
    if (B_STATUS(bank) == AILMENT_SLEEP) {
       p_bank[bank]->b_data.status = AILMENT_NONE;
       enqueue_message(0, bank, STRING_STATUS_CURED, 0);
    }
}

bool immunity_on_set_status(u8 bank, u8 source, enum Effect effect, bool settable)
{
    if (bank == source)
        return false;
    if ((effect == EFFECT_POISON) && settable) {
       p_bank[bank]->b_data.status = AILMENT_NONE;
       enqueue_message(0, bank, STRING_IMMUNE_ABILITY, ABILITY_IMMUNITY);
       return true;
    }
    return false;
}

struct b_ability b_immunity = {
    .on_update = immunity_on_update,
    .on_set_status = immunity_on_set_status,
};

// Flash Fire
u8 flash_fire_tryhit(u8 bank, u8 target, u16 move)
{
    if ((target != bank) &&  (MOVE_TYPE(move) == MTYPE_FIRE)) {
        enqueue_message(0, bank, STRING_IMMUNE_ABILITY, ABILITY_FLASH_FIRE);
        ADD_VOLATILE(bank, VOLATILE_FLASH_FIRE);
        return false;
    }
    return true;
}

u16 flash_fire_on_attack(u8 bank, u16 stat)
{
    if (MOVE_TYPE(CURRENT_MOVE(bank)) == MTYPE_FIRE) {
        return ((stat * 150) / 100);
    }
    return stat;
}

struct b_ability b_flash_fire = {
    .on_tryhit = flash_fire_tryhit,
    .on_attack = flash_fire_on_attack,
    .on_sp_attack = flash_fire_on_attack, // spatk and atk are clones in functionality

};


// Shield Dust
void shield_dust_on_mod_secondary(u8 bank, u8 target, u16 move, u8 ability, u16 item)
{

}

struct b_ability b_shield_dust = {
    .on_mod_secondary = shield_dust_on_mod_secondary,
};


// Own Tempo
void own_tempo_on_update(u8 bank)
{
    if (HAS_VOLATILE(bank, VOLATILE_CONFUSION)) {
       CLEAR_VOLATILE(bank, VOLATILE_CONFUSION);
       SET_CONFUSION_TURNS(bank, 0);
       enqueue_message(0, bank, STRING_CONFUSION_ENDED, ABILITY_OWN_TEMPO);
    }
}


bool own_tempo_on_set_status(u8 bank, u8 source, enum Effect effect, bool settable)
{
    if (bank == source)
        return false;
    if ((effect == EFFECT_CONFUSION) && settable) {
       SET_CONFUSION_TURNS(bank, 0);
       enqueue_message(0, bank, STRING_IMMUNE_ABILITY, ABILITY_OWN_TEMPO);
        return true;
    }
    return false;
}

struct b_ability b_own_tempo = {
    .on_update = own_tempo_on_update,
    .on_set_status = own_tempo_on_set_status,
};


// Suction Cups
u8 suction_cups_on_dragout(u8 bank)
{
    enqueue_message(0, bank, STRING_DRAGGED_OUT_FAILED, ABILITY_SUCTION_CUPS);
    return false;
}

struct b_ability b_suction_cups = {
    .on_dragout = suction_cups_on_dragout,
};


// Intimidate
void intimidate_on_switch(u8 bank)
{
    u8 foe = (bank);
    if (B_CURRENT_HP(foe) && (!(HAS_VOLATILE(foe, VOLATILE_SUBSTITUTE) ||
        HAS_VOLATILE(foe, VOLATILE_STAT_REDUC_IMMUNE))))  {
        //enqueue_message(0, bank, STRING_INTIMIDATE, 0);
    }
}

struct b_ability b_intimidate = {
    .on_switch = intimidate_on_switch,
};


// SHADOW TAG
void shadow_tag_on_switch(u8 bank)
{
    u8 foe = (bank);
    if (!(BANK_ABILITY(foe) == ABILITY_SHADOW_TAG) && !(b_pkmn_has_type(foe, TYPE_GHOST))) {
        ADD_VOLATILE(foe, VOLATILE_TRAPPED);
    }

    if (!(BANK_ABILITY(bank) == ABILITY_SHADOW_TAG) && !(b_pkmn_has_type(bank, TYPE_GHOST))) {
        ADD_VOLATILE(bank, VOLATILE_TRAPPED);
    }
}

struct b_ability b_shadow_tag = {
    .on_switch = shadow_tag_on_switch,
};


// ROUGH SKIN
void rough_skin_on_after_damage(u8 bank, u8 target, u16 move, u16 dmg, u8 ability, u16 item)
{
    if ((bank != target) && (MAKES_CONTACT(move, target))) {
        battle_master->b_moves[(bank)].after_dmg = TOTAL_HP(target) / 8;
    }
}

struct b_ability b_rough_skin = {
    .on_after_damage = rough_skin_on_after_damage,
};


// WONDER GUARD
u8 wonder_guard_on_tryhit(u8 bank, u8 target, u16 move)
{
    if ((bank == target) || (move == MOVE_STRUGGLE) || IS_MOVE_STATUS(move))
        return true;
    enqueue_message(0, bank, STRING_IMMUNE_ABILITY, ABILITY_WONDER_GUARD);
    return false;
}

struct b_ability b_wonder_gaurd = {
    .on_tryhit = wonder_guard_on_tryhit,
};


// LEVITATE
u8 levitate_on_tryhit(u8 bank, u8 target, u16 move)
{
    /*
    if (bank != target) {
        if ((MOVE_TYPE(move) == MTYPE_GROUND) && (!(battle_master->field_state.is_gravity) && !(B_IS_GROUNDED(target))))
            return false;
    }*/
    return true;

}

struct b_ability b_levitate = {
    .on_tryhit = levitate_on_tryhit,
};


// EFFECT SPORE
void effect_spore_on_after_damage(u8 bank, u8 target, u16 move, u16 dmg_taken, u8 ability, u16 item)
{


}

// SYNCHRONIZE
bool synchronize_on_set_status(u8 bank, u8 source, enum Effect effect, bool settable)
{
    if(source == bank)
        return false;
    //if((effect == EFFECT_TOXIC_SPIKES) && settable)
    //    return false;
    if(B_STATUS(bank) == AILMENT_SLEEP || B_STATUS(bank) == AILMENT_FREEZE || B_STATUS(bank) == AILMENT_CONFUSION)
        return false;
    set_status(bank, source, B_STATUS(bank));
    return false;
}

struct b_ability b_synchronize = {
    .on_set_status = synchronize_on_set_status,
};

// CLEAR BODY

// NATURAL CURE

// LIGHTNING ROD
/* NOTE: lightning rod might change the target as well which can't be handled here */
u8 lightning_rod_on_tryhit(u8 bank, u8 t_bank, u16 move)
{
    if(bank == t_bank)
        return true;
    if(moves[move].type != MTYPE_ELECTRIC)
        return true;
    stat_boost(bank, REQUEST_SPATK, 1);
    return false;
}

struct b_ability b_lightning_rod = {
    .on_tryhit = lightning_rod_on_tryhit,
};

// SERENE GRACE
void serene_grace_on_modify_move(u8 bank, u8 target, u16 move)
{
    if(moves[move].procs != NULL)
    {

    }
}

struct b_ability b_serene_grace = {
    .on_modify_move = serene_grace_on_modify_move,
};


// SWIFT SWIM
u16 swift_swim_on_speed(u8 bank, u16 amount)
{
/*    if(battle_master->field_state.is_raining)
    {
        return amount * 2;
    }
    return amount;
*/return 0;}

struct b_ability b_swift_swim = {
    .on_speed = swift_swim_on_speed,
};

// CHLOROPHYLL
u16 chlorophyll_on_speed(u8 bank, u16 amount)
{
    /*if(battle_master->field_state.is_sunny)
    {
        return amount * 2;
    }*/
    return amount;
}

struct b_ability b_chlorophyll = {
    .on_speed = chlorophyll_on_speed,
};

// ILLUMINATE
/* NOTE: illuminate does not have an in-battle effect */
struct b_ability b_illuminate = {

};


// TRACE

static u8 trace_ignored[] = {
    ABILITY_COMATOSE,
    ABILITY_DISGUISE,
    ABILITY_FLOWER_GIFT,
    ABILITY_FORECAST,
    ABILITY_ILLUSION,
    ABILITY_IMPOSTER,
    ABILITY_MULTITYPE,
    ABILITY_SCHOOLING,
    ABILITY_STANCE_CHANGE,
    ABILITY_TRACE,
    ABILITY_ZEN_MODE
};

void trace_on_update(u8 bank)
{
    u8 enemy_ability = BANK_ABILITY((bank));
    for(u8 i = 0; i < sizeof(trace_ignored); ++i)
    {
        if(trace_ignored[i] == enemy_ability)
            return;
    }
    set_ability(bank, (bank), enemy_ability);
}

struct b_ability b_trace =
{
    .on_update = trace_on_update,
};

// HUGE POWER

// POISON POINT

// INNER FOCUS

// MAGMA ARMOR

// WATER VEIL

// MAGNET PULL

// SOUNDPROOF

// RAIN DISH

// SAND STREAM

// PRESSURE

// THICK FAT

// EARLY BIRD

// FLAME BODY

// RUN AWAY

// KEEN EYE

// HYPER CUTTER

// PICKUP

// TRUANT

// HUSTLE

// CUTE CHARM

// PLUS

// MINUS

// FORECAST

// STICKY HOLD

// SHED SKIN

// GUTS

// MARVEL SCALE

// LIQUID OOZE

// OVERGROW

// BLAZE

// TORRENT

// SWARM

// ROCK HEAD

// DROUGHT

// ARENA TRAP

// VITAL SPIRIT

// WHITE SMOKE

// PURE POWER

// SHELL ARMOR

// AIR LOCK

// TANGLED FEET

// MOTOR DRIVE

// RIVALRY

// STEADFAST

// SNOW CLOAK

// GLUTTONY

// ANGER POINT

// UNBURDEN

// HEATPROOF

// SIMPLE

// DRY SKIN

// DOWNLOAD

// IRON FIST

// POISON HEAL

// ADAPTABILITY
void adaptability_on_modify_move(u8 bank, u8 t_bank, u16 move)
{
   battle_master->b_moves[(bank)].stab = 200;
}

struct b_ability b_adaptability = {
    .on_modify_move = adaptability_on_modify_move,
};


// SKILL LINK

// HYDRATION

// SOLAR POWER

// QUICK FEET

// NORMALIZE

// SNIPER

// MAGIC GUARD

// NO GUARD

// STALL

// TECHNICIAN

// LEAF GUARD

// KLUTZ

// MOLD BREAKER

// SUPER LUCK

// AFTERMATH

// ANTICIPATION

// FOREWARN

// UNAWARE

// TINTED LENS

// FILTER

// SLOW START

// SCRAPPY

// STORM DRAIN

// ICE BODY

// SOLID ROCK

// SNOW WARNING

// HONEY GATHER

// FRISK

// RECKLESS

// MULTITYPE

// FLOWER GIFT

// BAD DREAMS

// PICKPOCKET

// SHEER FORCE

// CONTRARY

// UNNERVE

// DEFIANT

// DEFEATIST

// CURSED BODY

// HEALER

// FRIEND GUARD

// WEAK ARMOR

// HEAVY METAL

// LIGHT METAL

// MULTISCALE

// TOXIC BOOST

// FLARE BOOST

// HARVEST

// TELEPATHY

// MOODY

// OVERCOAT

// POISON TOUCH

// REGENERATOR



// BIG PECKS
bool big_pecks_on_boost(u8 bank, s8 boost_amount, u8 stat)
{
    if (boost_amount > 0)
        return true;
    if (stat == REQUEST_DEF)
        return false;
    return true;
}

struct b_ability b_big_pecks = {
    .on_boost = big_pecks_on_boost,
};


// SAND RUSH
u16 sand_rush_on_speed(u8 bank, u16 speed)
{/*
    if (battle_master->field_state.is_sandstorm) {
        return speed * 2;
    }*/
    return speed;
}

bool sand_rush_on_immunity(u8 bank, enum Effect effect)
{
    //if (effect == EFFECT_SANDSTORM)
    //    return true;
    return false;
}

struct b_ability b_sand_rush = {
    .on_speed = sand_rush_on_speed,
    .on_immunity = sand_rush_on_immunity,
};


// WONDER SKIN
u16 wonder_skin_on_accuracy(u8 bank, u16 stat)
{

    return stat;
}

struct b_ability b_wonder_skin = {
    .on_accuracy = wonder_skin_on_accuracy
};


// ANALYTIC
void analytic_on_base_power(u8 bank, u16 move)
{
    if (bank == 0)
        B_MOVE_POWER(bank) = NUM_MOD(B_MOVE_POWER(bank), 130);
}

struct b_ability b_analytic = {
    .on_base_power = analytic_on_base_power,
};


// ILLUSION
/* TODO : think of how this one would work - probably copy transform */


// IMPOSTER
/* TODO : think of how this one would work - probably copy transform */


// INFILTRATOR
void infiltrator_on_modify_move(u8 bank, u8 tbank, u16 move)
{
    B_INFILTRATES(bank) = 1;
}

struct b_ability b_infiltrator = {
    .on_modify_move = infiltrator_on_modify_move,
};


// MUMMY
void mummy_on_after_damage(u8 bank, u8 target, u16 move, u16 dmg, u8 ability, u16 item)
{
    if (MAKES_CONTACT(move, bank)) {
        BANK_ABILITY(target) = ABILITY_MUMMY;
        //enqueue_message(0, target, STRING_ABILITY_CHANGE, ABILITY_MUMMY);
    }
}

struct b_ability b_mummy = {
    .on_after_damage = mummy_on_after_damage,
};


// MOXIE
void moxie_on_faint(u8 bank, u8 fainted_bank)
{
    if (bank != fainted_bank)
        stat_boost(bank, REQUEST_ATK, 1);
}

struct b_ability b_moxie = {
    .on_faint = moxie_on_faint,
};


// JUSTIFIED
void justified_on_after_damage(u8 bank, u8 target, u16 move, u16 dmg, u8 ability, u16 item)
{
    if (B_MOVE_HAS_TYPE(target, MTYPE_DARK)) {
        stat_boost(bank, REQUEST_ATK, 1);
    }
}

struct b_ability b_justified = {
    .on_after_damage = justified_on_after_damage,
};


// RATTLED
void rattled_on_after_damage(u8 bank, u8 target, u16 move, u16 dmg, u8 ability, u16 item)
{
    if (B_MOVE_HAS_TYPE(target, MTYPE_DARK) || B_MOVE_HAS_TYPE(target, MTYPE_BUG) ||
        B_MOVE_HAS_TYPE(target, MTYPE_GHOST)) {
        stat_boost(bank, REQUEST_SPD, 1);
    }
}

struct b_ability b_rattled = {
    .on_after_damage = rattled_on_after_damage,
};


// MAGIC BOUNCE
/* TODO : variation of dancer */


// SAP SIPPER
u8 sap_sipper_tryhit(u8 bank, u8 target, u16 move)
{
    if ((target != bank) &&  (MOVE_TYPE(move) == MTYPE_GRASS)) {
        stat_boost(bank, REQUEST_ATK, 1);
        enqueue_message(0, bank, STRING_IMMUNE_ABILITY, ABILITY_SAP_SIPPER);
        return false;
    }
    return true;
}

struct b_ability b_sap_sipper = {
    .on_tryhit = sap_sipper_tryhit,
};

// PRANKSTER
s8 prankster_on_priority_mod(u8 bank, u16 move)
{
    if (battle_master->b_moves[(bank)].category == MOVE_STATUS) {
        ADD_VOLATILE(bank, VOLATILE_PRANKSTERED);
        return 1;
    }
    CLEAR_VOLATILE(bank, VOLATILE_PRANKSTERED);
    return 0;
}

void prankster_on_modify_move(u8 bank, u8 tbank, u16 move)
{
    if (B_MOVE_IS_STATUS(bank))
        ACTION_PRANKSTER = 1;
}

struct b_ability b_prankster = {
    .on_priority_mod = prankster_on_priority_mod,
    .on_modify_move = prankster_on_modify_move,
};


// SAND FORCE
void sand_force_on_base_power(u8 bank, u16 move)
{
    if (B_MOVE_HAS_TYPE(bank, MTYPE_ROCK) || B_MOVE_HAS_TYPE(bank, MTYPE_STEEL) ||
        B_MOVE_HAS_TYPE(bank, MTYPE_GROUND)) {
        B_MOVE_POWER(bank) = NUM_MOD(B_MOVE_POWER(bank), 130);
    }
}

bool sand_force_on_immunity(u8 bank, enum Effect effect)
{
    //if (effect == EFFECT_SANDSTORM)
    //    return true;
    return false;
}

struct b_ability b_sand_force = {
    .on_base_power = sand_force_on_base_power,
    .on_immunity = sand_force_on_immunity,
};


// IRON BARBS
void iron_barbs_on_after_damage(u8 bank, u8 target, u16 move, u16 dmg, u8 ability, u16 item)
{
    if ((bank != target) && (MAKES_CONTACT(move, target))) {
        battle_master->b_moves[(bank)].after_dmg = TOTAL_HP(target) / 8;
    }
}

struct b_ability b_iron_barbs = {
    .on_after_damage = iron_barbs_on_after_damage,
};


// ZEN MODE
/* TODO : Pseudo forms will be implemented later */


// VICTORY STAR
void victor_star_on_modify_move(u8 bank, u8 tbank, u16 move)
{
    B_MOVE_ACCURACY(bank) = NUM_MOD(B_MOVE_ACCURACY(bank), 110);
}

struct b_ability b_victory_star = {
    .on_modify_move = victor_star_on_modify_move,
};


// TURBOBLAZE
void turboblaze_on_switch(u8 bank)
{
    ADD_VOLATILE(bank, VOLATILE_MOLDBREAKER);
}

void turboblaze_on_modify_move(u8 bank, u8 tbank, u16 move)
{
    B_MOVE_IGNORING_ABILITIES(bank) = 1;
}

struct b_ability b_turboblaze = {
    .on_switch = turboblaze_on_switch,
    .on_modify_move = turboblaze_on_modify_move,
};


// TERAVOLT
void teravolt_on_switch(u8 bank)
{
    ADD_VOLATILE(bank, VOLATILE_MOLDBREAKER);
}

void teravolt_on_modify_move(u8 bank, u8 tbank, u16 move)
{
    B_MOVE_IGNORING_ABILITIES(bank) = 1;
}

struct b_ability b_teravolt = {
    .on_switch = teravolt_on_switch,
    .on_modify_move = teravolt_on_modify_move,
};


// AROMA VEIL
void aroma_veil_on_switch(u8 bank)
{
    ADD_VOLATILE(bank, VOLATILE_AROMA_VEIL);
    return;
}

struct b_ability b_aroma_veil = {
    .on_switch = aroma_veil_on_switch,
};


// FLOWER VEIL
bool flower_veil_on_boost(u8 bank, s8 amount, u8 stat)
{
    if ((amount < 0) && ((B_PKMN_TYPE(bank, 0) == TYPE_GRASS) || (B_PKMN_TYPE(bank, 1) == TYPE_GRASS))) {
        enqueue_message(0, bank, STRING_IMMUNE_ABILITY, ABILITY_FLOWER_VEIL);
        return false;
    }
    return true;
}

bool flower_veil_on_set_status(u8 bank, u8 source, enum Effect effect, bool settable)
{
    if (bank == source)
        return false;
    if (settable) {
       p_bank[bank]->b_data.status = AILMENT_NONE;
       enqueue_message(0, bank, STRING_IMMUNE_ABILITY, ABILITY_FLOWER_VEIL);
       return true;
    }
    return false;
}

struct b_ability b_flower_veil = {
    .on_boost = flower_veil_on_boost,
    .on_set_status = flower_veil_on_set_status,
};


// CHEEK POUCH
/* TODO item mechanics aren't completed yet */


// PROTEAN
u8 protean_on_tryhit(u8 bank, u8 t_bank, u16 move)
{
    if (BANK_ABILITY(bank) == ABILITY_PROTEAN) {
        B_PKMN_TYPE(bank, 0) = B_MOVE_TYPE(bank, 0);
        enqueue_message(0, bank, STRING_PROTEAN, 0);
        return true;
    }
    return true;
}

struct b_ability b_protean = {
    .on_tryhit = protean_on_tryhit,
};


// FUR COAT
u16 fur_coat_on_defense(u8 bank, u16 amount)
{
    return (amount * 2);
}

struct b_ability b_fur_coat = {
    .on_defense = fur_coat_on_defense,
};


// MAGICIAN
/* TODO : Make stealable items table */


// BULLETPROOF
u8 bulletproof_on_tryhit(u8 bank, u8 t_bank, u16 move)
{
    if (bank == t_bank)
        return true;
    if (IS_BULLET(move))
        return false;
    return true;
}

struct b_ability b_bulletproof = {
    .on_tryhit = bulletproof_on_tryhit,
};


// COMPETITIVE
void competitive_on_after_boost(u8 booster, s8 amount)
{
    if (amount > 0)
        return;
    stat_boost(booster, REQUEST_SPATK, 2);
}

struct b_ability b_competitive = {
    .on_after_boost = competitive_on_after_boost,
};


// STRONG JAW
void strong_jaw_on_base_power(u8 bank, u16 move)
{
    if (IS_STRONG_JAW(move))
        B_MOVE_POWER(bank) = NUM_MOD(B_MOVE_POWER(bank), 150);
}

struct b_ability b_strong_jaw = {
    .on_base_power = strong_jaw_on_base_power,
};


// REFRIGERATE
void refrigerate_on_modify_move(u8 bank, u8 tbank, u16 move)
{
    u8 i;
    for (i = 0; i < 2; i ++) {
        if ((B_MOVE_TYPE(bank, i) == MTYPE_NORMAL)) {
            B_MOVE_TYPE(bank, i) = MTYPE_ICE;
            B_MOVE_POWER(bank) = NUM_MOD(B_MOVE_POWER(bank), 120);
            return;
        }
    }
}

struct b_ability b_refrigerate = {
    .on_modify_move = refrigerate_on_modify_move,
};


// SWEET VEIL
void sweet_veil_on_update(u8 bank)
{
    if (B_STATUS(bank) == AILMENT_SLEEP) {
       p_bank[bank]->b_data.status = AILMENT_NONE;
       enqueue_message(0, bank, STRING_STATUS_CURED, 0);
    }
}

bool sweet_veil_on_set_status(u8 bank, u8 source, enum Effect effect, bool settable)
{
    if (bank == source)
        return false;
    if ((effect == EFFECT_SLEEP) && (settable)) {
       p_bank[bank]->b_data.status = AILMENT_NONE;
       enqueue_message(0, bank, STRING_IMMUNE_ABILITY, ABILITY_SWEET_VEIL);
       return true;
    }
    return false;
}

struct b_ability b_sweet_veil = {
    .on_update = sweet_veil_on_update,
    .on_set_status = sweet_veil_on_set_status,
};


// STANCE CHANGE
/* TODO : Pseudo form changes to be implemented later */


// GALE WINGS
s8 gale_wings_on_priority_mod(u8 bank, u16 move)
{
    if (MOVE_TYPE(move) == MTYPE_FLYING) {
        if (B_CURRENT_HP(bank) == TOTAL_HP(bank))
            return 1;
    }
    return 0;
}

struct b_ability b_gale_wings = {
    .on_priority_mod = gale_wings_on_priority_mod,
};


// MEGA LAUNCHER
void mega_launcher_on_base_power(u8 bank, u16 move)
{
    if (IS_PULSE(move))
        B_MOVE_POWER(bank) = NUM_MOD(B_MOVE_POWER(bank), 150);
}

struct b_ability b_mega_launcher = {
    .on_base_power = mega_launcher_on_base_power,
};


// GRASS PELT
u16 grass_pelt_on_defense(u8 bank, u16 stat)
{
    //if (battle_master->field_state.grassy_terrain)
//        return NUM_MOD(stat, 150);
    return stat;
}

struct b_ability b_grass_pelt = {
    .on_defense = grass_pelt_on_defense,
};


// SYMBIOSIS
/* TODO : No effect on singles, skipped for now */


// TOUGH CLAWS
void tough_claws_on_base_power(u8 bank, u16 move)
{
    if (MAKES_CONTACT(move, bank))
        B_MOVE_POWER(bank) = NUM_MOD(B_MOVE_POWER(bank), 130);
}

struct b_ability b_tough_claws = {
    .on_base_power = tough_claws_on_base_power,
};


// PIXILATE
void pixilate_on_modify_move(u8 bank, u8 tbank, u16 move)
{
    u8 i;
    for (i = 0; i < 2; i ++) {
        if ((B_MOVE_TYPE(bank, i) == MTYPE_FAIRY)) {
            B_MOVE_POWER(bank) = NUM_MOD(B_MOVE_POWER(bank), 120);
            return;
        }
    }
}

struct b_ability b_pixilate = {
    .on_modify_move = pixilate_on_modify_move,
};


// GOOEY
void gooey_on_after_damage(u8 bank, u8 target, u16 move, u16 dmg, u8 ability, u16 item)
{
    if ((bank == TARGET_OF(target)) && (BANK_ABILITY(bank) == ABILITY_GOOEY))
        stat_boost(target, REQUEST_SPD, -1);
}

struct b_ability b_gooey = {
    .on_after_damage = gooey_on_after_damage,
};


// AERILATE
void aerilate_on_modify_move(u8 bank, u8 tbank, u16 move)
{
    u8 i;
    for (i = 0; i < 2; i ++) {
        if ((B_MOVE_TYPE(bank, i) == MTYPE_NORMAL)) {
            B_MOVE_TYPE(bank, i) = MTYPE_FLYING;
            B_MOVE_POWER(bank) = NUM_MOD(B_MOVE_POWER(bank), 120);
            return;
        }
    }
}

struct b_ability b_aerilate = {
    .on_modify_move = aerilate_on_modify_move,
};


// PARENTAL BOND
/* TODO Almost same issue as dancer */


// DARK AURA
void dark_aura_on_switch(u8 bank)
{
    //if (battle_master->field_state.aura_break)
    //    return;
    //battle_master->field_state.dark_aura = 1;
}

void dark_aura_on_base_power(u8 bank, u16 move)
{
    if (!((B_MOVE_TYPE(bank, 0) == MTYPE_DARK) || (B_MOVE_TYPE(bank, 1) == MTYPE_DARK)))
        return;
    /*if (battle_master->field_state.aura_break) {
        B_MOVE_POWER(bank) = NUM_MOD(B_MOVE_POWER(bank), 75);
    } else if (battle_master->field_state.dark_aura) {
        B_MOVE_POWER(bank) = NUM_MOD(B_MOVE_POWER(bank), 133);
    }*/
}

struct b_ability b_dark_aura = {
    .on_switch = dark_aura_on_switch,
    .on_base_power = dark_aura_on_base_power,
};


// FAIRY AURA
void fairy_aura_on_switch(u8 bank)
{
    /*if (battle_master->field_state.aura_break)
        return;
    battle_master->field_state.fairy_aura = 1;*/
}

void fairy_aura_on_base_power(u8 bank, u16 move)
{
    /*if (!((B_MOVE_TYPE(bank, 0) == MTYPE_FAIRY) || (B_MOVE_TYPE(bank, 1) == MTYPE_FAIRY)))
        return;
    if (battle_master->field_state.aura_break) {
        B_MOVE_POWER(bank) = NUM_MOD(B_MOVE_POWER(bank), 75);
    } else if (battle_master->field_state.fairy_aura) {
        B_MOVE_POWER(bank) = NUM_MOD(B_MOVE_POWER(bank), 133);
    }*/
}

struct b_ability b_fairy_aura = {
    .on_switch = fairy_aura_on_switch,
    .on_base_power = fairy_aura_on_base_power,
};


// AURA BREAK
void aura_break_on_switch(u8 bank)
{
    //battle_master->field_state.aura_break = 1;
}

struct b_ability b_aura_break = {
    .on_switch = aura_break_on_switch,
};


// PRIMORDIAL SEA
void primordial_sea_on_switch(u8 bank)
{
    //battle_master->field_state.is_primordial_sea = 1;
    enqueue_message(0, 0, STRING_PRIMORDIAL_SEA, 0);
}

struct b_ability b_primordial_sea = {
    .on_switch = primordial_sea_on_switch,
};


// DESOLATE LAND
void desolate_land_on_switch(u8 bank)
{
//    battle_master->field_state.is_desolate_land = 1;
    enqueue_message(0, 0, STRING_DESOLATE_LAND, 0);
}

struct b_ability b_desolate_land = {
    .on_switch = desolate_land_on_switch,
};


// DELTA STREAM
void delta_stream_on_switch(u8 bank)
{
//    battle_master->field_state.is_delta_stream = 1;
    enqueue_message(0, 0, STRING_DELTA_STREAM, 0);
}

struct b_ability b_delta_stream = {
    .on_switch = delta_stream_on_switch,
};


// STAMINA
void stamina_on_after_damage(u8 bank, u8 target, u16 move, u16 dmg, u8 ability, u16 item)
{
    if ((bank == TARGET_OF(target)) && (BANK_ABILITY(bank) == ABILITY_STAMINA))
        stat_boost(bank, REQUEST_DEF, 1);
}

struct b_ability b_stamina = {
    .on_after_damage = stamina_on_after_damage,
};


// WIMP OUT
/* TODO Look at this after implementation of switching */


// EMERGENCY EXIT
/* TODO Look at this after implementation of switching */


// WATER COMPACTION
void water_compaction_after_damage(u8 bank, u8 target, u16 move, u16 dmg, u8 ability, u16 item)
{
    if ((bank != target) && (BANK_ABILITY(target) == ABILITY_WATER_COMPACTION)) {
        if ((B_MOVE_TYPE(bank, 0) == MTYPE_WATER) || (B_MOVE_TYPE(bank, 1) == MTYPE_WATER))
             stat_boost(target, REQUEST_DEF, 2);
    }
}

struct b_ability b_water_compaction = {
    .on_after_damage = water_compaction_after_damage,
};


// MERCILESS
u16 merciless_on_critchance(u8 bank, u16 amount)
{
    if ((p_bank[TARGET_OF(bank)]->b_data.status == AILMENT_BAD_POISON) ||
        (p_bank[TARGET_OF(bank)]->b_data.status == AILMENT_POISON)) {
        return 10000;
    }
    return amount;
}

struct b_ability b_merciless = {
    .on_critchance = merciless_on_critchance,
};


// SHIELDS DOWN
/* TODO : In battle pseudo forms are something planned for later */


// STAKEOUT
u16 stakeout_on_damage(u8 bank, u8 tbank, u16 move, u16 dmg, u8 ability, u16 item)
{
    if (bank == tbank)
        return dmg;
    if (p_bank[tbank]->b_data.first_turn)
        return dmg * 2;
    return dmg;
}

struct b_ability b_stakeout = {
    .on_damage = stakeout_on_damage,
};


// WATER BUBBLE
void water_bubble_on_base_power(u8 bank, u16 move)
{
    // if user's using a water type move, increase power
    if (B_MOVE_HAS_TYPE(bank, MTYPE_WATER))
        B_MOVE_POWER(bank) *= 2;
}

u16 water_bubble_on_source_base_power(u8 base_power, u8 attacker, u8 defender, u16 move)
{
    // if the foe is going to use a fire type move, reduce it's power
    if (B_MOVE_HAS_TYPE(attacker, MTYPE_FIRE))
        return NUM_MOD(B_MOVE_POWER(attacker), 50);
    return base_power;
}

bool water_bubble_on_set_status(u8 bank, u8 atkbank, enum Effect effect, bool settable)
{
    if (bank == atkbank)
        return false;
    if (effect == EFFECT_BURN) {
        p_bank[bank]->b_data.status = AILMENT_NONE;
        enqueue_message(0, bank, STRING_IMMUNE_ABILITY, ABILITY_WATER_BUBBLE);
        return true;
    }
    return false;
}

void water_bubble_on_update(u8 bank)
{
    if (B_STATUS(bank) == AILMENT_BURN) {
       p_bank[bank]->b_data.status = AILMENT_NONE;
       enqueue_message(0, bank, STRING_STATUS_CURED, 0);
    }
}

struct b_ability b_water_bubble = {
    .on_base_power = water_bubble_on_base_power,
    .on_source_base_power = water_bubble_on_source_base_power,
    .on_update = water_bubble_on_update,
};


// STEELWORKER
void steelworker_on_base_power(u8 bank, u16 move)
{
    if (MOVE_TYPE(move) == MTYPE_STEEL)
        B_MOVE_POWER(bank) = NUM_MOD(MOVE_POWER(move), 150);
}

struct b_ability b_steel_worker = {
    .on_base_power = steelworker_on_base_power,
};


// BERSERK
void berserk_on_after_move_secondary(u8 bank, u8 src, u16 move, u8 ability, u16 item)
{
    if (bank == src)
        return;
    if (B_CURRENT_HP(bank) < (TOTAL_HP(bank) >> 1)) {
        stat_boost(bank, REQUEST_SPATK, 1);
    }
}

struct b_ability b_berserk = {
    .on_after_move_secondary = berserk_on_after_move_secondary,
};


// SLUSH RUSH
u16 slush_rush_on_speed(u8 bank, u16 amount)
{
    //if (battle_master->field_state.is_hail)
    //    return amount * 2;
    return amount;
}

struct b_ability b_slush_rush = {
    .on_speed = slush_rush_on_speed,
};


// LONG REACH
void long_reach_on_modify_move(u8 bank, u8 tbank, u16 move)
{
    B_MOVE_REMOVE_CONTACT(bank) = 1;
    return;
}

struct b_ability b_long_reach = {
    .on_modify_move = long_reach_on_modify_move,
};


// LIQUID VOICE
void liquid_voice_on_modify_move(u8 bank, u8 tbank, u16 move)
{
    if (IS_SOUND_BASE(move)) {
        B_MOVE_TYPE(bank, 0) = MTYPE_WATER;
        B_MOVE_TYPE(bank, 1) = MTYPE_WATER;
    }
}

struct b_ability b_liquid_voice = {
    .on_modify_move = liquid_voice_on_modify_move,
};


// TRIAGE
s8 triage_on_priority_mod(u8 bank, u16 move)
{
    if (IS_TRIAGE(move))
        return 3;
    return 0;
}

struct b_ability b_triage = {
    .on_priority_mod = triage_on_priority_mod,
};


// GALVANIZE
void galvanize_on_modify_move(u8 bank, u8 tbank, u16 move)
{
    u8 i;
    for (i = 0; i < 2; i ++) {
        if ((B_MOVE_TYPE(bank, i) == MTYPE_NORMAL)) {
            B_MOVE_TYPE(bank, i) = MTYPE_ELECTRIC;
            B_MOVE_POWER(bank) = NUM_MOD(B_MOVE_POWER(bank), 120);
            return;
        }
    }
}

struct b_ability b_galvanize = {
    .on_modify_move = galvanize_on_modify_move,
};


// SURGE SURFER
u16 surge_surfer_on_speed(u8 bank, u16 amount)
{
    //if (battle_master->field_state.electric_terrain)
    //    return amount * 2;
    return amount;
}

struct b_ability b_surge_surfer = {
    .on_speed = surge_surfer_on_speed,
};


// SCHOOLING
/* TODO : In battle pseudo forms are something planned for later */


// DISGUISE
/* TODO : In battle pseudo forms are something planned for later */


// BATTLE BOND
/* TODO : In battle pseudo forms are something planned for later */


// POWER CONSTRUCT
/* TODO : In battle pseudo forms are something planned for later */


// CORROSION
bool corrosion_on_set_status(u8 bank, u8 source, enum Effect effect, bool settable)
{
    // if attacker has corrosion, activate.
    if ((bank == source) && ((effect == EFFECT_POISON) || (effect == EFFECT_BAD_POISON))) {
        u8 ailment = (effect == EFFECT_POISON) ? AILMENT_POISON : AILMENT_BAD_POISON;
        p_bank[TARGET_OF(bank)]->b_data.status = ailment;
        enqueue_message(0, TARGET_OF(bank), STRING_AILMENT_APPLIED, ailment);
        return true;
    }
    return false;
}

struct b_ability b_corrosion = {
    .on_set_status = corrosion_on_set_status,
};


// COMATOSE
bool comatose_on_set_status(u8 bank, u8 source, enum Effect effect, bool settable)
{
    if (bank == source)
        return false;
    // if pokemon with the ability (bank) is being set a status, provide immunity
    p_bank[bank]->b_data.status = AILMENT_NONE;
    enqueue_message(0, bank, STRING_IMMUNE_ABILITY, ABILITY_COMATOSE);
    return true;
}

struct b_ability b_comatose = {
    .on_set_status = comatose_on_set_status,
};


// QUEENLY MAJESTY
u8 queenly_majesty_on_tryhit(u8 bank, u8 t_bank, u16 move)
{
    if (SIDE_OF(bank) != SIDE_OF(t_bank)) {
        if (battle_master->b_moves[(bank)].priority != 0)
            return false;
    }
    return true;
}

struct b_ability b_queenly_majesty = {
    .on_tryhit = queenly_majesty_on_tryhit,
};


// INNARDS OUT
void innards_out_on_after_damage(u8 bank, u8 target, u16 move, u16 dmg, u8 ability, u16 item)
{
    if ((bank != target) && (!B_CURRENT_HP(target))) {
        battle_master->b_moves[(bank)].after_dmg = dmg;
    }
}

struct b_ability b_innards_out = {
    .on_after_damage = innards_out_on_after_damage,
};


// DANCER
/* TODO : This is actually a special case of running an arbitrary move outside of a given Pokemon's turn. It ought to be enqueued and run after the current move */


// BATTERY
u16 battery_on_damage(u8 bank, u8 tbank, u16 move, u16 dmg, u8 ability, u16 item)
{
    if (bank == tbank)
        return dmg;
    if (IS_MOVE_SPECIAL(move))
        return ((dmg * 130) / 100);
    return dmg;
}

struct b_ability b_battery = {
    .on_damage = battery_on_damage,
};


// FLUFFY
u16 fluffy_on_damage(u8 bank, u8 tbank, u16 move, u16 dmg, u8 ability, u16 item)
{
    if (bank == tbank)
        return dmg;
    /* 50% more damage from fire moves */
    if ((battle_master->b_moves[(bank)].type[0] == MTYPE_FIRE) ||
        (battle_master->b_moves[(bank)].type[1] == MTYPE_FIRE)) {
        dmg = ((dmg * 150) / 100);
    }
    /* half damage from contact moves */
    if (MAKES_CONTACT(move, tbank)) {
        dmg = ((dmg * 50) / 100);
    }
    return dmg;
}

struct b_ability b_fluffy = {
    .on_damage = fluffy_on_damage,
};


// DAZZLING
u8 dazzling_on_tryhit(u8 bank, u8 t_bank, u16 move)
{
    if (SIDE_OF(bank) != SIDE_OF(t_bank)) {
        if (battle_master->b_moves[(bank)].priority != 0)
            return false;
    }
    return true;
}

struct b_ability b_dazzling = {
    .on_tryhit = dazzling_on_tryhit,
};


// SOUL-HEART
void soul_heart_on_faint(u8 bank, u8 fainted_bank)
{
    if (bank != fainted_bank) {
        stat_boost(bank, REQUEST_SPATK, 1);
    }
}

struct b_ability b_soul_heart = {
    .on_faint = soul_heart_on_faint,
};

// TANGLING HAIR
void tangling_hair_on_after_damage(u8 bank, u8 target, u16 move, u16 dmg, u8 ability, u16 item)
{
    if (MAKES_CONTACT(move, target) && (target != bank)) {
        stat_boost(bank, REQUEST_SPD, -1);
    }
}

struct b_ability b_tangling_hair = {
    .on_after_damage = tangling_hair_on_after_damage,
};


// RECEIVER
/* Doesn't do anything in singles, skipped for now */


// POWER OF ALCHEMY
/* Doesn't do anything in singles, skipped for now */


// BEAST BOOST
void beast_boost_on_faint(u8 bank, u8 fainted_bank)
{
    if (bank ==  fainted_bank)
        return;
    u8 i;
    u16 current_highest = 0;
    u16 high_stat = REQUEST_ATK;
    for (i = REQUEST_ATK; i < REQUEST_ATK + 4; i++) {
        u8 this_stat = pokemon_getattr(p_bank[bank]->this_pkmn, i, NULL);
        if (this_stat > current_highest) {
            high_stat = i;
            current_highest = this_stat;
        }
    }
    stat_boost(bank, high_stat, 1);
}

struct b_ability b_beast_boost = {
    .on_faint = beast_boost_on_faint,
};

// RKS SYSTEM
struct b_ability b_RKS_system = {
    // the type change should be handled in the overworld
};

// ELECTRIC SURGE
void electric_surge_on_switch(u8 bank)
{
//    battle_master->field_state.electric_terrain = 1;
    return;
}

struct b_ability b_electric_surge = {
    .on_switch = electric_surge_on_switch,
};

// PSYCHIC SURGE
void psychic_surge_on_switch(u8 bank)
{
//    battle_master->field_state.psychic_terrain = 1;
    return;
}

struct b_ability b_psychic_surge = {
    .on_switch = psychic_surge_on_switch,
};


// MISTY SURGE
void misty_surge_on_switch(u8 bank)
{
    //battle_master->field_state.misty_terrain = 1;
    return;
}

struct b_ability b_misty_surge = {
    .on_switch = misty_surge_on_switch,
};


// GRASSY SURGE
void grassy_surge_on_switch(u8 bank)
{
//    battle_master->field_state.grassy_terrain = 1;
    return;
}

struct b_ability b_grassy_surge = {
    .on_switch = grassy_surge_on_switch,
};


// FULL METAL BODY

// SHADOW SHIELD
u16 shadow_shield_on_damage(u8 bank, u8 tbank, u16 move, u16 dmg, u8 ability, u16 item)
{
    if(bank == tbank)
        return dmg;
    if (B_CURRENT_HP(bank) == TOTAL_HP(bank))
        return dmg / 2;
    return dmg;
}

struct b_ability b_shadow_shield = {
    .on_damage = shadow_shield_on_damage,
};


// PRISM ARMOR
u16 prism_armor_on_damage(u8 bank, u8 tbank, u16 move, u16 dmg, u8 ability, u16 item)
{
    /* TODO */
    return dmg;
}

struct b_ability b_prism_armor = {
    .on_damage = prism_armor_on_damage,
};



struct b_ability* abilities_table[11] = {
    &empty,
    &b_stench,
    &b_drizzle,
    &b_speed_boost,
    &b_battle_armor,
    &b_sturdy,
    &b_damp,
    &b_limber,
    &b_sand_veil,
    &b_static,
    &b_volt_absorb,


};
