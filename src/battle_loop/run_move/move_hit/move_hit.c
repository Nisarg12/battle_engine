#include <pokeagb/pokeagb.h>
#include "../../../battle_data/pkmn_bank.h"
#include "../../../battle_data/pkmn_bank_stats.h"
#include "../../../battle_data/battle_state.h"
#include "../../../moves/moves.h"
#include "../../../battle_text/battle_pick_message.h"
#include "../../move_chain_states.h"


extern void run_decision(void);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern bool peek_message(void);
extern void run_move(void);
extern bool try_hit(u8 bank);
extern bool target_exists(u8 bank);
extern u16 get_damage(u8 attacker, u8 defender, u16 move);
extern void hp_anim_change(u8 bank, s16 delta);
extern void hpbar_apply_dmg(u8 task_id);
extern void dprintf(const char * str, ...);
extern void set_status(u8 bank, u8 source, enum Effect status);
extern u16 rand_range(u16, u16);
extern bool is_fainted(void);
extern void move_procs_perform(u8 bank_index, u16 move);
extern void status_procs_perform(u8 bank_index);

bool damage_result_msg(u8 bank_index)
{

    // effectiveness msgs
    bool effective = true;
    switch (B_MOVE_EFFECTIVENESS(bank_index)) {
        case TE_IMMUNE:
            if (!B_MOVE_MULTI(bank_index))
                enqueue_message(0, bank_index, STRING_MOVE_IMMUNE, 0);
            effective = false;
            break;
        case TE_NOT_VERY_EFFECTIVE:
            if (!B_MOVE_MULTI(bank_index))
                enqueue_message(0, 0, STRING_MOVE_NVE, 0);
            break;
        case TE_SUPER_EFFECTIVE:
            if (!B_MOVE_MULTI(bank_index))
                enqueue_message(0, 0, STRING_MOVE_SE, 0);
            break;
        case TE_OHKO:
            if (!B_MOVE_MULTI(bank_index))
                enqueue_message(0, 0, STRING_OHKO, 0);
            break;
        default:
            break;
    };

    if (effective) {
        // crit msg if crit
        if (B_MOVE_WILL_CRIT(bank_index)) {
            enqueue_message(0, bank_index, STRING_MOVE_CRIT, 0);
            B_MOVE_WILL_CRIT_SET(bank_index, 0);
        }
    }
    return effective;
}

enum TryHitMoveStatus {
    CANT_USE_MOVE = 0,
    USE_MOVE_NORMAL,
    TARGET_MOVE_IMMUNITY,
    FAIL_SILENTLY,
};

enum TryHitMoveStatus move_tryhit(u8 attacker, u8 defender, u16 move)
{
    // add callbacks specific to field
    if (moves[move].on_tryhit_move) {
        add_callback(CB_ON_TRYHIT_MOVE, 0, 0, attacker, (u32)moves[move].on_tryhit_move);
    }
    // run callbacks
    build_execution_order(CB_ON_TRYHIT_MOVE);
    battle_master->executing = true;
    while (battle_master->executing) {
        enum TryHitMoveStatus status = pop_callback(attacker, move);
        if (status != USE_MOVE_NORMAL)
            return status;
    }
    return USE_MOVE_NORMAL;
}



