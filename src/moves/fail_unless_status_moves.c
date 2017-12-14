#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern void dprintf(const char * str, ...);
extern bool enqueue_message(u16 move, u8 user, enum battle_string_ids id, u16 effect);
extern u8 get_move_index(u16 move_id, u8 bank);
void set_status(u8 bank, enum Effect status);
extern u16 rand_range(u16 min, u16 max);
extern void do_damage(u8 bank_index, u16 dmg);


u8 snore_on_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if ((B_STATUS(TARGET_OF(user)) == AILMENT_SLEEP) || (BANK_ABILITY(TARGET_OF(user)) == ABILITY_COMATOSE))
        return true;
    return false;
}


/* Nightmare */
// Nightmare effect is removed is the sleep status is removed
extern u8 nightmare_on_status(u8 user, u8 src, u16 status, struct anonymous_callback* acb);
extern u8 nightmare_on_residual(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
u8 nightmare_on_modify_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if ((B_STATUS(TARGET_OF(user)) == AILMENT_SLEEP) || (BANK_ABILITY(TARGET_OF(user)) == ABILITY_COMATOSE))
        return true;

    // remove the residual
    delete_callback_src((u32)nightmare_on_residual, user);
    delete_callback_src((u32)nightmare_on_status, user);
    acb->in_use = false;
    return true;
}

u8 nightmare_on_status(u8 user, u8 src, u16 status, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if ((status == AILMENT_CURE) || (status == AILMENT_NONE)) {
        delete_callback_src((u32)nightmare_on_residual, user);
        delete_callback_src((u32)nightmare_on_modify_move, user);
        acb->in_use = false;
    }
    return true;
}

u8 nightmare_on_residual(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 status = B_STATUS(user);
    if ((status == AILMENT_SLEEP) || (BANK_ABILITY(TARGET_OF(user)) == ABILITY_COMATOSE)) {
        enqueue_message(MOVE_NIGHTMARE, user, STRING_CURSE_RESIDUAL, MOVE_NIGHTMARE);
        do_damage(user, MAX(1, (TOTAL_HP(user) / 4)));
    } else {
        delete_callback_src((u32)nightmare_on_status, user);
        delete_callback_src((u32)nightmare_on_modify_move, user);
    }
    return true;
}

u8 nightmare_on_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 status = B_STATUS(TARGET_OF(user));

    if ((status == AILMENT_SLEEP) || (BANK_ABILITY(TARGET_OF(user)) == ABILITY_COMATOSE)) {
        add_callback(CB_ON_RESIDUAL, 0, CB_PERMA, TARGET_OF(user), (u32)nightmare_on_residual);
        add_callback(CB_ON_STATUS, 0, CB_PERMA, TARGET_OF(user), (u32)nightmare_on_status);
        add_callback(CB_ON_MODIFY_MOVE, 0, CB_PERMA, TARGET_OF(user), (u32)nightmare_on_modify_move);
        return true;
    } else {
        return false;
    }
}
