#include <pokeagb/pokeagb.h>
#include "battle_data/pkmn_bank.h"
#include "battle_data/pkmn_bank_stats.h"
#include "battle_data/battle_state.h"
#include "moves/moves.h"
#include "battle_text/battle_pick_message.h"
#include "battle_events/battle_events.h"

extern void dprintf(const char * str, ...);
extern void reset_turn_bits(u8 bank);
extern void option_selection(u8 bank);
extern void sync_battler_struct(u8 bank);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern void wild_ai_pick_attack(u8 bank);
extern bool validate_move_player(u8 bank);
extern void add_bank_move_actions(void);
extern void battle_set_order(void);
extern u16 bank_interpret_selected_move(u16);


EventCallback wild_battle_events[] = {
    /* Pre-move */
    event_run_flee,
    event_switch,

    /* Run Move */
    event_before_move,
    event_config_move_hitlist,
    event_pre_move_hit, // loop through hit list and pick next target. If no target, go to aftermove
    event_move_tryhit_external, // multi hits subsequent hits start here
    event_move_damage,
    event_move_heal,
    event_move_effect,
    event_move_recoil,
    event_move_drain,
    event_move_boosts,
    event_move_ailments,
    event_multi_hit, // multi hits loop back to EventMoveHit or EventPreMoveHit
    event_after_move,
    event_fainted,


    /* Misc Events */
    event_move_failed,
    event_peek_message,
    init_damage,
    init_heal,
    event_faint,
    end_action,
    event_set_inactive,
    event_residual_effects,
    wild_battle_status_update,
    end_battle,
};


void battle_loop()
{

    CURRENT_ACTION = ACTION_HEAD;
    if (ACTION_HEAD != NULL) {
        dprintf("action: %x\n", ACTION_HEAD);
        // Don't run actions for fainted banks
        if (!ACTIVE_BANK(ACTION_HEAD->action_bank)) {
            end_action(ACTION_HEAD);
            return;
        }
        // the event is responsible for adjusting the action state of the top level action
        wild_battle_events[EVENT_STATE](ACTION_HEAD);
    } else {
        // all actions complete...
        dprintf("completed actions\n");
        for (u8 i = 0; i < BANK_MAX; i++) {
            if (!ACTIVE_BANK(i)) continue;
            p_bank[i]->b_data.first_turn = false;
            reset_turn_bits(i);
        }
        update_callbacks();
        option_selection(0);
        super.multi_purpose_state_tracker = 0;
    }
}


void validate_player_selected_move()
{
    // get and set currently selected move
    CURRENT_ACTION = ACTION_HEAD;
    u16 move_player = bank_interpret_selected_move(PLAYER_SINGLES_BANK);
    CURRENT_MOVE(PLAYER_SINGLES_BANK) = move_player;
    if (!validate_move_player(PLAYER_SINGLES_BANK)) {
        option_selection(PLAYER_SINGLES_BANK);
        return;
    }
    wild_ai_pick_attack(OPPONENT_SINGLES_BANK);
    add_bank_move_actions();
    set_callback1(battle_loop);
}

void end_battle(struct action* a)
{
    for (u8 i = 0; i < BANK_MAX; i++) {
        sync_battler_struct(i);
    }
    clear_actions();
    free(battle_master);
    free(p_bank[0]);
    exit_to_overworld_2_switch();
    set_callback1(c1_overworld);
}
