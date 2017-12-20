#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../battle_events/battle_events.h"


extern u16 rand_range(u16 min, u16 max);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern void dprintf(const char * str, ...);
extern u8 count_usable_moves(u8 bank);
extern u8 count_total_moves(u8 bank);
extern void set_attack_bm_inplace(u16 move, u8 new_bank);

/* Metronome */

const static u16 metronome_disallow[] = {
	MOVE_AFTER_YOU, MOVE_ASSIST, MOVE_BELCH, MOVE_BESTOW,
	MOVE_CELEBRATE, MOVE_CHATTER, MOVE_COPYCAT, MOVE_COUNTER,
	MOVE_COVET, MOVE_CRAFTY_SHIELD, MOVE_DESTINY_BOND, MOVE_DETECT,
	MOVE_DIAMOND_STORM, MOVE_DRAGON_ASCENT, MOVE_ENDURE, MOVE_FEINT,
	MOVE_FOCUS_PUNCH, MOVE_FOLLOW_ME, MOVE_FREEZE_SHOCK, MOVE_HAPPY_HOUR,
	MOVE_HELPING_HAND, MOVE_HOLD_HANDS, MOVE_HYPERSPACE_FURY,
	MOVE_HYPERSPACE_HOLE, MOVE_ICE_BURN, MOVE_KINGS_SHIELD,
	MOVE_LIGHT_OF_RUIN, MOVE_MAT_BLOCK, MOVE_ME_FIRST, MOVE_METRONOME,
	MOVE_MIMIC, MOVE_MIRROR_COAT, MOVE_MIRROR_MOVE, MOVE_NATURE_POWER,
	MOVE_ORIGIN_PULSE, MOVE_PRECIPICE_BLADES, MOVE_PROTECT, MOVE_QUASH,
	MOVE_QUICK_GUARD, MOVE_RAGE_POWDER, MOVE_RELIC_SONG, MOVE_SECRET_SWORD,
	MOVE_SKETCH, MOVE_SLEEP_TALK, MOVE_SNARL, MOVE_SNATCH,
	MOVE_SNORE, MOVE_SPIKY_SHIELD, MOVE_STEAM_ERUPTION, MOVE_STRUGGLE,
	MOVE_SWITCHEROO, MOVE_TECHNO_BLAST, MOVE_THIEF, MOVE_THOUSAND_ARROWS,
	MOVE_THOUSAND_WAVES, MOVE_TRANSFORM, MOVE_TRICK,
	MOVE_VCREATE, MOVE_WIDE_GUARD, MOVE_MAX,
};

u16 pick_rand_metronome_move()
{
	bool unusable_move = true;
    u16 selected_move = MOVE_NONE;
	while (unusable_move) {
		selected_move = rand_range(1, MOVE_MAX);
		for (u8 i = 0; i < (sizeof(metronome_disallow) / sizeof(u16)); i++) {
			if (metronome_disallow[i] == selected_move) {
				unusable_move = false;
				break;
			}
		}
		unusable_move = (unusable_move) ? false : true;
	}
	return selected_move;
}

u8 metronome_on_modify_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
	if (user != src) return true;
    CURRENT_MOVE(user) = pick_rand_metronome_move();
	set_attack_bm_inplace(CURRENT_MOVE(user), user);
	LAST_MOVE(user) = MOVE_METRONOME;
    enqueue_message(CURRENT_MOVE(user), user, STRING_ATTACK_USED, 0);
    return true;
}

/* Mirror Move */
u8 mirror_move_on_modify_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
	if (user != src) return true;
    // fails if target hasn't made a move or target's move isn't mirrorable
	u8 target = TARGET_OF(user);
    if ((LAST_MOVE(target) != MOVE_NONE) && (IS_MIRRORABLE(LAST_MOVE(target)))) {
        CURRENT_MOVE(user) = LAST_MOVE(target);
        set_attack_bm_inplace(CURRENT_MOVE(user), user);
		LAST_MOVE(user) = MOVE_MIRROR_MOVE;
        enqueue_message(CURRENT_MOVE(user), user, STRING_ATTACK_USED, 0);
        return true;
    }
    return false;
}

/* Sleep Talk */
const static u16 sleep_talk_disallow[] = {
    MOVE_ASSIST, MOVE_BEAK_BLAST, MOVE_BELCH,
    MOVE_BIDE, MOVE_CHATTER, MOVE_COPYCAT,
    MOVE_FOCUS_PUNCH, MOVE_ME_FIRST, MOVE_METRONOME,
    MOVE_MIMIC, MOVE_MIRROR_MOVE, MOVE_NATURE_POWER,
    MOVE_SHELL_TRAP, MOVE_SKETCH, MOVE_SLEEP_TALK, MOVE_UPROAR,

};

