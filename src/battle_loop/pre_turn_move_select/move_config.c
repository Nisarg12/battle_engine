#include <pokeagb/pokeagb.h>
#include "../../battle_data/pkmn_bank.h"
#include "../../battle_data/pkmn_bank_stats.h"
#include "../../battle_data/battle_state.h"
#include "../../moves/moves.h"
#include "../../battle_text/battle_pick_message.h"
#include "../move_chain_states.h"

extern u16 rand_range(u16 min, u16 max);
extern void dprintf(const char * str, ...);
extern u8 count_usable_moves(u8 bank);
extern u8 count_total_moves(u8 bank);
extern bool move_is_usable(u8 bank, u16 move);
extern u8 move_pp_count(u16 move_id, u8 bank);
extern u8 get_move_index(u16 move_id, u8 bank);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);

u16 pick_player_attack()
{
    if (p_bank[PLAYER_SINGLES_BANK]->b_data.is_running)
        return 0;
    if (p_bank[PLAYER_SINGLES_BANK]->b_data.skip_move_select)
        return LAST_MOVE(PLAYER_SINGLES_BANK);
    u16 player_moveid = battle_master->battle_cursor.cursor_pos;
    if (player_moveid == 1) {
        player_moveid += 1;
    } else if (player_moveid == 2) {
        player_moveid -= 1;
    }
    p_bank[PLAYER_SINGLES_BANK]->b_data.pp_index = player_moveid;
    return (B_GET_MOVE(PLAYER_SINGLES_BANK, player_moveid));
}

bool external_move_disable_callbacks(u8 bank, u16 move)
{
    // callbacks for external move disables
    build_execution_order(CB_ON_DISABLE_MOVE);
    battle_master->executing = true;
    while (battle_master->executing) {
        // if a move is successfully disabled, the child should return false to halt
        if (!pop_callback(bank, move)) {
            // this is to stoping moves in multi turn is left to the child callback.
            // some moves like disable stop them, but moves like torment do not
            battle_master->executing = false;
            return false;
        }
    }
    return true;
}

// Return True = Continue; False = Reset
bool player_move_selection_singles()
{
    // get and set currently selected move
    u16 move_player = pick_player_attack();
    CURRENT_MOVE(PLAYER_SINGLES_BANK) = move_player;

    // if player is fleeing, don't use a move
    if (p_bank[PLAYER_SINGLES_BANK]->b_data.is_running) {
        CURRENT_MOVE(PLAYER_SINGLES_BANK) = MOVE_NONE;
        p_bank[PLAYER_SINGLES_BANK]->b_data.pp_index = 0xFF;
        return true;
    }

    // if player doesn't have an available move, struggle instead
    u8 index = 0;
    // suppress messages
    u8 front_index = battle_master->queue_front_index;
    u8 q_size = battle_master->queue_size;
    for (u8 i = 0; i < 4; i++) {
        // if move is usable insert to array
        u16 selected_move = B_GET_MOVE(PLAYER_SINGLES_BANK, i);
        if (move_is_usable(PLAYER_SINGLES_BANK, selected_move)) {
            if (external_move_disable_callbacks(PLAYER_SINGLES_BANK, selected_move)) {
                index++;
            }
        }
    }
    // restore suppression
    battle_master->queue_front_index = front_index;
    battle_master->queue_size = q_size;
    if (index < 1) {
        // all moves disabled or unpickable
        CURRENT_MOVE(PLAYER_SINGLES_BANK) = MOVE_STRUGGLE;
        B_MOVE_CAN_CRIT(PLAYER_SINGLES_BANK) = false;
        p_bank[PLAYER_SINGLES_BANK]->b_data.pp_index = 0xFF;
        return true;
    }

    // ensure player has PP for move being used
    if (move_pp_count(move_player, PLAYER_SINGLES_BANK) < 1) {
        enqueue_message(0, 0, STRING_NO_PP, 0);
        return false;
    }
    B_MOVE_CAN_CRIT(PLAYER_SINGLES_BANK) = true;
    return external_move_disable_callbacks(PLAYER_SINGLES_BANK, move_player);
}

void pick_wild_singles_ai_attack()
{
    // If bank is attempting to flee, don't queue a move
    if (p_bank[OPPONENT_SINGLES_BANK]->b_data.is_running) {
        CURRENT_MOVE(OPPONENT_SINGLES_BANK) = MOVE_NONE;
        p_bank[OPPONENT_SINGLES_BANK]->b_data.pp_index = 0xFF;
        return;
    }

    // if AI is locked into current move, use that move instead
    if (p_bank[OPPONENT_SINGLES_BANK]->b_data.skip_move_select) {
        CURRENT_MOVE(OPPONENT_SINGLES_BANK) = LAST_MOVE(OPPONENT_SINGLES_BANK);
        return;
    }

    // if AI is charging or recharging, don't use a move
    if (HAS_VOLATILE(OPPONENT_SINGLES_BANK, VOLATILE_CHARGING) || HAS_VOLATILE(OPPONENT_SINGLES_BANK, VOLATILE_RECHARGING)) {
        // use move was using last time.
        return;
    }

    // pick a valid move for the AI
    u16 to_pick[4] = {0};
    u8 index = 0;
    for (u8 i = 0; i < 4; i++) {
        // if move is usable insert to array
        u16 selected_move = B_GET_MOVE(OPPONENT_SINGLES_BANK, i);
        if (move_is_usable(OPPONENT_SINGLES_BANK, selected_move)) {
            if (external_move_disable_callbacks(OPPONENT_SINGLES_BANK, selected_move)) {
                to_pick[index] = selected_move;
                index++;
            }
        }
    }
    if (index < 1) {
        // all moves disabled or unpickable
        CURRENT_MOVE(OPPONENT_SINGLES_BANK) = MOVE_STRUGGLE;
        B_MOVE_CAN_CRIT(OPPONENT_SINGLES_BANK) = false;
        p_bank[OPPONENT_SINGLES_BANK]->b_data.pp_index = 0xFF;
        return;
    }

    u16 rand_index = rand_range(0, index);
    CURRENT_MOVE(OPPONENT_SINGLES_BANK) = to_pick[rand_index];
    p_bank[OPPONENT_SINGLES_BANK]->b_data.pp_index = get_move_index(to_pick[rand_index], OPPONENT_SINGLES_BANK);
    B_MOVE_CAN_CRIT(OPPONENT_SINGLES_BANK) = true;
    return true;
}


