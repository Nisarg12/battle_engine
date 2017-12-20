#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern void dprintf(const char * str, ...);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern bool b_pkmn_set_type(u8 bank, enum PokemonType type);
extern bool b_pkmn_has_type(u8 bank, enum PokemonType type);
extern void set_attack_bm_inplace(u8 bank, u8 index, s8 priority);


// Secret power
u8 secret_power_on_modify(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    B_AILMENT_PROCS_CHANCE_TARGET(user) = 30;
    if (battle_master->field_state.is_electric_terrain) {
        B_AILMENT_PROCS_TARGET(user) = AILMENT_PARALYZE;
    } else if (battle_master->field_state.is_grassy_terrain) {
        B_AILMENT_PROCS_TARGET(user) = AILMENT_SLEEP;
    } else if (battle_master->field_state.is_misty_terrain) {
        B_AILMENT_PROCS_CHANCE_TARGET(user) = 0;
        B_TARGET_STAT_MOD_CHANCE(user, 3) = 30;
        B_TARGET_STAT_MOD_AMOUNT(user, 3) = -1;
    } else if (battle_master->field_state.is_psychic_terrain) {
        B_AILMENT_PROCS_CHANCE_TARGET(user) = 0;
        B_TARGET_STAT_MOD_CHANCE(user, 2) = 30;
        B_TARGET_STAT_MOD_AMOUNT(user, 2) = -1;
    } else {
        B_AILMENT_PROCS_TARGET(user) = AILMENT_PARALYZE;
    }
    return true;
}

// Floral Healing
/* Floral Healing will fail in singles */
u8 floral_healing_on_modify(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != TARGET_OF(src)) return true;
    if (SIDE_OF(user) != SIDE_OF(src)) return false; // fail case in singles
    if (battle_master->field_state.is_grassy_terrain)
        B_HEAL(user) = 67;
    return true;
}


// Camouflage
u8 camouflage_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 type = MTYPE_EGG;
    if (battle_master->field_state.is_electric_terrain) {
        type = MTYPE_ELECTRIC;
    } else if (battle_master->field_state.is_grassy_terrain) {
        type = MTYPE_GRASS;
    } else if (battle_master->field_state.is_misty_terrain) {
        type = MTYPE_FAIRY;
    } else if (battle_master->field_state.is_psychic_terrain) {
        type = MTYPE_PSYCHIC;
    } else {
        type = TYPE_NORMAL;
    }
    if (b_pkmn_has_type(user, type))
        return false;
    if (b_pkmn_set_type(user, type)) {
        enqueue_message(MOVE_CAMOUFLAGE, user, STRING_CONVERSION_TYPE, B_PKMN_TYPE(user, 0));
        return true;
    }
    return false;
}

// Nature power
u8 nature_power_on_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (battle_master->field_state.is_electric_terrain) {
        CURRENT_MOVE(user) = MOVE_THUNDERBOLT;
    } else if (battle_master->field_state.is_grassy_terrain) {
        CURRENT_MOVE(user) = MOVE_ENERGY_BALL;
    } else if (battle_master->field_state.is_misty_terrain) {
        CURRENT_MOVE(user) = MOVE_MOONBLAST;
    } else if (battle_master->field_state.is_psychic_terrain) {
        CURRENT_MOVE(user) = MOVE_PSYCHIC;
    } else {
        CURRENT_MOVE(user) = MOVE_TRI_ATTACK;
    }
    set_attack_bm_inplace(user, (user), MOVE_PRIORITY(CURRENT_MOVE(user)));
    LAST_MOVE(user) = MOVE_NATURE_POWER;
    enqueue_message(CURRENT_MOVE(user), user, STRING_ATTACK_USED, 0);
    return true;
}