bool is_allowed_sleep_talk(u16 move)
{
    for (u8 i = 0; i < (sizeof(sleep_talk_disallow) / sizeof(u16)); i++) {
        if (move == sleep_talk_disallow[i])
            return false;
    }
    return true;
}

u8 sleep_talk_before_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
	if (user != src) return true;
    if ((B_STATUS(user) == AILMENT_SLEEP) || (BANK_ABILITY(user) == ABILITY_COMATOSE)) {
        CLEAR_VOLATILE(user, VOLATILE_SLEEP_TURN);
        enqueue_message(0, user, STRING_FAST_ASLEEP, 0);
    }
	// don't fail here, fail after posting "Used move" string
	return true;

}

u8 sleep_talk_on_modify_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
	if (user != src) return true;
    if ((B_STATUS(user) == AILMENT_SLEEP) || (BANK_ABILITY(user) == ABILITY_COMATOSE)) {
        // fail if no moves learnt
        u16 move_set[4] = {MOVE_NONE, MOVE_NONE, MOVE_NONE, MOVE_NONE};
        u8 array_slot = 0;
        for (u8 i = 0; i < 4; i++) {
            u16 this_move = B_GET_MOVE(user, i);
            if ((this_move != MOVE_NONE) && (is_allowed_sleep_talk(this_move)) && (!(IS_CHARGE(this_move)))) {
                move_set[array_slot] = this_move;
                array_slot++;
            }
        }
        if (array_slot) {
            CURRENT_MOVE(user) = move_set[rand_range(0, array_slot)];
            set_attack_bm_inplace(CURRENT_MOVE(user), user);
			CURRENT_ACTION->move = CURRENT_MOVE(user);
			LAST_MOVE(user) = MOVE_SLEEP_TALK;
            enqueue_message(CURRENT_MOVE(user), user, STRING_ATTACK_USED, 0);
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}


/* Assist */
const static u16 assist_disallow[] = {
    MOVE_ASSIST, MOVE_BELCH, MOVE_BESTOW,
    MOVE_BOUNCE, MOVE_CHATTER, MOVE_CIRCLE_THROW,
    MOVE_COPYCAT, MOVE_COUNTER, MOVE_COVET,
    MOVE_DESTINY_BOND, MOVE_DETECT, MOVE_DIG,
    MOVE_DIVE, MOVE_DRAGON_TAIL, MOVE_ENDURE,
    MOVE_FEINT, MOVE_FLY, MOVE_FOCUS_PUNCH,
    MOVE_FOLLOW_ME, MOVE_HELPING_HAND, MOVE_KINGS_SHIELD,
    MOVE_MAT_BLOCK, MOVE_ME_FIRST, MOVE_METRONOME,
    MOVE_MIMIC, MOVE_MIRROR_COAT, MOVE_MIRROR_MOVE,
    MOVE_NATURE_POWER, MOVE_PHANTOM_FORCE, MOVE_PROTECT,
    MOVE_RAGE_POWDER, MOVE_ROAR, MOVE_SHADOW_FORCE,
    MOVE_SKETCH, MOVE_SKY_DROP, MOVE_SLEEP_TALK,
    MOVE_SNATCH, MOVE_SPIKY_SHIELD, MOVE_STRUGGLE,
    MOVE_SWITCHEROO, MOVE_THIEF, MOVE_TRANSFORM,
    MOVE_TRICK, MOVE_WHIRLWIND, MOVE_MAX, MOVE_NONE,
};

bool is_allowed_assist(u16 move)
{
    for (u8 i = 0; i < ((sizeof(assist_disallow)) / sizeof(u16)); i++) {
        if (assist_disallow[i] == move)
            return false;
    }
    return true;
}

u8 assist_on_modify_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
	if (user != src) return true;
    u16 move_set[24] = {MOVE_NONE};
    u8 array_slot = 0;
    u8 poke_count = count_pokemon();
    struct Pokemon* bank_party = (SIDE_OF(user)) ? (&party_opponent[0]) : (&party_player[0]);
    for (u8 i = 0; i < poke_count; i++) {
        if ((u32)&bank_party[i] == (u32)p_bank[user]->this_pkmn)
            continue;
        for (u8 j = 0; j < 4; j++) {
            u16 this_move = pokemon_getattr(&bank_party[i], REQUEST_MOVE1 + j, NULL);
            if (is_allowed_assist(this_move)) {
                move_set[array_slot] = this_move;
                array_slot++;
            }
        }
    }

    if (array_slot) {
        CURRENT_MOVE(user) = move_set[rand_range(0, array_slot)];
		CURRENT_ACTION->move = CURRENT_MOVE(user);
        set_attack_bm_inplace(CURRENT_MOVE(user), user);
        enqueue_message(CURRENT_MOVE(user), user, STRING_ATTACK_USED, 0);
		LAST_MOVE(user) = MOVE_ASSIST;
        return true;
    } else {
        return false;
    }
}


