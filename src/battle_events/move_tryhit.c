#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"
#include "battle_events/battle_events.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern u16 rand_range(u16 min, u16 max);
extern void set_attack_bm_inplace(u16 move_id, u8 bank);
extern void reset_turn_bits(u8 bank);

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


bool tryhit_semi_invulnerble(u8 attacker, u8 defender, u16 move)
{
    if (HAS_VOLATILE(defender, VOLATILE_SEMI_INVULNERABLE)) {
        // add callbacks specific to field
        u16 move = CURRENT_MOVE(attacker);
        if (moves[move].on_inv_tryhit_move) {
            add_callback(CB_ON_TRYHIT_INV_MOVE, 0, 0, attacker, (u32)moves[move].on_inv_tryhit_move);
        }
        // run callbacks
        u8 old_index = CB_EXEC_INDEX;
        u32* old_execution_array = push_callbacks();
        build_execution_order(CB_ON_TRYHIT_INV_MOVE);
        battle_master->executing = true;
        bool is_inv = true;
        while (battle_master->executing) {
            if (!pop_callback(attacker, move)) {
                is_inv = false;
                break;
            }
        }
        restore_callbacks(old_execution_array);
        CB_EXEC_INDEX = old_index;
        if (is_inv) {
            // if user's move doesn't hit invulnerable target
            enqueue_message(0, attacker, STRING_INVULNERABLE, 0);
            return false;
        }
    }
    return true;
}


bool try_hit(u8 attacker)
{
    u16 move = CURRENT_MOVE(attacker);
    u8 defender = TARGET_OF(attacker);
    // if move fails under gravity, fail
    if (HAS_VOLATILE(attacker, VOLATILE_GRAVITY) && IS_GRAVITY(move)) {
        enqueue_message(move, attacker, STRING_FAILED, 0);
        return false;
    }

    // if move fails under heal block, fail
    if (HAS_VOLATILE(attacker, VOLATILE_HEAL_BLOCK) && IS_HEAL(move)) {
        enqueue_message(move, attacker, STRING_FAILED, 0);
        return false;
    }

    // if target is in semi invulnerability do checks
    if (!tryhit_semi_invulnerble(attacker, defender, move))
        return false;


    // if moves never misses, exit early
    u8 move_accuracy = B_MOVE_ACCURACY(attacker);
    if (move_accuracy > 100) {
        return true;
    }
    // OHKO moves have their own accuracy checks
    if (IS_OHKO(move))
        return true;
    // standard accuracy formula check
    u16 target_evasion;
    if (B_MOVE_IGNORE_EVASION(attacker)) {
        target_evasion = 100;
    } else {
        target_evasion = B_EVASION_STAT(defender);
    }
    u16 user_accuracy = B_ACCURACY_STAT(attacker);

    u16 result = (user_accuracy / target_evasion) * move_accuracy;
    if (rand_range(0, 100) <= result)
        return true;
    if (target_evasion > 100) {
        enqueue_message(0, defender, STRING_ATTACK_AVOIDED, 0);
    } else {
        enqueue_message(0, attacker, STRING_ATTACK_MISSED, 0);
    }
    return false;
}

void event_move_tryhit_external(struct action* current_action)
{
    u8 bank_index = current_action->action_bank;
    u16 move = current_action->move;
    CURRENT_MOVE(bank_index) = move;
    TARGET_OF(bank_index) = current_action->target;
    if (current_action->reset_move_config) {
        reset_turn_bits(bank_index);
        set_attack_bm_inplace(move, bank_index);
        current_action->reset_move_config = false;
    }
    /* Flinch interrupts move */
    if (rand_range(0, 100) < B_FLINCH(bank_index)) {
        enqueue_message(0, bank_index, STRING_FLINCHED, 0);
        B_FLINCH(bank_index) = 0;
        CURRENT_ACTION->event_state = EventMoveFailed;
        return;
    }
    B_FLINCH(bank_index) = 0;

    /* Move tryhit callbacks */
    switch (move_tryhit(bank_index, TARGET_OF(bank_index), move)) {
        case CANT_USE_MOVE:
            B_MOVE_FAILED(bank_index) = true;
            enqueue_message(move, bank_index, STRING_FAILED, move);
            break;
        case TARGET_MOVE_IMMUNITY:
            B_MOVE_FAILED(bank_index) = true;
            enqueue_message(0, bank_index, STRING_MOVE_IMMUNE, 0);
            break;
        case FAIL_SILENTLY:
            B_MOVE_FAILED(bank_index) = true;
            break;
        default:
            break;
    };
    if (B_MOVE_FAILED(bank_index)) {
        CURRENT_ACTION->event_state = EventMoveFailed;
        return;
    }

    /* General tryhit move */
    if (try_hit(bank_index)) {
        current_action->event_state++;
    } else {
        CURRENT_ACTION->event_state = EventMoveFailed;
    }
}
