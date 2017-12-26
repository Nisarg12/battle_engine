#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"
#include "battle_events/battle_events.h"
#include "../abilities/battle_abilities.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);

enum BeforeMoveStatus {
    CANT_USE_MOVE = 0,
    USE_MOVE_NORMAL,
    TARGET_MOVE_IMMUNITY,
    SILENT_FAIL,
    SILENT_CONTINUE,
};


enum BeforeMoveStatus before_move_cb(u8 attacker)
{
    for (u8 i = 0; i < BANK_MAX; i++) {
        u8 ability = p_bank[i]->b_data.ability;
        if ((abilities[ability].before_move) && (ACTIVE_BANK(i)))
            add_callback(CB_ON_BEFORE_MOVE, 0, 0, i, (u32)abilities[ability].before_move);
    }
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

void event_before_move(struct action* current_action)
{
    /* If bank is recharging, then recharge and end action. */
    if (HAS_VOLATILE(ACTION_BANK, VOLATILE_RECHARGING)) {
        CLEAR_VOLATILE(ACTION_BANK, VOLATILE_RECHARGING);
        enqueue_message(0, ACTION_BANK, STRING_MUST_RECHARGE, 0);
        return;
    }
    CURRENT_MOVE(ACTION_BANK) = CURRENT_ACTION->move;
    /* Resolve before move callbacks */
    u8 result = before_move_cb(ACTION_BANK);
    switch (result) {
        case CANT_USE_MOVE:
        case TARGET_MOVE_IMMUNITY:
        case SILENT_FAIL:
            enqueue_message(0, ACTION_BANK, STRING_FAILED, 0);
            end_action(current_action);
            return;
        case USE_MOVE_NORMAL:
            break;
    };

    /* Before Move effects which cause turn ending */

    if (HAS_VOLATILE(ACTION_BANK, VOLATILE_SLEEP_TURN)) {
        enqueue_message(0, ACTION_BANK, STRING_FAST_ASLEEP, 0);
        B_MOVE_FAILED(ACTION_BANK) = true;
    } else if (HAS_VOLATILE(ACTION_BANK, VOLATILE_CONFUSE_TURN) || HAS_VOLATILE(ACTION_BANK, VOLATILE_CHARGING)) {
        B_MOVE_FAILED(ACTION_BANK) = true;
    } else if (HAS_VOLATILE(ACTION_BANK, VOLATILE_ATK_SKIP_TURN)) {
        B_MOVE_FAILED(ACTION_BANK) = true;
        CLEAR_VOLATILE(ACTION_BANK, VOLATILE_ATK_SKIP_TURN);
    } else {
        B_MOVE_FAILED(ACTION_BANK) = false;
    }
    if (B_MOVE_FAILED(ACTION_BANK)) {
        CURRENT_ACTION->event_state = EventMoveFailed;
        return;
    }
    // display "Pokemon used move!"
    enqueue_message(CURRENT_MOVE(ACTION_BANK), ACTION_BANK, STRING_ATTACK_USED, 0);
    current_action->event_state++;
}
