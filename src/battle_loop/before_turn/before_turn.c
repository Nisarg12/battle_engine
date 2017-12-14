#include <pokeagb/pokeagb.h>
#include "../../battle_data/pkmn_bank.h"
#include "../../battle_data/pkmn_bank_stats.h"
#include "../../battle_data/battle_state.h"
#include "../../moves/moves.h"
#include "../../battle_text/battle_pick_message.h"
#include "../move_chain_states.h"

extern u16 pick_player_attack(void);
extern u16 pick_opponent_attack(void);
extern u16 rand_range(u16 min, u16 max);
extern void set_attack_battle_master(u8 bank, u8 index, s8 priority);
extern u8 set_target_bank(u8 user_bank, u16 move_id);

s8 get_move_priority(u8 bank)
{
    u16 move = CURRENT_MOVE(bank);

    /* update selected move's innate priority */
    s8 priority = 0;
    priority += MOVE_PRIORITY(move);

    /* on flee the actor has a priority high enough to outspeed everything except pursuit */
    if (p_bank[bank]->b_data.is_running)
        priority = 6;
    return priority;
}

void battle_set_order()
{
    u16 player_speed = B_SPEED_STAT(PLAYER_SINGLES_BANK);
    u16 opponent_speed = B_SPEED_STAT(OPPONENT_SINGLES_BANK);
    if (player_speed == opponent_speed) {
        // roll speed tie
        if (rand_range(0, 1))
            player_speed++;
        else
            opponent_speed++;
    }

    // higher speed goes first. Exec before move callbacks in order.
    u16 m1 = 0;
    u16 m2 = 0;
    bool speed_result = (battle_master->field_state.speed_inverse) ? (player_speed < opponent_speed) : (player_speed > opponent_speed);
    if (speed_result) {
        battle_master->first_bank = PLAYER_SINGLES_BANK;
        battle_master->second_bank = OPPONENT_SINGLES_BANK;
        m1 = CURRENT_MOVE(PLAYER_SINGLES_BANK);
        m2 = CURRENT_MOVE(OPPONENT_SINGLES_BANK);
    } else {
        battle_master->first_bank = OPPONENT_SINGLES_BANK;
        battle_master->second_bank = PLAYER_SINGLES_BANK;
        m1 = CURRENT_MOVE(OPPONENT_SINGLES_BANK);
        m2 = CURRENT_MOVE(PLAYER_SINGLES_BANK);
    }

    if (moves[m1].before_turn) {
        add_callback(CB_ON_BEFORE_TURN, 0, 0, battle_master->first_bank, (u32)moves[m1].before_turn);
    }
    if (moves[m2].before_turn) {
        add_callback(CB_ON_BEFORE_TURN, 0, 0, battle_master->second_bank, (u32)moves[m2].before_turn);
    }
    
    s8 player_priority = get_move_priority(PLAYER_SINGLES_BANK);
    s8 opp_priority = get_move_priority(OPPONENT_SINGLES_BANK);
    B_MOVE_PRIORITY(PLAYER_SINGLES_BANK) = player_priority;
    B_MOVE_PRIORITY(OPPONENT_SINGLES_BANK) = opp_priority;
    
    // run base power callbacks
    build_execution_order(CB_ON_BEFORE_TURN);
    battle_master->executing = true;
    while (battle_master->executing) {
        u8 attacker = CB_MASTER[CB_EXEC_ORDER[CB_EXEC_INDEX]].source_bank;
        u16 move = CURRENT_MOVE(CB_MASTER[CB_EXEC_ORDER[CB_EXEC_INDEX]].source_bank);
        pop_callback(attacker, move);
    }
    // before turn could've changed the priorities
    player_priority = B_MOVE_PRIORITY(PLAYER_SINGLES_BANK);
    opp_priority = B_MOVE_PRIORITY(OPPONENT_SINGLES_BANK);

    /* Turn order, higher priority will go first */
    if (player_priority > opp_priority) {
        battle_master->first_bank = PLAYER_SINGLES_BANK;
        battle_master->second_bank = OPPONENT_SINGLES_BANK;
    } else if (player_priority < opp_priority) {
        battle_master->first_bank = OPPONENT_SINGLES_BANK;
        battle_master->second_bank = PLAYER_SINGLES_BANK;
    }

    set_attack_battle_master(battle_master->first_bank, 0, battle_master->first_bank == PLAYER_SINGLES_BANK ? player_priority : opp_priority);
    set_attack_battle_master(battle_master->second_bank, 1, battle_master->second_bank == OPPONENT_SINGLES_BANK ? opp_priority : player_priority);
    battle_master->b_moves[battle_master->first_bank].flinch = 0;
    battle_master->b_moves[battle_master->second_bank].flinch = 0;
    super.multi_purpose_state_tracker = 0;
    battle_master->execution_index = 0;
}
