#include "abilities/battle_abilities.h"
#include "battle_data/pkmn_bank_stats.h"
#include "moves/moves.h"
#include <pokeagb/pokeagb.h>

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);

u16 rand_range(u16 min, u16 max) { return (rand() / (0xFFFF / (max - min))) + min; }

bool knows_move(u16 move_id, struct Pokemon *p) {
    u8 i;
    for (i = REQUEST_MOVE1; i < (REQUEST_MOVE1 + 4); i++) {
        u16 move = pokemon_getattr(p, i, NULL);
        if (move == move_id)
            return true;
    }
    return false;
}

void obj_delete_free_and_keep_pal(struct Object *obj) {
    obj_free_tiles_by_tag(obj);
    obj_free_rotscale_entry(obj);
    obj_delete_and_free_tiles(obj);
}

u8 get_ability(struct Pokemon *p) {
    u8 ability_bit = pokemon_getattr(p, REQUEST_ABILITY_BIT, NULL);
    return pokemon_base_stats[pokemon_getattr(p, REQUEST_SPECIES, NULL)].ability[ability_bit];
}

bool ignoring_item(struct Pokemon *p) { return (get_ability(p) == ABILITY_KLUTZ); }

bool b_pkmn_has_type(u8 bank, enum PokemonType type) {
    u8 i;
    for (i = 0; i < sizeof(p_bank[bank]->b_data.type); i++) {
        if (p_bank[bank]->b_data.type[i] == type) {
            return true;
        }
    }
    return false;
}

bool on_ground(u8 bank) {
    if (b_pkmn_has_type(bank, TYPE_FLYING) || (p_bank[bank]->b_data.ability == ABILITY_LEVITATE)) {
        if (p_bank[bank]->b_data.is_grounded)
            return true;
        return false;
    }
    return true;
}

void stat_boost(u8 bank, u8 stat_id, s8 amount) {
    if (!amount)
        return;
    if (ABILITIES_MAX > BANK_ABILITY(bank)) {
        if (abilities_table[BANK_ABILITY(bank)]->on_boost) {
            if (abilities_table[BANK_ABILITY(bank)]->on_boost(bank, amount, stat_id))
                return;
        }
    }
    extern void dprintf(const char *str, ...);
    switch (stat_id) {
    case REQUEST_ATK: {
        p_bank[bank]->b_data.attack += amount;
        break;
    }
    case REQUEST_DEF: {
        p_bank[bank]->b_data.defense += amount;
        break;
    }
    case REQUEST_SPD: {
        p_bank[bank]->b_data.speed += amount;
        break;
    }
    case REQUEST_SPATK: {
        p_bank[bank]->b_data.sp_atk += amount;
        break;
    }
    case REQUEST_SPDEF: {
        p_bank[bank]->b_data.sp_def += amount;
        break;
    }
    case 51: // evasion
    {
        p_bank[bank]->b_data.evasion += amount;
        break;
    }
    case 52: // accuracy
    {
        p_bank[bank]->b_data.accuracy += amount;
        break;
    }
    case 53: // crit
    {
        p_bank[bank]->b_data.crit_mod += amount;
        break;
    }
    default:
        dprintf("wtf, no boost men\n");
        return;
    };

    amount += 6;
    switch (amount) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        enqueue_message(0, bank, STRING_STAT_MOD_HARSH_DROP, stat_id);
        break;
    case 5:
        enqueue_message(0, bank, STRING_STAT_MOD_DROP, stat_id);
        break;
    case 6:
        break;
    case 7:
        enqueue_message(0, bank, STRING_STAT_MOD_RISE, stat_id);
        break;
    default:
        enqueue_message(0, bank, STRING_STAT_MOD_HARSH_RISE, stat_id);
        break;
    };
}

