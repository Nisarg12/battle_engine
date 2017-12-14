#include <pokeagb/pokeagb.h>
#include "battle_data/pkmn_bank.h"
#include "battle_data/pkmn_bank_stats.h"

extern void ailment_decode(u8 bank, u8 ailment);
extern u8 ailment_encode(u8 bank);
extern u8 get_ability(struct Pokemon* p);
extern void status_graphical_update(u8 bank, enum Effect status);


void update_pbank(u8 bank, struct update_flags* flags)
{
    // base stats
    u16 species = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_SPECIES, NULL);
    p_bank[bank]->b_data.weight = pokemon_get_weight(species_to_pokedex_index(species), 1) / 10;
    p_bank[bank]->b_data.species = species;
    p_bank[bank]->b_data.gender = pokemon_get_gender(p_bank[bank]->this_pkmn);
    p_bank[bank]->b_data.current_hp = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_CURRENT_HP, NULL);;
    p_bank[bank]->b_data.total_hp = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_TOTAL_HP, NULL);;
    p_bank[bank]->b_data.ability = get_ability(p_bank[bank]->this_pkmn);
    p_bank[bank]->b_data.item = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_HELD_ITEM, NULL);
    p_bank[bank]->b_data.level = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_LEVEL, NULL);
    p_bank[bank]->b_data.poke_ball = 0;
    p_bank[bank]->b_data.type[0] = pokemon_base_stats[species].type[0];
    p_bank[bank]->b_data.type[1] = pokemon_base_stats[species].type[1];
    p_bank[bank]->b_data.type[1] = (p_bank[bank]->b_data.type[1]) ? p_bank[bank]->b_data.type[1] : TYPE_NONE;
    p_bank[bank]->b_data.type[2] = TYPE_NONE;

    // pp and moves
    for (u8 i = 0; i < 4; i++) {
        p_bank[bank]->b_data.moves[i] = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_MOVE1 + i, NULL);
        p_bank[bank]->b_data.move_pp[i] = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_PP1 + i, NULL);
    }

    // raw base stats
    p_bank[bank]->b_data.attack_raw = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_ATK, NULL);
    p_bank[bank]->b_data.defense_raw = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_DEF, NULL);
    p_bank[bank]->b_data.speed_raw = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_SPD, NULL);
    p_bank[bank]->b_data.sp_atk_raw = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_SPATK, NULL);
    p_bank[bank]->b_data.sp_def_raw = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_SPDEF, NULL);

    // pokemon IVs
    p_bank[bank]->b_data.hp_iv = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_HP_IV, NULL);
    p_bank[bank]->b_data.attack_iv = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_ATK_IV, NULL);
    p_bank[bank]->b_data.defense_iv = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_DEF_IV, NULL);
    p_bank[bank]->b_data.speed_iv = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_SPD_IV, NULL);
    p_bank[bank]->b_data.sp_atk_iv = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_SPATK_IV, NULL);
    p_bank[bank]->b_data.sp_def_iv = pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_SPDEF_IV, NULL);
    p_bank[bank]->b_data.will_move = true;

    // status ailment
    ailment_decode(bank, pokemon_getattr(p_bank[bank]->this_pkmn, REQUEST_STATUS_AILMENT, NULL));
    extern void battle_start_ailment_callback_init(u8 bank);
    battle_start_ailment_callback_init(bank);
    if (!flags->pass_stats) {
        p_bank[bank]->b_data.attack = 0;
        p_bank[bank]->b_data.defense = 0;
        p_bank[bank]->b_data.speed = 0;
        p_bank[bank]->b_data.sp_atk = 0;
        p_bank[bank]->b_data.sp_def = 0;
        p_bank[bank]->b_data.accuracy = 0;
        p_bank[bank]->b_data.evasion = 0;
        p_bank[bank]->b_data.crit_mod = 0;
    }

    // user actions should always be cleared
    p_bank[bank]->b_data.is_running = 0;
    p_bank[bank]->b_data.using_item = 0;
    p_bank[bank]->b_data.is_switching = 0;
    p_bank[bank]->b_data.skip_move_select = 0;
    p_bank[bank]->b_data.first_turn = 1;

    if (!flags->pass_atk_history) {
        p_bank[bank]->b_data.my_target = 0xFF;
        p_bank[bank]->b_data.last_move = 0;
        p_bank[bank]->b_data.current_move = 0;
        p_bank[bank]->b_data.last_damage = 0;
        p_bank[bank]->b_data.last_attacked_by = 0xFF;
        p_bank[bank]->b_data.last_used_item = 0;
        p_bank[bank]->b_data.ate_berry = 0;
    }

    if (!flags->pass_status) {
        p_bank[bank]->b_data.status = 0;
        p_bank[bank]->b_data.confusion_turns = 0;
        p_bank[bank]->b_data.status_turns = 0;
        p_bank[bank]->b_data.substitute_health = 0;
        p_bank[bank]->b_data.v_status = 0;
        p_bank[bank]->b_data.v_status2 = 0;
        p_bank[bank]->b_data.is_grounded = 0;
    } else {
        status_graphical_update(bank, p_bank[bank]->b_data.status);
    }

    if (!flags->pass_disables) {

    }

    p_bank[bank]->b_data.illusion = 0;
    p_bank[bank]->b_data.fainted = 0;

    p_bank[bank]->b_data.stockpile_uses = 0;
    p_bank[bank]->b_data.stockpile_def_boost = 0;
    p_bank[bank]->b_data.stockpile_spdef_boost = 0;
}


