#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern void dprintf(const char * str, ...);
extern bool enqueue_message(u16 move, u8 user, enum battle_string_ids id, u16 effect);
extern void do_damage(u8 bank_index, u16 dmg);
extern bool b_pkmn_has_type(u8 bank, enum PokemonType type);


u8 powder_on_tryhit_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (TARGET_OF(src) != user) return true;
    if (B_MOVE_HAS_TYPE(user, MTYPE_FIRE)) {
        u16 dmg = MIN((TOTAL_HP(user) >> 2), B_CURRENT_HP(user));
        do_damage(user, MAX(1, dmg));
        return false;
    }
    return true;
}

u8 venom_drench_on_tryhit_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    return (B_STATUS(TARGET_OF(user)) == AILMENT_POISON);
}

u8 flower_shield_before_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    for (u8 i = 0; i < BANK_MAX; i++) {
        if (b_pkmn_has_type(TARGET_OF(user), MTYPE_GRASS))
            return true;
    }
    enqueue_message(move, user, STRING_ATTACK_USED, 0);
    return false;
}

u8 flower_shield_on_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    // non grass types fail silently
    return (b_pkmn_has_type(TARGET_OF(user), MTYPE_GRASS)) ? 1 : 3;
}

u8 cotton_spore_on_tryhit_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    return (!(b_pkmn_has_type(TARGET_OF(src), MTYPE_GRASS)));
}

u8 synchonoise_on_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    for (u8 i = 0; i < sizeof(p_bank[user]->b_data.type); i++) {
        if (p_bank[user]->b_data.type[i] == MTYPE_EGG)
            continue;
        if (b_pkmn_has_type(TARGET_OF(user), p_bank[user]->b_data.type[i])) {
            dprintf("type is shared\n");
            return true;
        }
    }
    return false;
}


u8 focus_energy_on_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (HAS_VOLATILE(user, VOLATILE_FOCUS_ENERGY))
        return false;
    ADD_VOLATILE(user, VOLATILE_FOCUS_ENERGY);
    return true;
}
