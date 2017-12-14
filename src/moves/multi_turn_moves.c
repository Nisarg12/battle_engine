#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../battle_loop/move_chain_states.h"

extern void dprintf(const char * str, ...);
extern void run_decision(void);
extern u16 rand_range(u16 min, u16 max);
extern void set_status(u8 bank, enum Effect status);

/* Thrash, Outrage, Petal Dance */
u8 thrash_on_tryhit_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if ((p_bank[user]->b_data.skip_move_select) && (p_bank[user]->b_data.move_lock_counter < 1)) {
        p_bank[user]->b_data.skip_move_select = false;
        p_bank[user]->b_data.move_lock_counter = 0;
        CLEAR_VOLATILE(user, VOLATILE_MULTI_TURN);
    } else if (p_bank[user]->b_data.skip_move_select) {
        p_bank[user]->b_data.move_lock_counter--;
    } else {
        p_bank[user]->b_data.move_lock_counter = rand_range(1, 2);
        p_bank[user]->b_data.skip_move_select = true;
        ADD_VOLATILE(user, VOLATILE_MULTI_TURN);
    }
    return 1;
}

u8 thrash_on_move_fail(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    p_bank[user]->b_data.skip_move_select = false;
    p_bank[user]->b_data.move_lock_counter = 0;
    return 1;
}

void thrash_on_after_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (p_bank[user]->b_data.skip_move_select == false) {
        if (!(HAS_VOLATILE(user, VOLATILE_CONFUSION))) {
            set_status(user, AILMENT_CONFUSION);
        }
    }
    return;
}


/* shared callbacks with Ice ball */
u8 rollout_on_before_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
	if ((p_bank[user]->b_data.skip_move_select) && (p_bank[user]->b_data.move_lock_counter < 1)) {
		p_bank[user]->b_data.skip_move_select = false;
        p_bank[user]->b_data.move_lock_counter = 0;
        CLEAR_VOLATILE(user, VOLATILE_MULTI_TURN);
	} else if (p_bank[user]->b_data.skip_move_select) {
		p_bank[user]->b_data.move_lock_counter--;
	} else {
		 p_bank[user]->b_data.move_lock_counter = 5;
        p_bank[user]->b_data.skip_move_select = true;
        ADD_VOLATILE(user, VOLATILE_MULTI_TURN);
	}
	return 1;
}

u8 rollout_on_move_fail(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    p_bank[user]->b_data.skip_move_select = false;
    p_bank[user]->b_data.move_lock_counter = 0;
    return 1;
}

void rollout_on_base_power_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
	if (HAS_VOLATILE(user, VOLATILE_DEFENSE_CURL)) {
		B_MOVE_POWER(user) *= 2;
	}
	B_MOVE_POWER(user) = (B_MOVE_POWER(user) << (5 - p_bank[user]->b_data.move_lock_counter));
}