void sync_battler_struct(u8 bank)
{
    u16 c_hp = p_bank[bank]->b_data.current_hp;
    u8 ailment = ailment_encode(bank);
    pokemon_setattr(p_bank[bank]->this_pkmn, REQUEST_CURRENT_HP, &c_hp);
    pokemon_setattr(p_bank[bank]->this_pkmn, REQUEST_STATUS_AILMENT, &ailment);
    for (u8 i = 0; i < 4; i++) {
        u8 pp = B_GET_MOVE_PP(bank, i);
        pokemon_setattr(p_bank[bank]->this_pkmn, REQUEST_PP1 + i, &pp);
    }
}


bool update_bank_hit_list(u8 bank_index)
{
    u16 move = CURRENT_MOVE(bank_index);
    u8 list[4] = {BANK_MAX, BANK_MAX, BANK_MAX, BANK_MAX};

    if (M_HITS_SELF(move)) {
        // Target is user
        list[0] = bank_index;
    } else if (M_HITS_ALLY_OR_SELF(move)) {
        // todo. The choice between ally and self.
        list[0] = bank_index;
    } else if (M_HITS_TARGET(move)) {
        // Target is selected Target
        list[0] = FOE_BANK(bank_index);
    } else if (M_HITS_ALLY(move)) {
        // hits ally
        if (SIDE_OF(bank_index) == PLAYER_SIDE) {
            list[0] = bank_index ? 0 : 1;
        } else {
            list[0] = (bank_index < 2) ? 2 : 1;
        }
        // in singles hitting your ally does nothing.
        return false;
    } else if (M_HITS_ADJ(move)) {
        for (u8 i = 0; i < 4; i++) {
            list[i] = (i != bank_index) ? i : BANK_MAX;
        }
    } else if (M_HITS_FOE_SIDE(move)) {
        // Target is the foe side
        if (SIDE_OF(bank_index) == PLAYER_SIDE) {
            list[0] = 2;
            list[1] = 3;
        } else {
            list[0] = 0;
            list[1] = 1;
        }
    } else if (M_HITS_MY_SIDE(move)) {
        // Target is the user side
        if (SIDE_OF(bank_index) == PLAYER_SIDE) {
            list[0] = 0;
            list[1] = 1;
        } else {
            list[0] = 2;
            list[1] = 3;
        }
    } else if (M_HITS_ALL(move)) {
        // Target is everything
        for (u8 i = 0; i < 4; i++) {
            list[i] = i;
        }
    } else {
        // no target
        return false;
    }
    for (u8 i = 0; i < 4; i++) {
        battle_master->bank_hit_list[i] = list[i];
    }

    return true;
}
