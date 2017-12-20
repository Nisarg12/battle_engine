#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/battle_state.h"
#include "battle_pick_message.h"
#include "../battle_events/battle_events.h"

extern void dprintf(const char * str, ...);
extern void battle_loop(void);
extern void pick_battle_message(u16 move_id, u8 user_bank, enum BattleTypes battle_type, enum battle_string_ids id, u16 move_effect_id);


struct action* enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect)
{
    struct action* a = prepend_action(ACTION_BANK, ACTION_BANK, ActionText, EventPeekMessage);
    a->action_bank = bank;
    a->move = move;
    a->priv[0] = id;
    a->priv[1] = effect;
    return a;
}


void event_peek_message(struct action* current_action)
{
    extern void play_bmessage(void);
    super.callback2_backup = super.callback1;
    super.callback1 = play_bmessage;
    battle_master->execution_index = super.multi_purpose_state_tracker;
    super.multi_purpose_state_tracker = 0;
}


void play_bmessage()
{
    switch(super.multi_purpose_state_tracker) {
        case 0:
        {
            struct action* a = CURRENT_ACTION;
            pick_battle_message(a->move, a->action_bank, battle_type_flag, a->priv[0], a->priv[1]);
            battle_show_message((u8*)string_buffer, 0x18);
            super.multi_purpose_state_tracker++;
        }
            break;
        case 1:
            if (!dialogid_was_acknowledged(0x18)) {
                super.multi_purpose_state_tracker++;
            }
            break;
        case 25:
        {
            end_action(CURRENT_ACTION);
            super.callback1 = super.callback2_backup;
            super.multi_purpose_state_tracker = battle_master->execution_index;
            battle_master->execution_index = 0;
            break;
        }
        default:
            super.multi_purpose_state_tracker++;
            break;
    };
}
