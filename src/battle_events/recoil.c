#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern void do_damage(u8 bank_index, u16 dmg);

void event_move_recoil(struct action* current_action) {
    u8 bank = current_action->action_bank;
    u16 move = CURRENT_MOVE(bank);
    if (B_FAINTED(bank)) {
        CURRENT_ACTION->event_state++;
        return;
    }
    // check for recoil
    if (moves[move].recoil_struggle) {
        // struggle recoil is based off max health
        do_damage(bank, NUM_MOD(TOTAL_HP(bank), moves[move].recoil));
        enqueue_message(move, bank, STRING_RECOIL, 0);
    } else if (B_MOVE_DMG(bank) != 0 && moves[move].recoil > 0) {
        // normal recoil based on move damage
        do_damage(bank, NUM_MOD(B_MOVE_DMG(bank), moves[move].recoil));
        enqueue_message(move, bank, STRING_RECOIL, 0);
    }
    CURRENT_ACTION->event_state++;
}
