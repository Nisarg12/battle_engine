#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../battle_events/battle_events.h"

extern void dprintf(const char * str, ...);

u8 whirlwind_on_tryhit_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 target = TARGET_OF(user);
    if ((HAS_VOLATILE(target, VOLATILE_INGRAIN)) ||
     (BANK_ABILITY(target) == ABILITY_SUCTION_CUPS)) {
        return 0; // fail move
    } else {
        return 1; // use move
    }
}

u8 whirlwind_on_effect_cb(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    // exit battle
    prepend_action(user, NULL, ActionRun, EventEndBattle);
    return 0;
}
