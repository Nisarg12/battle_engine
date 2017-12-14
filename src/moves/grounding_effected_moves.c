#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern void dprintf(const char * str, ...);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern void do_heal(u8 bank_index, u8 percent_heal);
extern void stat_boost(u8 bank, u8 stat_id, s8 amount);
extern bool is_grounded(u8 bank);
extern bool b_pkmn_has_type(u8 bank, enum PokemonType type);

/* The actual grounded Immunity was moved directly to the effectiveness calculator
 * Since it's shared in so many overlapping moves, that makes sense to do.
 */

/* Ingrain */
u8 ingrain_on_residual(u8 user, u8 src, u16 status_id, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (B_CURRENT_HP(user) != TOTAL_HP(user)) {
        do_heal(user, 6);
        enqueue_message(NULL, user, STRING_HEAL, 0);
        p_bank[user]->b_data.is_grounded = true;
    }
    return true;
}

u8 ingrain_on_effect(u8 user, u8 src, u16 status_id, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (has_callback_src((u32)ingrain_on_residual, user)) return false;
    ADD_VOLATILE(user, VOLATILE_INGRAIN);
    ADD_VOLATILE(user, VOLATILE_TRAPPED);
    p_bank[user]->b_data.is_grounded = true;
    add_callback(CB_ON_RESIDUAL, 0, 0xFF, user, (u32)ingrain_on_residual);
    enqueue_message(NULL, user, STRING_ROOTS_PLANTED, 0);
    return true;
}


/* Smack down */
u8 smackdown_on_invul_tryhit(u8 user, u8 src, u16 status_id, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 target = TARGET_OF(user);
    if (HAS_VOLATILE(target, VOLATILE_FLYING) || HAS_VOLATILE(target, VOLATILE_BOUNCE) ||
        HAS_VOLATILE(user, VOLATILE_SKYDROP)) {
        return false;
     }
     return true;
}

u8 smackdown_on_effect(u8 user, u8 src, u16 status_id, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 target = TARGET_OF(user);
    p_bank[target]->b_data.is_grounded = true;
    ADD_VOLATILE(target, VOLATILE_SMACK_DOWN);
    // Cancel moves Fly and Bounce and Sky attacks in effect manually
    if (HAS_VOLATILE(target, VOLATILE_FLYING)) {
        CLEAR_VOLATILE(target, VOLATILE_FLYING);
        CLEAR_VOLATILE(target, VOLATILE_SEMI_INVULNERABLE);
        CLEAR_VOLATILE(target, VOLATILE_CHARGING);
        enqueue_message(NULL, target, STRING_FELL_DOWN, NULL);
    } else if (HAS_VOLATILE(target, VOLATILE_BOUNCE)) {
        CLEAR_VOLATILE(target, VOLATILE_BOUNCE);
        CLEAR_VOLATILE(target, VOLATILE_SEMI_INVULNERABLE);
        CLEAR_VOLATILE(target, VOLATILE_CHARGING);
        enqueue_message(NULL, target, STRING_FELL_DOWN, NULL);
    } else if (HAS_VOLATILE(user, VOLATILE_SKYDROP)) {
        CLEAR_VOLATILE(target, VOLATILE_SKYDROP);
        CLEAR_VOLATILE(target, VOLATILE_SEMI_INVULNERABLE);
        CLEAR_VOLATILE(target, VOLATILE_CHARGING);
        enqueue_message(NULL, target, STRING_FELL_DOWN, NULL);
    }

    // remove effects of Telekinesis and Magnet rise
    if (HAS_VOLATILE(target, VOLATILE_MAGNET_RISE)) {
        extern u8 magnet_rise_on_residual(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
        extern u8 magnet_rise_on_effectiveness(u8 target_type, u8 src, u16 move_type, struct anonymous_callback* acb);
        delete_callback_src((u32)magnet_rise_on_residual, target);
        delete_callback_src((u32)magnet_rise_on_effectiveness, target);
        CLEAR_VOLATILE(target, VOLATILE_MAGNET_RISE);
    }
    if (HAS_VOLATILE(target, VOLATILE_TELEKINESIS)) {
        extern u8 telekinesis_on_residual(u8 user, u8 src, u16 move, struct anonymous_callback* acb);
        extern u8 telekinesis_on_effectiveness(u8 target_type, u8 src, u16 move_type, struct anonymous_callback* acb);
        delete_callback_src((u32)telekinesis_on_effectiveness, target);
        delete_callback_src((u32)telekinesis_on_residual, target);
        CLEAR_VOLATILE(target, VOLATILE_TELEKINESIS);
    }
    return true;
}


/* Rotiller */
u8 rototiller_on_effect(u8 user, u8 src, u16 status_id, struct anonymous_callback* acb)
{
    if (user != src) return true;
    bool boosted = false;
    for (u8 i = 0; i < BANK_MAX; i++) {
        if (b_pkmn_has_type(i, MTYPE_GRASS) && is_grounded(i)) {
            // boosts apply
            stat_boost(i, ATTACK_MOD, 1);
            stat_boost(i, SPATTACK_MOD, 1);
            boosted = true;
        }
    }
    return boosted;
}


/* Thousand Arrows */
u8 thousand_arrows_on_effectiveness(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    u8 attacker = acb->data_ptr >> 16;
    u16 effectiveness = ((acb->data_ptr << 16) >> 16);
    if (attacker != src) return effectiveness;
    if (b_pkmn_has_type(TARGET_OF(attacker), MTYPE_FLYING))
        return 100;
    return effectiveness;
}

u8 thousand_arrow_on_tryhit_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 target = TARGET_OF(user);
    if (!is_grounded(target)) {
        add_callback(CB_ON_EFFECTIVENESS, 0, 0, user, (u32)thousand_arrows_on_effectiveness);
    }
    // not grounded, apply grounding and effectiveness Callback.
    bool has_smackdown = HAS_VOLATILE(target, VOLATILE_SMACK_DOWN);
    smackdown_on_effect(user, src, move, acb);
    if (!has_smackdown) {
        CLEAR_VOLATILE(target, VOLATILE_SMACK_DOWN);
    }
    ADD_VOLATILE(target, VOLATILE_THOUSAND_ARROWS);
    B_IS_GROUNDED(target) = true;
    return true;
}
