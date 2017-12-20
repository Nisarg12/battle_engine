#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern void flat_heal(u8 bank, u16 heal);

void event_move_drain(struct action* current_action)
{
    u8 bank = current_action->action_bank;
    u16 move = CURRENT_MOVE(bank);
    if (B_FAINTED(bank)) {
        CURRENT_ACTION->event_state++;
        return;
    }
    if ((B_MOVE_DMG(bank) > 0) && (moves[move].drain > 0))
        flat_heal(bank, NUM_MOD(B_MOVE_DMG(bank), moves[move].drain));
    CURRENT_ACTION->event_state++;
}
