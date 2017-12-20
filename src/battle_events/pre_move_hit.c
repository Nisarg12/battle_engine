#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"
#include "battle_events/battle_events.h"

extern void set_attack_bm_inplace(u16 move_id, u8 bank);
extern bool update_bank_hit_list(u8 bank_index);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);

bool on_modify_move(u8 attacker, u8 defender, u16 move)
{
    // add callbacks specific to field
    if (moves[move].on_modify_move) {
        add_callback(CB_ON_MODIFY_MOVE, 0, 0, attacker, (u32)moves[move].on_modify_move);
    }
    // run callbacks
    build_execution_order(CB_ON_MODIFY_MOVE);
    battle_master->executing = true;
    while (battle_master->executing) {
        if (!(pop_callback(attacker, move)))
            return false;
    }
    return true;
}


void event_config_move_hitlist(struct action* current_action)
{
    if (!update_bank_hit_list(ACTION_BANK)) {
        // Move didn't specify a target
        enqueue_message(0, ACTION_BANK, STRING_FAILED, 0);
        CURRENT_ACTION->event_state = EventMoveFailed;
        return;
    }
    // reduce PP
    if ((!HAS_VOLATILE(ACTION_BANK, VOLATILE_MULTI_TURN)) && (!B_REDUCE_PP(ACTION_BANK))) {
        u8 pp_index = p_bank[ACTION_BANK]->b_data.pp_index;
        if (pp_index < 4) {
            p_bank[ACTION_BANK]->b_data.move_pp[pp_index]--;
        }
    }
    current_action->event_state++;
}


void event_pre_move_hit(struct action* current_action)
{
    bool will_move = false;
    u8 bank_index = current_action->action_bank;
    // check if hit list has a bank to hit. Mark said bank as hit and continue
    for (u8 i = 0; i < sizeof(battle_master->bank_hit_list); i++) {
        if (battle_master->bank_hit_list[i] < BANK_MAX) {
            if (ACTIVE_BANK(battle_master->bank_hit_list[i])) {
                TARGET_OF(bank_index) = battle_master->bank_hit_list[i];
                CURRENT_ACTION->target = battle_master->bank_hit_list[i];
                battle_master->bank_hit_list[i] = BANK_MAX;
                will_move = true;
                break;
            } else {
                if (battle_type_flag == BATTLE_MODE_WILD) {
                    if (B_FAINTED(battle_master->bank_hit_list[i])) {
                        enqueue_message(0, bank_index, STRING_NO_TARGET, 0);
                        CURRENT_ACTION->event_state = EventMoveFailed;
                    }
                }
                battle_master->bank_hit_list[i] = BANK_MAX;
            }
        }
    }

    // if there was a bank to hit, prepare data structures
    if (will_move) {
        // reset battle master move structure for this move
        CURRENT_MOVE(ACTION_BANK) = CURRENT_ACTION->move;
        if (current_action->reset_move_config) {
            reset_turn_bits(bank_index);
            current_action->reset_move_config = false;
        }
        set_attack_bm_inplace(CURRENT_MOVE(bank_index), bank_index);
        if (!on_modify_move(bank_index, TARGET_OF(bank_index), CURRENT_MOVE(bank_index))) {
            B_MOVE_FAILED(bank_index) = true;
            enqueue_message(0, bank_index, STRING_FAILED, 0);
            CURRENT_ACTION->event_state = EventMoveFailed;
            return;
        }
        CURRENT_ACTION->event_state++;
    } else {
        CURRENT_ACTION->event_state = EventMoveAfterMove;
    }
}
