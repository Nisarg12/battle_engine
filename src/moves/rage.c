#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern bool b_pkmn_has_type(u8 bank, enum PokemonType type);
extern void stat_boost(u8 bank, u8 stat_id, s8 amount);
extern u16 rand_range(u16 min, u16 max);

void rage_on_damage_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (TARGET_OF(user) != src) return;
    if (B_MOVE_DMG(user) > 0) {
        // +1 atk if hit
        stat_boost(src, 0, 1);
    }
}

u8 rage_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    add_callback(CB_ON_DAMAGE_MOVE, 0, 0, src, (u32)rage_on_damage_move);
    return true;
}


// Ancient Power
u8 ancient_power_on_modify_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (rand_range(0, 100) < 10) {
        B_USER_STAT_MOD_CHANCE(user, ATTACK_MOD) = 100;
        B_USER_STAT_MOD_CHANCE(user, DEFENSE_MOD) = 100;
        B_USER_STAT_MOD_CHANCE(user, SPATTACK_MOD) = 100;
        B_USER_STAT_MOD_CHANCE(user, SPDEFENSE_MOD) = 100;
        B_USER_STAT_MOD_CHANCE(user, SPEED_MOD) = 100;
        B_USER_STAT_MOD_AMOUNT(user, ATTACK_MOD) = 1;
        B_USER_STAT_MOD_AMOUNT(user, DEFENSE_MOD) = 1;
        B_USER_STAT_MOD_AMOUNT(user, SPATTACK_MOD) = 1;
        B_USER_STAT_MOD_AMOUNT(user, SPDEFENSE_MOD) = 1;
        B_USER_STAT_MOD_AMOUNT(user, SPEED_MOD) = 1;
    }
    return true;
}
