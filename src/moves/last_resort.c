#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern void dprintf(const char * str, ...);
extern bool enqueue_message(u16 move, u8 user, enum battle_string_ids id, u16 effect);
extern u8 count_total_moves(u8 bank);

u8 last_resort_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 count = count_total_moves(user);
    if (count < 2) return false; // Must know 2 moves
    for (u8 i = 0; i < count; i++) {
        if (p_bank[user]->b_data.moves[i] == MOVE_LAST_RESORT)
            continue;
        if (p_bank[user]->b_data.moves_used[i] == MOVE_NONE) {
            return false;
        }
    }
    return true;
}
