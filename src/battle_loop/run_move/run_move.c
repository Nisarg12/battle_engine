#include <pokeagb/pokeagb.h>
#include "../../battle_data/pkmn_bank.h"
#include "../../battle_data/pkmn_bank_stats.h"
#include "../../battle_data/battle_state.h"
#include "../../moves/moves.h"
#include "../../battle_text/battle_pick_message.h"
#include "../move_chain_states.h"
#include "../../status_effects/status.h"

extern void run_decision(void);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern bool peek_message(void);
extern void move_hit(void);
extern void on_faint(void);
extern bool target_exists(u8 bank);
extern void run_move_failed_cbs(u8 attacker, u8 defender, u16 move);
extern void do_residual_status_effects(u8 order);
extern void dprintf(const char * str, ...);
extern void c1_residual_status_effects(void);
extern void hpbar_apply_dmg(u8 task_id);
extern bool is_grounded(u8 bank);
extern void set_attack_bm_inplace(u16 move_id, u8 new_bank, u8 index);
extern bool update_bank_hit_list(u8 bank_index);
extern u8 get_move_index(u16 move_id, u8 bank);


enum BeforeMoveStatus {
    CANT_USE_MOVE = 0,
    USE_MOVE_NORMAL,
    TARGET_MOVE_IMMUNITY,
    SILENT_FAIL,
    SILENT_CONTINUE,
};


enum BeforeMoveStatus before_move_cb(u8 attacker)
{
    u16 move = CURRENT_MOVE(attacker);
    // add callbacks specific to field
    if (moves[move].before_move) {
        add_callback(CB_ON_BEFORE_MOVE, 0, 0, attacker, (u32)moves[move].before_move);
    }
    // run callbacks
    build_execution_order(CB_ON_BEFORE_MOVE);
    battle_master->executing = true;
    while (battle_master->executing) {
        enum BeforeMoveStatus status = pop_callback(attacker, move);
        if (status != USE_MOVE_NORMAL)
            return status;
    }
    return USE_MOVE_NORMAL;
}

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


