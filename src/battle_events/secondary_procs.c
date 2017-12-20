#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern void set_status(u8 bank, enum Effect status);
extern void stat_boost(u8 bank, u8 stat_id, s8 amount);
extern u16 rand_range(u16 min, u16 max);

/* Boost event */
void event_move_boosts(struct action* current_action)
{
    u8 bank = current_action->action_bank;
    if (B_FAINTED(bank)) {
        CURRENT_ACTION->event_state++;
        return;
    }
    // back up cbs
    u32* old_execution_array = push_callbacks();
    u8 old_index = CB_EXEC_INDEX;
    // Stat modifying callbacks
    build_execution_order(CB_ON_BEFORE_STAT_MOD);
    battle_master->executing = true;
    while (battle_master->executing) {
        pop_callback(bank, NULL);
    }
    restore_callbacks(old_execution_array);
    CB_EXEC_INDEX = old_index;

	/* first step is to apply user boosts */
	for (u8 i = 0; i < 8; i++) {
		if (B_USER_STAT_MOD_CHANCE(bank, i) >= rand_range(0, 100)) {
			stat_boost(bank, i, B_USER_STAT_MOD_AMOUNT(bank, i));
			B_USER_STAT_MOD_CHANCE(bank, i) = 0;
            return;
		}
	}

	/* second step is to apply target boosts */
	for (u8 i = 0; i < 8; i++) {
		if (B_TARGET_STAT_MOD_CHANCE(bank, i) >= rand_range(0, 100)) {
			stat_boost(TARGET_OF(bank), i, B_TARGET_STAT_MOD_AMOUNT(bank, i));
			B_TARGET_STAT_MOD_CHANCE(bank, i) = 0;
            return;
		}
	}
    CURRENT_ACTION->event_state++;
}


/* Ailment events */
void event_move_ailments(struct action* current_action)
{
    u8 bank = current_action->action_bank;
    if (B_FAINTED(bank)) {
        CURRENT_ACTION->event_state++;
        return;
    }
	if (B_AILMENT_PROCS_CHANCE_USER(bank) >= rand_range(1, 100)) {
		// apply status user
		B_AILMENT_PROCS_CHANCE_USER(bank) = 0;
		set_status(bank, B_AILMENT_PROCS_USER(bank));
		return;
	}

	if (B_AILMENT_PROCS_CHANCE_TARGET(bank) >= rand_range(1, 100)) {
		// apply status target
		B_AILMENT_PROCS_CHANCE_TARGET(bank) = 0;
		set_status(TARGET_OF(bank), B_AILMENT_PROCS_TARGET(bank));
		return;
	}
    CURRENT_ACTION->event_state++;
}
