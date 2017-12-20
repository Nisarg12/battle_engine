#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern void dprintf(const char * str, ...);
extern bool enqueue_message(u16 move, u8 user, enum battle_string_ids id, u16 effect);

u8 pollen_puff_on_tryhit_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 target_bank = TARGET_OF(user);
    if (SIDE_OF(user) == SIDE_OF(target_bank)) {
        B_MOVE_POWER(user) = 0;
        if (TOTAL_HP(target_bank) == B_CURRENT_HP(target_bank)) {
            return false;
        }
    } else {
        B_HEAL(user) = 0;
    }
    return true;
}
