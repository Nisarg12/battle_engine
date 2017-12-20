#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"
#include "battle_events.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);


void event_residual_effects(struct action* current_action)
{
	switch (current_action->priv[0]) {
        case 0:
            build_execution_order(CB_ON_RESIDUAL);
            battle_master->executing = true;
            s8 last_index = -1;
            for (u8 i = 0; i < 4; i++) {
                if (ACTIVE_BANK(battle_master->bank_order[i]) && (!B_FAINTED(battle_master->bank_order[i])))
                    last_index++;
            }
            if (last_index < 0) {
                end_action(CURRENT_ACTION);
                return;
            }
            battle_master->c1_prestate = last_index;
            current_action->priv[0]++;
            battle_master->bank_state = 0;
            break;
        case 1:
			{
			u8 bank = battle_master->bank_order[battle_master->bank_state];
			prepend_action(bank, NULL, ActionHighPriority, EventDoFaints);
            if (battle_master->executing) {
                if (battle_master->bank_state == battle_master->c1_prestate) {
                    if (B_FAINTED(bank) == false) {
                        pop_callback(bank , CURRENT_MOVE(bank));
                        battle_master->bank_state = 0;
                    } else {
                        CB_EXEC_INDEX++;
                        if (CB_EXEC_ORDER[CB_EXEC_INDEX] == ANON_CB_MAX)
                            end_action(CURRENT_ACTION);
                        battle_master->bank_state = 0;
                    }
                } else {
                    if (B_FAINTED(bank) == false)
                        run_callback(bank, CURRENT_MOVE(bank));
                    battle_master->bank_state++;
                }
            } else {
                end_action(CURRENT_ACTION);
                return;
            }
            break;
			}
        };
}
