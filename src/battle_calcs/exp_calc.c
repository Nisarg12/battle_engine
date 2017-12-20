#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern void dprintf(const char * str, ...);

u32 calc_exp(u8 fainted, u8 reciever)
{
    u8 trainer_mon = (battle_type_flag != BATTLE_MODE_WILD) ? 150 : 100;
    // base species recieve incase of species index change
    u16 base_yield = pokemon_getattr(p_bank[fainted]->this_pkmn, REQUEST_SPECIES, NULL);
    base_yield = pokemon_base_stats[base_yield].xp_yield;
    u8 fainted_lvl = p_bank[fainted]->b_data.level;
    u8 reciever_lvl = p_bank[reciever]->b_data.level;
    
    u8 traded = pokemon_getattr(p_bank[reciever]->this_pkmn, REQUEST_TID, NULL);
    traded = (traded == saveblock2->trainerid) ? 100 : 150;
    
    u32 exp_part1 = fainted_lvl * base_yield;
    exp_part1 = NUM_MOD(exp_part1, trainer_mon);
    exp_part1 = exp_part1 / 5;
    
    u32 exp_part2 = ((2 *fainted_lvl) + 10);
    exp_part2 *= (exp_part2 * NUM_MOD(exp_part2, 50));
    u32 exp_part3 = (fainted_lvl + reciever_lvl + 10);
    exp_part3 *= (exp_part3 * NUM_MOD(exp_part3, 50));
    exp_part2 /= exp_part3;
    exp_part2 = MAX(exp_part2, 1);
    exp_part1 = (exp_part1 * exp_part2) + 1;
    return NUM_MOD(exp_part1, traded);   
}

/* TODO exp share item - probably as a callback here */
/* TODO Lucky egg item - probably as a callback here */
/* TODO Passpower/O-power - probably not implementing this */
void give_exp(u8 fainted, u8 reciever)
{
    u32 exp = calc_exp(fainted, reciever);
    enqueue_message(0, reciever, STRING_EXP_GAIN, exp);
    /* TODO add task that grants exp*/
    exp += pokemon_getattr(p_bank[reciever]->this_pkmn, REQUEST_EXP_POINTS, NULL);
    pokemon_setattr(p_bank[reciever]->this_pkmn, REQUEST_EXP_POINTS, &exp);
    u16 total_hp = pokemon_getattr(p_bank[reciever]->this_pkmn, REQUEST_TOTAL_HP, NULL);
    recalculate_stats(p_bank[reciever]->this_pkmn);
    u8 new_lvl = pokemon_getattr(p_bank[reciever]->this_pkmn, REQUEST_LEVEL, NULL);
    if (new_lvl > p_bank[reciever]->b_data.level) {
        enqueue_message(0, reciever, STRING_LEVEL_UP, 0);
        u16 total_hp_new = pokemon_getattr(p_bank[reciever]->this_pkmn, REQUEST_TOTAL_HP, NULL);
        B_CURRENT_HP(reciever) += (total_hp_new - total_hp);
    }
    return;
}