void set_attack_battle_master(u8 bank, u8 index, s8 priority)
{
    u16 move_id = p_bank[bank]->b_data.current_move;
    p_bank[bank]->b_data.last_damage = 0;
    B_LAST_MOVE_FAILED(bank) = B_MOVE_FAILED(bank);
    B_MOVE_FAILED(bank) = false;

    battle_master->b_moves[index].user_bank = bank;
    battle_master->b_moves[index].move_id = move_id;
    battle_master->b_moves[index].priority = priority;
    battle_master->b_moves[index].stab = 150; // move stab bonus
    battle_master->b_moves[index].power = MOVE_POWER(move_id);
    battle_master->b_moves[index].category = MOVE_CATEGORY(move_id);
    battle_master->b_moves[index].type[0] = MOVE_TYPE(move_id);
    battle_master->b_moves[index].type[1] = MTYPE_EGG;
    battle_master->b_moves[index].accuracy = MOVE_ACCURACY(move_id);
    battle_master->b_moves[index].remove_contact = false;
    battle_master->b_moves[index].copied = false;
    battle_master->b_moves[index].ignore_abilities = false;
    battle_master->b_moves[index].prankstered = HAS_VOLATILE(bank, VOLATILE_PRANKSTERED);
    battle_master->b_moves[index].heal = moves[move_id].heal;
    CLEAR_VOLATILE(bank, VOLATILE_PRANKSTERED);
    battle_master->b_moves[index].infiltrates = false;
    if (moves[move_id].multi_hit[0]) {
        u8 hit_times = 0;
        if ((moves[move_id].multi_hit[0] == 2) && (moves[move_id].multi_hit[1] == 5)) {
            switch(rand_range(0, 3)) {
                case 0:
                    hit_times = 2;
                    break;
                case 1:
                    hit_times = 3;
                    break;
                case 2:
                    hit_times = rand_range(4, 6);
                    break;
            };
        } else {
            if (moves[move_id].multi_hit[0] == moves[move_id].multi_hit[1]) {
                hit_times = moves[move_id].multi_hit[1];
            } else {
                hit_times = rand_range(moves[move_id].multi_hit[0], moves[move_id].multi_hit[1]);
                hit_times |= 1;
            }
        }
        battle_master->b_moves[index].hit_times = hit_times - 1;
        battle_master->b_moves[index].hit_counter = 1;
    }
    //B_MOVE_CAN_CRIT(bank) = true;
    battle_master->b_moves[index].b_procs = *(moves[move_id].procs);
}

void reset_turn_bits(u8 bank)
{
    p_bank[bank]->b_data.is_running = 0;
    p_bank[bank]->b_data.is_switching = 0;
    p_bank[bank]->b_data.first_turn = 0;
    memset((void*)(&battle_master->b_moves[B_MOVE_BANK(bank)]), 0x0, sizeof(struct move_used));

}

void set_attack_bm_inplace(u16 move_id, u8 new_bank, u8 index)
{
    battle_master->b_moves[index].user_bank = new_bank;
    battle_master->b_moves[index].move_id = move_id;
    battle_master->b_moves[index].priority = B_MOVE_PRIORITY(new_bank);
    battle_master->b_moves[index].stab = 150; // move stab bonus
    battle_master->b_moves[index].power = MOVE_POWER(move_id);
    battle_master->b_moves[index].category = MOVE_CATEGORY(move_id);
    battle_master->b_moves[index].type[0] = MOVE_TYPE(move_id);
    battle_master->b_moves[index].type[1] = MTYPE_EGG;
    battle_master->b_moves[index].accuracy = MOVE_ACCURACY(move_id);
    battle_master->b_moves[index].remove_contact = false;
    battle_master->b_moves[index].copied = false;
    battle_master->b_moves[index].ignore_abilities = false;
    battle_master->b_moves[index].prankstered = false;
    battle_master->b_moves[index].heal = moves[move_id].heal;
    battle_master->b_moves[index].infiltrates = false;
    if (moves[move_id].multi_hit[0]) {
        u8 hit_times = 0;
        if ((moves[move_id].multi_hit[0] == 2) && (moves[move_id].multi_hit[1] == 5)) {
            switch(rand_range(0, 3)) {
                case 0:
                    hit_times = 2;
                    break;
                case 1:
                    hit_times = 3;
                    break;
                case 2:
                    hit_times = rand_range(4, 6);
                    break;
            };
        } else {
            if (moves[move_id].multi_hit[0] == moves[move_id].multi_hit[1]) {
                hit_times = moves[move_id].multi_hit[1];
            } else {
                hit_times = rand_range(moves[move_id].multi_hit[0], moves[move_id].multi_hit[1]);
                hit_times |= 1;
            }
        }
        battle_master->b_moves[index].hit_times = hit_times - 1;
        battle_master->b_moves[index].hit_counter = 1;
    }
    battle_master->b_moves[index].b_procs = *(moves[move_id].procs);
}
