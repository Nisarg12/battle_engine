#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern void dprintf(const char * str, ...);

u8 gust_on_modify(u8 user, u8 target, u16 move){
    if ((CURRENT_MOVE(target) == MOVE_BOUNCE) || (CURRENT_MOVE(target) == MOVE_FLY)) {
        if (HAS_VOLATILE(target, VOLATILE_SEMI_INVULNERABLE)) {
            B_MOVE_POWER(user) *= 2;
        }
    }
    return 1;
}