/* Copycat */
const static u16 copycat_disallow[] = {
    MOVE_ASSIST, MOVE_BANEFUL_BUNKER, MOVE_BESTOW, MOVE_CHATTER,
    MOVE_CIRCLE_THROW, MOVE_COPYCAT, MOVE_COUNTER, MOVE_COVET,
    MOVE_DESTINY_BOND, MOVE_DETECT, MOVE_DRAGON_TAIL, MOVE_ENDURE,
    MOVE_FEINT, MOVE_FOCUS_PUNCH, MOVE_FOLLOW_ME, MOVE_HELPING_HAND,
    MOVE_ME_FIRST, MOVE_METRONOME, MOVE_MIMIC, MOVE_MIRROR_COAT,
    MOVE_MIRROR_MOVE, MOVE_NATURE_POWER, MOVE_PROTECT,
    MOVE_RAGE_POWDER, MOVE_ROAR, MOVE_SKETCH, MOVE_SLEEP_TALK,
    MOVE_SNATCH, MOVE_STRUGGLE, MOVE_SWITCHEROO, MOVE_THIEF,
    MOVE_TRANSFORM, MOVE_TRICK, MOVE_WHIRLWIND, MOVE_NONE, MOVE_MAX
};

bool is_allowed_copycat(u16 move)
{
    for (u8 i = 0; i < ((sizeof(copycat_disallow)) / sizeof(u16)); i++) {
        if (copycat_disallow[i] == move)
            return false;
    }
    return true;
}

u8 copycat_on_modify_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
	if (user != src) return true;
    u16 last_move = battle_master->field_state.last_used_move;
    if (is_allowed_copycat(last_move)) {
        CURRENT_MOVE(user) = last_move;
        set_attack_bm_inplace(CURRENT_MOVE(user), user);
        enqueue_message(CURRENT_MOVE(user), user, STRING_ATTACK_USED, 0);
		LAST_MOVE(user) = MOVE_COPYCAT;
        return true;
    } else {
        return false;
    }
}

/* Magic coat */
u16 magic_coat_tryhit_anon(u8 user, u8 source, u16 move, struct anonymous_callback* acb)
{
	if ((user == source) || ACTION_BOUNCED || (!IS_REFLECTABLE(move)))
		return true;
	struct action* a = next_action(user, user, ActionMove, EventMoveTryHit);
	a->move = CURRENT_MOVE(user);
	a->target = user;
	a->has_bounced = true;
	a->reset_move_config = true;
	enqueue_message(CURRENT_MOVE(user), source, STRING_BOUNCED_BACK, 0);
	B_MOVE_FAILED(user) = true;
	acb->in_use = false;
	return 3; // fail silently
}

u8 magic_coat_on_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
	if (user != src) return true;
	enqueue_message(CURRENT_MOVE(user), user, STRING_SHROUDED_MAGICCOAT, 0);
	add_callback(CB_ON_TRYHIT_MOVE, 2, 0, user, (u32)(magic_coat_tryhit_anon));
	return true;
}


/* Me first */
const static u16 me_first_disallow[] = {
	MOVE_CHATTER, MOVE_COUNTER, MOVE_COVET, MOVE_FOCUS_PUNCH,
	MOVE_ME_FIRST, MOVE_METAL_BURST, MOVE_MIRROR_COAT,
	MOVE_STRUGGLE, MOVE_THIEF, MOVE_MAX, MOVE_NONE
};

u8 me_first_on_base_power_anon(u8 attacker, u8 src, u16 move, struct anonymous_callback* acb)
{
	u16 base_power = B_MOVE_POWER(attacker);
	if (attacker == src) {
		acb->in_use = false;
		B_MOVE_POWER(attacker) = NUM_MOD(base_power, 150);
	}
	return true;
}

u8 me_first_on_tryhit(u8 user, u8 src, u16 move_user, struct anonymous_callback* acb)
{
	if (user != src) return true;
	// fail if user hasn't moved before target
	if (user != 0)
		return false;
	u8 defender = TARGET_OF(user);
	u16 move = CURRENT_MOVE(defender);
	if (IS_MOVE_STATUS(move))
		return false;
	for (u8 i = 0; i < (sizeof(me_first_disallow) / sizeof(u16)); i++) {
		if (me_first_disallow[i] == move)
			return false;
	}

	// move is valid to be copied
	CURRENT_MOVE(user) = move;
	set_attack_bm_inplace(CURRENT_MOVE(user), user);
	LAST_MOVE(user) = MOVE_ME_FIRST;
	enqueue_message(CURRENT_MOVE(user), user, STRING_ATTACK_USED, 0);
	add_callback(CB_ON_BASE_POWER_MOVE, 4, 0, user, (u32)me_first_on_base_power_anon);
	acb->in_use = false;
	return true;
}