extern void status_graphical_update(u8 bank, enum Effect status);
void set_status(u8 bank, u8 source, enum Effect status) {
    bool status_applied = false;
    // lowest priority for override are types and current status
    switch (status) {
    case EFFECT_NONE:
        // clear all ailments. Cured
        p_bank[bank]->b_data.status = EFFECT_NONE;
        p_bank[bank]->b_data.status_turns = 0;
        p_bank[bank]->b_data.confusion_turns = 0;
        enqueue_message(0, bank, STRING_AILMENT_CURED, 0);
        status_graphical_update(bank, status);
        return;
        break;
    case EFFECT_PARALYZE:
        // electric types are immune. Already status'd is immune
        if ((b_pkmn_has_type(bank, TYPE_ELECTRIC)) || (p_bank[bank]->b_data.status != AILMENT_NONE)) {
            status_applied = false;
        } else {
            status_applied = true;
        }
        break;
    case EFFECT_BURN:
        // fire types are immune.  Already status'd is immune
        if ((b_pkmn_has_type(bank, TYPE_FIRE)) || (p_bank[bank]->b_data.status != AILMENT_NONE)) {
            status_applied = false;
        } else {
            status_applied = true;
        }
        break;
    case EFFECT_POISON:
    case EFFECT_BAD_POISON:
        // poison and steel types are immune. Already status'd is immune
        if ((b_pkmn_has_type(bank, TYPE_POISON)) || (b_pkmn_has_type(bank, TYPE_STEEL)) || (p_bank[bank]->b_data.status != AILMENT_NONE)) {
            status_applied = false;
        } else {
            status_applied = true;
        }
        break;
    case EFFECT_SLEEP:
        // sleep isn't affected by type
        if ((p_bank[bank]->b_data.status != AILMENT_NONE)) {
            status_applied = false;
        } else {
            status_applied = true;
        }
        break;
    case EFFECT_FREEZE:
        // ice types cannot be frozen
        if ((b_pkmn_has_type(bank, TYPE_FIRE)) || (p_bank[bank]->b_data.status != AILMENT_NONE)) {
            status_applied = false;
        } else {
            status_applied = true;
        }
        break;
    case EFFECT_CONFUSION:
        // Confusion isn't affected by type
        if ((p_bank[bank]->b_data.status != AILMENT_NONE)) {
            status_applied = false;
        } else {
            status_applied = true;
        }
        p_bank[bank]->b_data.confusion_turns = rand_range(1, 4);
        break;
    default:
        break;
    };

    // execute per side on_set_status callbacks -  things like safeguard checks should be done here
    /* TODO */

    // on set status callbacks Ability
    // the ability of target being status'd exec
    if (BANK_ABILITY(bank) < ABILITIES_MAX && abilities_table[BANK_ABILITY(bank)]->on_set_status) {
        // check if ability modified outcome of set status
        if (abilities_table[BANK_ABILITY(bank)]->on_set_status(bank, source, status, status_applied)) {
            // status would've been set in callback. Exit.
            return;
        }
    }
    // the ability of the attack user execution
    if (BANK_ABILITY(source) < ABILITIES_MAX && abilities_table[BANK_ABILITY(source)]->on_set_status) {
        // check if ability modified outcome of set status
        if (abilities_table[BANK_ABILITY(source)]->on_set_status(source, source, status, status_applied)) {
            // status would've been set in callback. Exit.
            return;
        }
    }

    if (status_applied) {
        p_bank[bank]->b_data.status = status;
        status_graphical_update(bank, status);
        enqueue_message(0, bank, STRING_AILMENT_APPLIED, status);
    } else {
        enqueue_message(0, bank, STRING_AILMENT_IMMUNE, status);
    }
}

u8 ailment_encode(u8 bank) {
    switch (p_bank[bank]->b_data.status) {
    case AILMENT_SLEEP:
        return p_bank[bank]->b_data.status_turns & 7;
    case AILMENT_POISON:
        return 1 << 3;
    case AILMENT_BURN:
        return 1 << 4;
    case AILMENT_FREEZE:
        return 1 << 5;
    case AILMENT_PARALYZE:
        return 1 << 6;
    case AILMENT_BAD_POISON:
        return 1 << 7;
    default:
        return 0;
    }
}

void ailment_decode(u8 bank, u8 ailment) {
    if ((ailment & 7) > 0) {
        p_bank[bank]->b_data.status = AILMENT_SLEEP;
        p_bank[bank]->b_data.status_turns = ailment & 7;
    } else if (ailment & (1 << 3))
        p_bank[bank]->b_data.status = AILMENT_POISON;
    else if (ailment & (1 << 4))
        p_bank[bank]->b_data.status = AILMENT_BURN;
    else if (ailment & (1 << 5))
        p_bank[bank]->b_data.status = AILMENT_FREEZE;
    else if (ailment & (1 << 6))
        p_bank[bank]->b_data.status = AILMENT_PARALYZE;
    else if (ailment & (1 << 7))
        p_bank[bank]->b_data.status = AILMENT_BAD_POISON;
}

// TODO: IMPLEMENT
void set_ability(u8 bank, u8 source, u8 new_ability) {}
