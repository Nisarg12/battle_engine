#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"
#include "battle_events/battle_events.h"
#include "../abilities/battle_abilities.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern bool is_grounded(u8 bank);
extern u8 get_move_index(u16 move_id, u8 bank);

void event_after_move(struct action* current_move)
{
    u8 bank = current_move->action_bank;
    u16 move = CURRENT_MOVE(bank);
    for (u8 i = 0; i < BANK_MAX; i++) {
        u8 ability = p_bank[i]->b_data.ability;
        if ((abilities[ability].on_after_move) && (ACTIVE_BANK(i)))
            add_callback(CB_ON_AFTER_MOVE, 0, 0, i, (u32)abilities[ability].on_after_move);
    }
    // add callbacks specific to field
    if (moves[move].on_after_move) {
        add_callback(CB_ON_AFTER_MOVE, 0, 0, bank, (u32)moves[move].on_after_move);
    }

    // run callbacks
    build_execution_order(CB_ON_AFTER_MOVE);
    battle_master->executing = true;
    while (battle_master->executing) {
        pop_callback(bank, move);
    }
    if (IS_RECHARGE(move)) {
        ADD_VOLATILE(bank, VOLATILE_RECHARGING);
    }

    /* Passive residual effects from engine structs */
    if (is_grounded(bank)) {
        B_IS_GROUNDED(bank) = true;
    }
    LAST_MOVE(bank) = move;
    // update moves used history
    p_bank[bank]->b_data.will_move = false;
    u8 slot = get_move_index(move, bank);
    p_bank[bank]->b_data.moves_used[slot] = move;
    battle_master->field_state.last_used_move = move;
    CURRENT_ACTION->event_state++;
}
