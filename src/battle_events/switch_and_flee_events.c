#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"
#include "battle_events/battle_events.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern void run_flee(void);
extern bool b_pkmn_has_type(u8 bank, u8 type);
extern u16 rand_range(u16 min, u16 max);


bool bank_trapped(u8 bank)
{
    if (b_pkmn_has_type(bank, TYPE_GHOST))
        return false;
    if (HAS_VOLATILE(bank, VOLATILE_TRAPPED)) {
        return true;
    }
    return false;
}


/* Event switch related */
void switch_battler(u8 switching_bank)
{
    /* TODO actual switching */
    return;
}

void move_on_switch_cb(u8 attacker)
{
    u16 move = CURRENT_MOVE(attacker);
    // add callbacks specific to field
    if (moves[move].before_switch) {
        add_callback(CB_ON_BEFORE_SWITCH, 0, 0, attacker, (u32)moves[move].before_switch);
    }
    // run callbacks
    build_execution_order(CB_ON_BEFORE_SWITCH);
    battle_master->executing = true;
    while (battle_master->executing) {
        pop_callback(attacker, move);
    }
}

void run_after_switch(u8 attacker)
{
    u16 move = CURRENT_MOVE(attacker);
    if (moves[move].on_start) {
        add_callback(CB_ON_START, 0, 0, attacker, (u32)moves[move].on_start);
    }
    // run on start callbacks
    build_execution_order(CB_ON_START);
    battle_master->executing = true;
    while (battle_master->executing) {
        pop_callback(attacker, move);
    }
}

void event_switch(struct action* current_action)
{
    move_on_switch_cb(ACTION_BANK);
    switch_battler(ACTION_BANK);
    run_after_switch(ACTION_BANK);
    end_action(current_action);
    dprintf("tried to switch\n");
}


/* Event flee related */
bool can_flee_by_random(u8 bank)
{
    // TODO: How does the flee formula change in doubles?
    p_bank[bank]->b_data.flee_count++;

    u16 reference = B_SPEED_STAT_UMOD(bank) * 128;
    reference /= B_SPEED_STAT_UMOD((bank ? 0 : 2));
    reference += (30 * p_bank[bank]->b_data.flee_count);
    reference = reference & 0xFF;

    u16 random = rand_range(0,255);
    return random < reference;
}


void event_run_flee(struct action* current_action)
{
    if (!can_flee_by_random(ACTION_BANK)) {
        // we cannot flee because we failed the dice roll
        dprintf("failed to run\n");
        enqueue_message(MOVE_NONE, ACTION_BANK, STRING_FLEE_FAILED, 0);
        end_action(current_action);
    } else {
        // flee the battle
        dprintf("successful run\n");
        enqueue_message(MOVE_NONE, ACTION_BANK, STRING_FLEE, 0);
        prepend_action(ACTION_BANK, ACTION_BANK, ActionHighPriority, EventEndBattle);
        end_action(current_action);
    }
}
