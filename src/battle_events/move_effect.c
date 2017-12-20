#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"
#include "battle_events/battle_events.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);


void event_move_effect(struct action* current_action)
{
    /* execute move effect */
    u8 attacker = current_action->action_bank;
    u16 move = CURRENT_MOVE(attacker);

    if (B_FAINTED(attacker)) {
        CURRENT_ACTION->event_state++;
        return;
    }

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
            CURRENT_ACTION->event_state = EventMoveFailed;
            return;
        }
    }
    CURRENT_ACTION->event_state++;
}
