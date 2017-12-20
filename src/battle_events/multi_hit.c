#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"
#include "battle_events/battle_events.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern bool damage_result_msg(u8 bank_index);

void event_multi_hit(struct action* current_action)
{
    u8 bank = current_action->action_bank;
    // set flinch chance of target
    B_FLINCH(TARGET_OF(bank)) = B_MOVE_FLINCH(bank);
    // if multi-hit not satisfied call again
    if (battle_master->b_moves[bank].hit_times > 0) {
        battle_master->b_moves[bank].hit_times--;
        battle_master->b_moves[bank].hit_counter++;
        if (B_CURRENT_HP(TARGET_OF(bank)) < 1) {
            battle_master->b_moves[bank].hit_times = 0;
            return; // rerun this state with the status of move hits completed
        } else {
            CURRENT_ACTION->event_state = EventMoveTryHit;
            B_MOVE_DMG(bank) = 0;
            B_MOVE_WILL_CRIT(bank) = false;
            return;
        }
    } else {
        // multi-hits complete. Damage strings.
        if (battle_master->b_moves[bank].hit_counter > 0) {
            u16 temp = battle_master->b_moves[bank].hit_counter;
            if (battle_master->b_moves[bank].hit_counter > 1) {
                battle_master->b_moves[bank].hit_counter = 1;
                battle_master->b_moves[bank].hit_times = 1;
                damage_result_msg(bank);
            }
            battle_master->b_moves[bank].hit_counter = temp;
            enqueue_message(0, 0, STRING_MULTI_HIT, battle_master->b_moves[bank].hit_counter);
        }
        // go back to hitlist config
        CURRENT_ACTION->event_state = EventPreMoveHit;
    }

}
