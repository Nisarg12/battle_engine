#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern void dprintf(const char * str, ...);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern u8 move_pp_count(u16 move_id, u8 bank);
extern void set_attack_battle_master(u8 bank, u8 index, s8 priority);
extern bool knows_move(u16 move_id, u8 bank);

const static u16 encore_disallow[] = {
    MOVE_ASSIST, MOVE_COPYCAT, MOVE_ENCORE, MOVE_ME_FIRST,
    MOVE_METRONOME, MOVE_MIMIC, MOVE_MIRROR_MOVE,
    MOVE_NATURE_POWER, MOVE_SKETCH, MOVE_SLEEP_TALK,
    MOVE_STRUGGLE, MOVE_TRANSFORM, MOVE_NONE,
};

/* Encore */
u8 encore_on_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (move_pp_count(LAST_MOVE(TARGET_OF(user)), TARGET_OF(user)) < 1) return false;
    for (u8 i = 0; i < (sizeof(encore_disallow) / sizeof(u16)); i++) {
        if (LAST_MOVE(TARGET_OF(user)) == encore_disallow[i])
            return false;
    }
    return true;
}

u8 encore_on_disable(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (move_pp_count(LAST_MOVE(user), user) < 1) {
        acb->in_use = false;
        return true;
    }
    if (move != LAST_MOVE(user)) {
        // we want to display a message when player picks an invalid move.
        // Obviously not display the message to the AI.
        if (!SIDE_OF(user))
            enqueue_message(move, user, STRING_CANT_REASON, MOVE_ENCORE);
        return false;
    }
    return true;
}

u8 encore_on_residual(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (move_pp_count(CURRENT_MOVE(user), user) < 1) {
        acb->in_use = false;
        p_bank[user]->b_data.skip_move_select = false;
    }
    return true;
}

u8 encore_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 target = TARGET_OF(user);
    // fail if first turn/no move selected last turn
    if (LAST_MOVE(target) == 0) return false;
    // fail if target's move has no PP left
    if (move_pp_count(LAST_MOVE(target), target) < 1) return false;
    // fail if encore active on target already
    if (has_callback_src((u32)encore_on_disable, target)) return false;
    CURRENT_MOVE(target) = LAST_MOVE(target);
    set_attack_battle_master(target, B_MOVE_BANK(target), 0);
    add_callback(CB_ON_RESIDUAL, 0, 3, target, (u32)encore_on_residual);
    add_callback(CB_ON_DISABLE_MOVE, 0, 3, target, (u32)encore_on_disable);
    enqueue_message(NULL, target, STRING_RECIEVED_ENCORE, NULL);
    return true;
}


/* Torment */
u8 torment_on_disable(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (move == LAST_MOVE(user)) {
        // we want to display a message when player picks an invalid move.
        // Obviously not display the message to the AI.
        if (!SIDE_OF(user))
            enqueue_message(move, user, STRING_CANT_REASON, MOVE_TORMENT);
        return false;
    }
    return true;
}

u8 torment_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (has_callback_src((u32)torment_on_disable, TARGET_OF(user))) return false;
    add_callback(CB_ON_DISABLE_MOVE, 0, 0xFF, TARGET_OF(user), (u32)torment_on_disable);
    enqueue_message(NULL, TARGET_OF(user), STRING_SUBJECTED_TORM, NULL);
    return true;
}


/* Taunt */
u8 taunt_on_before_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != TARGET_OF(src)) return true;
    enqueue_message(CURRENT_MOVE(user), user, STRING_ATTACK_USED, 0);
    return (!B_MOVE_IS_STATUS(move));
}

u8 taunt_on_disable(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (IS_MOVE_STATUS(move)) {
        // we want to display a message when player picks an invalid move.
        // Obviously not display the message to the AI.
        if (!SIDE_OF(user))
            enqueue_message(move, user, STRING_CANT_REASON, MOVE_TORMENT);
        return false;
    }
    return true;
}

u8 taunt_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (has_callback_src((u32)taunt_on_disable, TARGET_OF(user))) return false;
    add_callback(CB_ON_DISABLE_MOVE, 0, 0x5, TARGET_OF(user), (u32)taunt_on_disable);
    enqueue_message(NULL, TARGET_OF(user), STRING_FELL_FOR_TAUNT, NULL);
    return true;
}


/* Disable */
u8 disable_on_before_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != TARGET_OF(src)) return true;
    enqueue_message(CURRENT_MOVE(user), user, STRING_ATTACK_USED, 0);
    return false;
}

bool disable_on_disable_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (move == acb->data_ptr) {
        // we want to display a message when player picks an invalid move.
        // Obviously not display the message to the AI.
        if (!SIDE_OF(user))
            enqueue_message(move, user, STRING_CANT_REASON, MOVE_DISABLE);
        return false;
    }
    return true;
}

u8 disable_on_effect_cb(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
	if (user != src) return true;
    u8 target = TARGET_OF(user);
    // fail if first turn/no move selected last turn
    if (LAST_MOVE(target) == 0) return false;
    // fail if target's move has no PP left
    if (move_pp_count(LAST_MOVE(target), target) < 1) return false;
    // fail if effect already active on target
    if (has_callback_src((u32)disable_on_disable_move, target)) return false;
    u8 id = add_callback(CB_ON_DISABLE_MOVE, 0, 4, target, (u32)disable_on_disable_move);
    CB_MASTER[id].data_ptr = LAST_MOVE(target);
    enqueue_message(LAST_MOVE(target), user, STRING_DISABLED, 0);
	return true;
}


/* Throat Chop */
u8 throat_chop_on_before_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != TARGET_OF(src)) return true;
    if (IS_SOUND_BASE(CURRENT_MOVE(user))) {
        enqueue_message(CURRENT_MOVE(user), user, STRING_ATTACK_USED, 0);
        return false;
    } else {
        return true;
    }
}

bool throat_chop_disable_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (IS_SOUND_BASE(move)) {
        // we want to display a message when player picks an invalid move.
        // Obviously not display the message to the AI.
        if (!SIDE_OF(user))
            enqueue_message(move, user, STRING_CANT_REASON, MOVE_THROAT_CHOP);
        return false;
    }
    return true;
}

u8 throat_chop_on_effect_cb(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
	if (user != src) return true;
    u8 target = TARGET_OF(user);
    // don't reapply effect if effect is active
    if (has_callback_src((u32)throat_chop_disable_move, target)) return true;
    add_callback(CB_ON_DISABLE_MOVE, 0, 2, target, (u32)throat_chop_disable_move);
	return true;
}

/* Imprison */
bool imprison_disable_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user == src) return true;
    if (knows_move(move, src)){
        if (!SIDE_OF(user))
            enqueue_message(move, user, STRING_CANT_REASON, MOVE_IMPRISON);
        return false;
    }
    return true;
}

u8 imprison_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
	if (user != src) return true;
    // don't reapply effect if effect is active, fail move
    if (has_callback_src((u32)imprison_disable_move, user)) return false;
    add_callback(CB_ON_DISABLE_MOVE, 0, CB_PERMA, user, (u32)imprison_disable_move);
	return true;
}
