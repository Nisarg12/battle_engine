#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"
#include "battle_events/battle_events.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern void dprintf(const char * str, ...);
extern void give_exp(u8 fainted, u8 reciever);


void wild_battle_status_update(struct action* current_action)
{
    // if the player side has fainted and there are no switch-ins, end the battle (port PC)
    if (B_IS_FAINTED(PLAYER_SINGLES_BANK)) {
        for (u8 i = 0; i < 6; i++) {
            u16 species = pokemon_getattr(&party_player[i], REQUEST_SPECIES, NULL);
            bool is_egg = pokemon_getattr(&party_player[i], REQUEST_IS_EGG, NULL);
            u16 current_hp = pokemon_getattr(&party_player[i], REQUEST_CURRENT_HP, NULL);

            // valid if it's a valid species, isn't an egg, and is alive.
            if ((species < SPECIES_MAX) && (species > SPECIES_MISSINGNO) &&
             (!is_egg) && (current_hp > 0)) {
                // TODO: force pick of a switch-in
                CURRENT_ACTION->event_state = EventEndBattle;
                dprintf("should force a switch\n");
                //end_action(CURRENT_ACTION);
                return;
            }
        }
    }

    // if foe side has fainted end the battle
    if (B_IS_FAINTED(OPPONENT_SINGLES_BANK)) {
        CURRENT_ACTION->event_state = EventEndBattle;
    } else {
        end_action(CURRENT_ACTION);
    }
}
