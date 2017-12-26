#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern void dprintf(const char * str, ...);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern void do_heal(u8 bank_index, u8 heal);
extern void set_status(u8 bank, enum Effect status);
extern void flat_heal(u8 bank, u16 heal);
extern u8 get_ability(struct Pokemon* p);
extern void silent_cure_major(u8 bank);

/* Purify */
void purify_on_after_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return;
    do_heal(user, 50);
    enqueue_message(NULL, user, STRING_HEAL, 0);
}

u8 purify_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (B_STATUS(user) != AILMENT_NONE) {
        set_status(user, EFFECT_CURE);
        add_callback(CB_ON_AFTER_MOVE, 0, 0, user, (u32)purify_on_after_move);
        return true;
    } else {
        return false;
    }
}

/* Strength sap */
void strength_sap_on_after_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return;
    flat_heal(user, acb->data_ptr);
    enqueue_message(NULL, user, STRING_HEAL, 0);
}

u8 strength_sap_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 id = add_callback(CB_ON_AFTER_MOVE, 0, 0, user, (u32)strength_sap_on_after_move);
    CB_MASTER[id].data_ptr = B_ATTACK_STAT(TARGET_OF(user));
    return true;
}


/* Sparkling Aria */
u8 sparkling_aria_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 target = TARGET_OF(user);
    if (B_STATUS(target) == AILMENT_BURN) {
        set_status(target, EFFECT_CURE);
    }
    return true;
}


/* Shore up */
u8 shore_up_on_modify_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (battle_master->field_state.is_sandstorm)
        B_HEAL(user) = 67;
    return true;
}


/* Aromatherapy */
extern void clear_ailments_silent(u8 bank);
u8 aromatherapy_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 partner_bank;
    if (SIDE_OF(user)) {
        partner_bank = (user > 2) ? 2 : 3;
    } else {
        partner_bank = (user > 0) ? 0 : 1;
    }
    bool success = false;
    struct Pokemon* p;
    struct Pokemon* user_p;
    struct Pokemon* partner;

    // get party & battle side members
    if (SIDE_OF(user)) {
        p =  &party_opponent[0];
        user_p = p_bank[user]->this_pkmn;
        partner = p_bank[((user > 2) ? 3 : 2)]->this_pkmn;
    } else {
        p =  &party_player[0];
        user_p = p_bank[user]->this_pkmn;
        partner = p_bank[((user > 0) ? 0 : 1)]->this_pkmn;
    }

    // Heal party
    for (u8 i = 0; i < 6; i++) {
        // don't check field pokemon
        if ((&p[i] == user_p) || (&p[i] == partner))
            continue;
        // don't heal sapsipper
        if (get_ability(&p[i]) == ABILITY_SAP_SIPPER)
            continue;
        // heal conditions
        if (pokemon_getattr(&p[i], REQUEST_STATUS_AILMENT, NULL)) {
            u8 zero = 0;
            pokemon_setattr(&p[i], REQUEST_STATUS_AILMENT, &zero);
            success = true;
        }
    }

    // heal field members
    if (B_STATUS(user) != AILMENT_NONE) {
        clear_ailments_silent(user);
        success = true;
    }

    if (B_STATUS(partner_bank) != AILMENT_NONE) {
        clear_ailments_silent(partner_bank);
    }
    if (success) {
        enqueue_message(NULL, user, STRING_SOOTHING_AROMA, 0);
    }
    return success;
}


/* refresh */
u8 refresh_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (B_STATUS(user)) {
        silent_cure_major(user);
        enqueue_message(0, user, STRING_AILMENT_CURED, 0);
        return true;
    }
    return false;
}


/* Heal Bell */
u8 heal_bell_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 partner_bank;
    if (SIDE_OF(user)) {
        partner_bank = (user > 2) ? 2 : 3;
    } else {
        partner_bank = (user > 0) ? 0 : 1;
    }
    bool success = false;
    struct Pokemon* p = (SIDE_OF(user)) ? (&party_opponent[0]) : (&party_player[0]);

    // Heal party
    for (u8 i = 0; i < 6; i++) {
        // don't heal sapsipper
        if (get_ability(&p[i]) == ABILITY_SOUNDPROOF)
            continue;
        // heal conditions
        u8 id;
        if ((id = pokemon_getattr(&p[i], REQUEST_STATUS_AILMENT, NULL))) {
            u8 zero = 0;
            dprintf("id %d was cured of %d\n", i, id);
            pokemon_setattr(&p[i], REQUEST_STATUS_AILMENT, &zero);
            success = true;
        }
    }

    // heal field members
    if (B_STATUS(user) != AILMENT_NONE) {
        clear_ailments_silent(user);
        success = true;
    }

    if (B_STATUS(partner_bank) != AILMENT_NONE) {
        clear_ailments_silent(partner_bank);
        success = true;
    }
    if (success) {
        enqueue_message(NULL, user, STRING_BELL_CHIMED, 0);
    }
    return success;
}

u8 heal_pulse_on_tryhit_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 target_bank = TARGET_OF(user);
    if (TOTAL_HP(target_bank) == B_CURRENT_HP(target_bank)) {
        return false;
    }
    return true;
}