void move_hit()
{
    if (task_is_running(hpbar_apply_dmg))
        return;
    while (peek_message())
        return;

    u8 bank_index = (battle_master->execution_index) ? battle_master->second_bank : battle_master->first_bank;
    u16 move = CURRENT_MOVE(bank_index);
    switch (super.multi_purpose_state_tracker) {
        case S_MOVE_TRYHIT:
            // flinch means no moving
            if (rand_range(0, 100) <= battle_master->b_moves[B_MOVE_BANK(bank_index)].flinch) {
                battle_master->b_moves[B_MOVE_BANK(bank_index)].flinch = 0;
                enqueue_message(0, bank_index, STRING_FLINCHED, 0);
                battle_master->c1_after_faint_check = run_move;
                battle_master->c1_prestate = S_RESIDUAL_MOVES;
                super.multi_purpose_state_tracker = S_RUN_FAINT;
                set_callback1(run_decision);
                return;
            }
            // move tryhit callback
            switch (move_tryhit(bank_index, TARGET_OF(bank_index), move)) {
                case CANT_USE_MOVE:
                    B_MOVE_FAILED(bank_index) = 1;
                    enqueue_message(move, bank_index, STRING_FAILED, move);
                    super.multi_purpose_state_tracker = S_MOVE_FAILED;
                    set_callback1(run_move);
                    return;
                case TARGET_MOVE_IMMUNITY:
                    B_MOVE_FAILED(bank_index) = 1;
                    enqueue_message(0, bank_index, STRING_MOVE_IMMUNE, 0);
                    super.multi_purpose_state_tracker = S_MOVE_FAILED;
                    set_callback1(run_move);
                    return;
                case FAIL_SILENTLY:
                    B_MOVE_FAILED(bank_index) = 1;
                    super.multi_purpose_state_tracker = S_MOVE_FAILED;
                    set_callback1(run_move);
                    return;
                default:
                    break;
            };
            // move landing is success
            super.multi_purpose_state_tracker = S_GENERAL_TRYHIT;
            break;
        case S_GENERAL_TRYHIT:
            if (!try_hit(bank_index)) {
                // move has missed
                B_MOVE_FAILED(bank_index) = 1;
                super.multi_purpose_state_tracker = S_MOVE_FAILED;
                set_callback1(run_move);
                return;
            }
            super.multi_purpose_state_tracker = S_DAMAGE_CALC_AND_APPLY;
            break;
        case S_DAMAGE_CALC_AND_APPLY:
            {
                // get dmg
                u16 dmg = get_damage(bank_index, TARGET_OF(bank_index), CURRENT_MOVE(bank_index));
                if (dmg < 1) {
                    super.multi_purpose_state_tracker = S_HEAL_CALC_AND_APPLY;
                    return;
                }
                if (!damage_result_msg(bank_index)) {
                    set_callback1(run_move);
                    super.multi_purpose_state_tracker = S_MOVE_FAILED;
                    return;
                }
                battle_master->b_moves[B_MOVE_BANK(bank_index)].dmg = dmg;
                p_bank[TARGET_OF(bank_index)]->b_data.last_damage = dmg;
                p_bank[TARGET_OF(bank_index)]->b_data.last_attacked_by = bank_index;

                // HP bar damage animation
                s16 delta = B_CURRENT_HP(TARGET_OF(bank_index)) - dmg;
                delta = MAX(delta, 0);
                hp_anim_change(TARGET_OF(bank_index), delta);
                super.multi_purpose_state_tracker = S_HEAL_CALC_AND_APPLY;
            }
            break;
        case S_HEAL_CALC_AND_APPLY:
            /* TODO calc healing */
            if (B_HEAL(bank_index)) {
                u16 heal_target =  TARGET_OF(bank_index);
                u16 heal = B_HEAL(bank_index);
                heal = NUM_MOD(TOTAL_HP(heal_target), heal);
                if (TOTAL_HP(heal_target) < (heal + B_CURRENT_HP(heal_target))) {
                    heal = TOTAL_HP(heal_target) - B_CURRENT_HP(heal_target);
                }
                if (heal > 0) {
                    hp_anim_change(heal_target, heal + B_CURRENT_HP(heal_target));
                    enqueue_message(CURRENT_MOVE(bank_index), heal_target, STRING_HEAL, 0);
                }
            }
            super.multi_purpose_state_tracker = S_MOVE_EFFECT;
            break;
        case S_MOVE_EFFECT:
        {
            /* execute move effect */
            u8 attacker = bank_index;
            u16 move = CURRENT_MOVE(attacker);

            // add callbacks specific to field
            if (moves[move].on_effect_cb) {
                add_callback(CB_ON_EFFECT, 0, 0, attacker, (u32)moves[move].on_effect_cb);
            }
            // run callbacks
            build_execution_order(CB_ON_EFFECT);
            battle_master->executing = true;
            while (battle_master->executing) {
                if (!(pop_callback(attacker, move))) {
                    enqueue_message(move, attacker, STRING_FAILED, 0);
					super.multi_purpose_state_tracker = S_MOVE_FAILED;
					set_callback1(run_move);
					return;
                }
            }
			super.multi_purpose_state_tracker = S_RECOIL_APPLY;
			break;
        }
        case S_RECOIL_APPLY:
            // check for recoil
            if (moves[CURRENT_MOVE(bank_index)].recoil_struggle) {
                // struggle recoil is based off max health
                u16 recoil = NUM_MOD(TOTAL_HP(bank_index), moves[CURRENT_MOVE(bank_index)].recoil);
                s16 delta = B_CURRENT_HP(bank_index) - recoil;
                delta = MAX(delta, 0);
                hp_anim_change(bank_index, delta);
                enqueue_message(CURRENT_MOVE(bank_index), bank_index, STRING_RECOIL, 0);
            } else if (battle_master->b_moves[B_MOVE_BANK(bank_index)].dmg != 0 && moves[CURRENT_MOVE(bank_index)].recoil > 0) {
                u16 recoil = NUM_MOD(battle_master->b_moves[B_MOVE_BANK(bank_index)].dmg, moves[CURRENT_MOVE(bank_index)].recoil);
                s16 delta = B_CURRENT_HP(bank_index) - recoil;
                delta = MAX(delta, 0);
                hp_anim_change(bank_index, delta);
                enqueue_message(CURRENT_MOVE(bank_index), bank_index, STRING_RECOIL, 0);
            } 
            super.multi_purpose_state_tracker = S_DRAIN_APPLY;
            break;
        case S_DRAIN_APPLY:
        {
            if (battle_master->b_moves[B_MOVE_BANK(bank_index)].dmg != 0 && moves[CURRENT_MOVE(bank_index)].drain > 0 ){
                u16 drain = NUM_MOD(battle_master->b_moves[B_MOVE_BANK(bank_index)].dmg, moves[CURRENT_MOVE(bank_index)].drain);
                s16 delta = B_CURRENT_HP(bank_index) + drain;
                delta = MIN(delta, TOTAL_HP(bank_index));
                hp_anim_change(bank_index, delta);
                enqueue_message(CURRENT_MOVE(bank_index), bank_index, STRING_DRAIN, 0);
            }
            super.multi_purpose_state_tracker = S_SECONDARY_ROLL_CHANCE;
            break;
        }
        case S_SECONDARY_ROLL_CHANCE:
        // Roll secondary boosts self
        {
			move_procs_perform(bank_index, move);
            break;
        }
		case S_SECONDARY_ROLL_AILMENTS:
        // roll status ailments
		{
			status_procs_perform(bank_index);
			break;
		}
        case S_AFTER_MOVE_SECONDARY:
            // set flinch chance of target
            battle_master->b_moves[B_MOVE_BANK(TARGET_OF(bank_index))].flinch = M_FLINCH(move);
            // if multi-hit not satisfied call again
            if (battle_master->b_moves[B_MOVE_BANK(bank_index)].hit_times > 0) {
                battle_master->b_moves[B_MOVE_BANK(bank_index)].hit_times--;
                battle_master->b_moves[B_MOVE_BANK(bank_index)].hit_counter++;
                if (is_fainted()) {
                    battle_master->b_moves[B_MOVE_BANK(bank_index)].hit_times = 0;
                    super.multi_purpose_state_tracker = S_AFTER_MOVE_SECONDARY;
                } else {
                    super.multi_purpose_state_tracker = S_MOVE_TRYHIT;
                    B_MOVE_DMG(bank_index) = 0;
                }
            } else {
                if (battle_master->b_moves[B_MOVE_BANK(bank_index)].hit_counter > 0) {
                    u16 temp = battle_master->b_moves[B_MOVE_BANK(bank_index)].hit_counter;
                    if (battle_master->b_moves[B_MOVE_BANK(bank_index)].hit_counter > 1) {
                        battle_master->b_moves[B_MOVE_BANK(bank_index)].hit_counter = 1;
                        battle_master->b_moves[B_MOVE_BANK(bank_index)].hit_times = 1;
                        damage_result_msg(bank_index);
                    }
                    battle_master->b_moves[B_MOVE_BANK(bank_index)].hit_counter = temp;
                    enqueue_message(0, 0, STRING_MULTI_HIT, battle_master->b_moves[B_MOVE_BANK(bank_index)].hit_counter);
                }
                super.multi_purpose_state_tracker = S_RUN_MOVE_HIT;
                set_callback1(run_move);
            }

            break;
        case S_AFTER_MOVE:
            {
                // add callbacks specific to field
                u8 attacker = bank_index;
                if (moves[move].on_after_move) {
                    add_callback(CB_ON_AFTER_MOVE, 0, 0, attacker, (u32)moves[move].on_after_move);
                }

                // run callbacks
                build_execution_order(CB_ON_AFTER_MOVE);
                battle_master->executing = true;
                while (battle_master->executing) {
                    pop_callback(attacker, move);
                }
                if (IS_RECHARGE(move)) {
                    ADD_VOLATILE(bank_index, VOLATILE_RECHARGING);
                }
                super.multi_purpose_state_tracker = S_RUN_FAINT;
                battle_master->c1_after_faint_check = run_move;
                battle_master->c1_prestate = S_RESIDUAL_MOVES;
                set_callback1(run_move);
            }
            break;
    };
}