void run_move()
{
    while (peek_message())
        return;
    if (task_is_running(hpbar_apply_dmg))
        return;
    u8 bank_index = (battle_master->execution_index) ? battle_master->second_bank : battle_master->first_bank;
    switch(super.multi_purpose_state_tracker) {
        case S_BEFORE_MOVE:
        {
            if (HAS_VOLATILE(bank_index, VOLATILE_RECHARGING)) {
                super.multi_purpose_state_tracker = S_RESIDUAL_MOVES;
                CLEAR_VOLATILE(bank_index, VOLATILE_RECHARGING);
                enqueue_message(0, bank_index, STRING_MUST_RECHARGE, 0);
                return;
            }
			/* Move callbacks  */
            u8 result = before_move_cb(bank_index);
            switch (result) {
                case CANT_USE_MOVE:
                case TARGET_MOVE_IMMUNITY:
                    enqueue_message(0, bank_index, STRING_FAILED, 0);
                case SILENT_FAIL:
                    battle_master->move_completed = true;
                    super.multi_purpose_state_tracker = S_MOVE_FAILED;
                    return;
                case SILENT_CONTINUE:
                    break;
            };

			/* Before Move effects which cause turn ending */
			super.multi_purpose_state_tracker = S_RESIDUAL_MOVES;
            B_MOVE_FAILED(bank_index) = true;
			if (HAS_VOLATILE(bank_index, VOLATILE_SLEEP_TURN)) {
				enqueue_message(0, bank_index, STRING_FAST_ASLEEP, 0);
                break;
			} else if (HAS_VOLATILE(bank_index, VOLATILE_CONFUSE_TURN)) {
                break;
			} else if (HAS_VOLATILE(bank_index, VOLATILE_ATK_SKIP_TURN)) {
                CLEAR_VOLATILE(bank_index, VOLATILE_ATK_SKIP_TURN);
                break;
            } else if (HAS_VOLATILE(bank_index, VOLATILE_CHARGING)) {
                break;
            } else {
				// display "Pokemon used move!"
                B_MOVE_FAILED(bank_index) = false;
                if (result != SILENT_CONTINUE)
		            enqueue_message(CURRENT_MOVE(bank_index), bank_index, STRING_ATTACK_USED, 0);
				super.multi_purpose_state_tracker = S_CONFIG_MOVE_EXEC;
			}
            break;
        }
        case S_CONFIG_MOVE_EXEC:
        {
            /* Initialize target banks for moves */
            if (!update_bank_hit_list(bank_index)) {
                // Move didn't specify a target
                enqueue_message(0, bank_index, STRING_FAILED, 0);
                super.multi_purpose_state_tracker = S_MOVE_FAILED;
                return;
            }
            // reduce PP
            if ((!HAS_VOLATILE(bank_index, VOLATILE_MULTI_TURN)) && (!B_REDUCE_PP(bank_index))) {
                u8 pp_index = p_bank[bank_index]->b_data.pp_index;
                if (pp_index < 4) {
                    p_bank[bank_index]->b_data.move_pp[pp_index]--;
                }
            }
            battle_master->move_completed = false;
            super.multi_purpose_state_tracker = S_RUN_MOVE_HIT;
            break;
        }
        case S_RUN_MOVE_HIT:
        {
            bool will_move = false;
            for (u8 i = 0; i < sizeof(battle_master->bank_hit_list); i++) {
                if (battle_master->bank_hit_list[i] < BANK_MAX) {
                    if (ACTIVE_BANK(battle_master->bank_hit_list[i])) {
                        TARGET_OF(bank_index) = battle_master->bank_hit_list[i];
                        battle_master->bank_hit_list[i] = BANK_MAX;
                        will_move = true;
                        break;
                    } else {
                        battle_master->bank_hit_list[i] = BANK_MAX;
                    }
                }
            }
            if (will_move) {
                // reset battle master move structure for this move
                set_attack_bm_inplace(CURRENT_MOVE(bank_index), bank_index, B_MOVE_BANK(bank_index));
                B_MOVE_DMG(bank_index) = 0;
                B_MOVE_EFFECTIVENESS(bank_index) = 0;
                if (!on_modify_move(bank_index, TARGET_OF(bank_index), CURRENT_MOVE(bank_index))) {
                    B_MOVE_FAILED(bank_index) = true;
                    enqueue_message(0, bank_index, STRING_FAILED, 0);
                    battle_master->move_completed = true;
                    super.multi_purpose_state_tracker = S_AFTER_MOVE;
                    set_callback1(move_hit);
                    return;
                }
                set_callback1(move_hit); // move hit will advance the state when complete
                super.multi_purpose_state_tracker = S_MOVE_TRYHIT;
            } else {
                battle_master->move_completed = true;
                super.multi_purpose_state_tracker = S_AFTER_MOVE;
                set_callback1(move_hit);
            }
            break;
        }
        case S_MOVE_FAILED:
        {
            if (B_MOVE_FAILED(bank_index)) {
                run_move_failed_cbs(bank_index, TARGET_OF(bank_index), CURRENT_MOVE(bank_index));
            }
            if (battle_master->move_completed) {
                battle_master->field_state.last_used_move = CURRENT_MOVE(bank_index);
                super.multi_purpose_state_tracker = S_RUN_FAINT;
                battle_master->c1_after_faint_check = run_move;
                battle_master->c1_prestate = S_RESIDUAL_MOVES;
            } else {
                super.multi_purpose_state_tracker = S_RUN_MOVE_HIT;
            }
            break;
        }
        case S_RUN_FAINT:
            // Run on faint stuff
            set_callback1(on_faint);
            super.multi_purpose_state_tracker = S_CHECK_BANK1_FAINT;
            break;
        case S_RESIDUAL_MOVES:
        {
            if (battle_master->repeat_move) {
                super.multi_purpose_state_tracker = S_BEFORE_MOVE;
                battle_master->repeat_move = false;
                set_callback1(run_move);
                return;
            }
            if (bank_index != battle_master->first_bank) {
                for (u8 i = 0; i < 2; i++) {
                    battle_master->b_moves[i].flinch = 0;
                }
                extern void c1_residual_callbacks(void);
                set_callback1(c1_residual_callbacks);
                super.multi_purpose_state_tracker = 0;
                return;
            }
            super.multi_purpose_state_tracker = S_WAIT_HPUPDATE_RUN_MOVE;
            break;
        }
        case S_WAIT_HPUPDATE_RUN_MOVE:
        {
            /* Passive residual effects from engine structs */
            if (is_grounded(bank_index)) {
                B_IS_GROUNDED(bank_index) = true;
            }
            LAST_MOVE(bank_index) = CURRENT_MOVE(bank_index);
            // update moves used history
            p_bank[bank_index]->b_data.will_move = false;
            u16 last_move = LAST_MOVE(bank_index);
            u8 slot = get_move_index(last_move, bank_index);
            p_bank[bank_index]->b_data.moves_used[slot] = last_move;
            super.multi_purpose_state_tracker = S_RUN_MOVE_ALTERNATE_BANK;
            set_callback1(run_decision);
            break;
        }
    };
}


void c1_residual_callbacks()
{
    while (peek_message())
		return;
	if (task_is_running(hpbar_apply_dmg))
		return;
	switch (super.multi_purpose_state_tracker) {
        case 0:
        {
            u16 player_speed = B_SPEED_STAT(PLAYER_SINGLES_BANK);
            u16 opponent_speed = B_SPEED_STAT(OPPONENT_SINGLES_BANK);
            battle_master->bank_state = (player_speed < opponent_speed) ? PLAYER_SINGLES_BANK : OPPONENT_SINGLES_BANK;
            build_execution_order(CB_ON_RESIDUAL);
            battle_master->executing = true;
            super.multi_purpose_state_tracker++;
            break;
        }
        case 1:
        {
            super.multi_purpose_state_tracker = S_RUN_FAINT;
            battle_master->c1_after_faint_check = c1_residual_callbacks;
            battle_master->c1_prestate = 2;
            set_callback1(run_move);
            break;
        }
        case 2:
        {
            if (battle_master->executing) {
                battle_master->bank_state  = FOE_BANK(battle_master->bank_state);
                run_callback(battle_master->bank_state , CURRENT_MOVE(battle_master->bank_state));
                super.multi_purpose_state_tracker = 3;
            } else {
                super.multi_purpose_state_tracker = 4;
            }
            break;
        }
        case 3:
        {
            if (battle_master->executing) {
                battle_master->bank_state  = FOE_BANK(battle_master->bank_state);
                pop_callback(battle_master->bank_state , CURRENT_MOVE(battle_master->bank_state ));
                super.multi_purpose_state_tracker = 1;
            } else {
                super.multi_purpose_state_tracker = 4;
            }
            break;
        }
        default:
            super.multi_purpose_state_tracker = S_RUN_FAINT;
            battle_master->c1_after_faint_check = run_move;
            battle_master->c1_prestate = S_WAIT_HPUPDATE_RUN_MOVE;
            set_callback1(run_move);
            break;
        };
}
