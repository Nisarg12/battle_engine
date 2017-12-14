#include <pokeagb/pokeagb.h>
#include "../../../battle_data/pkmn_bank.h"
#include "../../../battle_data/pkmn_bank_stats.h"
#include "../../../battle_data/battle_state.h"
#include "../../../moves/moves.h"
#include "../../../battle_text/battle_pick_message.h"
#include "../../move_chain_states.h"

extern u16 rand_range(u16 min, u16 max);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);

bool try_hit(u8 attacker)
{
    // if move fails under gravity, fail
    if (HAS_VOLATILE(attacker, VOLATILE_GRAVITY) && IS_GRAVITY(CURRENT_MOVE(attacker))) {
        enqueue_message(CURRENT_MOVE(attacker), attacker, STRING_FAILED, 0);
        return false;
    }

    // if move fails under heal block, fail
    if (HAS_VOLATILE(attacker, VOLATILE_HEAL_BLOCK) && IS_HEAL(CURRENT_MOVE(attacker))) {
        enqueue_message(CURRENT_MOVE(attacker), attacker, STRING_FAILED, 0);
        return false;
    }

    // if target is in semi invulnerability do checks
    u8 defender = TARGET_OF(attacker);
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

    // if moves never misses, exit early
    u8 move_accuracy = B_MOVE_ACCURACY(attacker);
    if (move_accuracy > 100) {
        return true;
    } else {
        // OHKO moves have their own accuracy checks
        if (IS_OHKO(CURRENT_MOVE(attacker))) return true;
    }
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

/*
bool is_immune(u8 attacker, u8 defender, u16 move)
{
    return false;
}
*/
