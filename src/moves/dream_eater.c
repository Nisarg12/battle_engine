#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern void dprintf(const char * str, ...);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);

u8 dream_eater_on_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    return ((B_STATUS(TARGET_OF(user)) == AILMENT_SLEEP) || (BANK_ABILITY(TARGET_OF(user)) == ABILITY_COMATOSE));
}
