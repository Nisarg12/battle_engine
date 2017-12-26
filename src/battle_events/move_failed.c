#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"
#include "battle_events/battle_events.h"
#include "../abilities/battle_abilities.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);

void event_move_failed(struct action* current_action)
{
    u8 bank = current_action->action_bank;
    for (u8 i = 0; i < BANK_MAX; i++) {
        u8 ability = p_bank[i]->b_data.ability;
        if ((abilities[ability].on_fail) && (ACTIVE_BANK(i)))
            add_callback(CB_ON_MOVE_FAIL, 0, 0, i, (u32)abilities[ability].on_fail);
    }
    u16 move = CURRENT_MOVE(bank);
    // add callbacks specific to field
    if (moves[move].on_move_fail) {
        add_callback(CB_ON_MOVE_FAIL, 0, 0, bank, (u32)moves[move].on_move_fail);
    }
    // run callbacks
    build_execution_order(CB_ON_MOVE_FAIL);
    battle_master->executing = true;
    while (battle_master->executing) {
        pop_callback(bank, move);
    }
    CURRENT_ACTION->event_state = EventDoFaints;
}