/* Snatch */
u16 statch_tryhit_anon(u8 user, u8 source, u16 move, struct anonymous_callback* acb)
{
	if ((user == source) || (!IS_SNATCHABLE(move))) {
		return true;
	}
	struct action* a = next_action(source, source, ActionMove, EventMoveTryHit);
	a->target = source,
	a->has_bounced = true;
	a->reset_move_config = true;
	a->move = CURRENT_MOVE(user);
	enqueue_message(0, source, STRING_SNATCHED_MOVE, 0);
	acb->in_use = false;
	return 3;
}

u8 snatch_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
	if (user != src) return true;
	enqueue_message(CURRENT_MOVE(user), user, STRING_SNATCH_WAITING, 0);
	add_callback(CB_ON_TRYHIT_MOVE, 0, 0, user, (u32)(statch_tryhit_anon));
	acb->in_use = false;
	return true;
}


/* Instruct */
const static u16 instruct_disallow[] = {
	MOVE_ASSIST, MOVE_BEAK_BLAST, MOVE_BIDE, MOVE_COPYCAT, MOVE_FOCUS_PUNCH,
	MOVE_ICE_BALL, MOVE_INSTRUCT, MOVE_ME_FIRST, MOVE_METRONOME, MOVE_MIMIC,
	MOVE_MIRROR_MOVE, MOVE_NATURE_POWER, MOVE_OUTRAGE, MOVE_PETAL_DANCE,
	MOVE_ROLLOUT, MOVE_SHELL_TRAP, MOVE_SKETCH, MOVE_SLEEP_TALK, MOVE_THRASH,
	MOVE_TRANSFORM, MOVE_NONE,
};

u8 instruct_revert_on_after_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
	if (HAS_VOLATILE(user, VOLATILE_INSTRUCT)) {
		CLEAR_VOLATILE(user, VOLATILE_INSTRUCT);
		CURRENT_MOVE(user) = (u16)acb->data_ptr;
		acb->data_ptr = false;
	}
	return true;
}

void instruct_on_after_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
	if (user != src) return;
	u8 target = TARGET_OF(user);
	if (LAST_MOVE(target) == MOVE_NONE) return;
	ADD_VOLATILE(target, VOLATILE_INSTRUCT);

	struct action* a = next_action(target, TARGET_OF(target), ActionMove, EventBeforeMove);
	a->target = TARGET_OF(target),
	a->reset_move_config = true;
	a->move = LAST_MOVE(target);

	acb->in_use = false;
	u8 id = add_callback(CB_ON_AFTER_MOVE, 0, 0, user, (u32)instruct_revert_on_after_move);
	CB_MASTER[id].data_ptr = CURRENT_MOVE(target);
	CURRENT_MOVE(target) = LAST_MOVE(target);
}

u8 instruct_on_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
	if (user != src) return true;
	u8 last_atk = LAST_MOVE(TARGET_OF(user));

	if (last_atk == MOVE_NONE) return false;
	if (IS_RECHARGE(last_atk) || IS_CHARGE(last_atk)) return false;
	for (u8 i = 0 ; i < (sizeof(instruct_disallow) / sizeof(u16)); i++) {
		if (last_atk == instruct_disallow[i])
			return false;
	}
	return true;
}


// After you
u8 after_you_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
	if (user != src) return true;
	// fail if target has moved already
	if (!p_bank[TARGET_OF(user)]->b_data.will_move) return false;
	// fail if target moves next anyways
	if (CURRENT_ACTION->next_action->action_bank == TARGET_OF(user)) return false;
	struct action* a = find_action(TARGET_OF(user), ActionMove);
	if (a == NULL) return false;
	u8 target_backup = a->target;
	u16 move_backup = a->move;
	u8 event_state_backup = a->event_state;
	end_action(a);
	a = next_action(TARGET_OF(user), target_backup, ActionMove, event_state_backup);
	a->move = move_backup;
	return true;
}


// Quash
u8 quash_on_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
	if (user != src) return true;
	// fail if target has moved already
	if (!p_bank[TARGET_OF(user)]->b_data.will_move) return false;

	// find and terminate target's turn
	struct action* a = find_action(TARGET_OF(user), ActionMove);
	if (a == NULL) return false;
	u8 target_backup = a->target;
	u16 move_backup = a->move;
	u8 event_state_backup = a->event_state;
	end_action(a);

	// find last action before turns end, and put opponent action before it
	a = find_action(0xFF, ActionHighPriority); // residual effects
	struct action* backup_current = CURRENT_ACTION;
	CURRENT_ACTION = a;
	a = prepend_action(TARGET_OF(user), NULL, ActionMove, event_state_backup);
	CURRENT_ACTION = backup_current;
	a->move = move_backup;
	a->target = target_backup;
	a->event_state = event_state_backup;
	return true;
}
