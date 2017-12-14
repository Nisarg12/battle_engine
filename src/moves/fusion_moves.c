#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern void dprintf(const char * str, ...);
extern bool enqueue_message(u16 move, u8 user, enum battle_string_ids id, u16 effect);

void fusion_flare_on_base_power(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return;
    for (u8 i = 0; i < BANK_MAX; i++) {
        if (LAST_MOVE(i) == MOVE_FUSION_BOLT) {
            B_MOVE_POWER(user) *= 2;
            return;
        }
    }
}

void fusion_bolt_on_base_power(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return;
    for (u8 i = 0; i < BANK_MAX; i++) {
        if (LAST_MOVE(i) == MOVE_FUSION_FLARE) {
            B_MOVE_POWER(user) *= 2;
            return;
        }
    }
}
