#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"
#include "battle_events/battle_events.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern u16 get_damage(u8 attacker, u8 defender, u16 move);
extern bool damage_result_msg(u8 bank_index);
extern void hpbar_apply_dmg(u8 task_id);
extern void hp_anim_change(u8 bank, s16 delta);
extern void battle_loop(void);

void apply_damage()
{
    switch (super.multi_purpose_state_tracker) {
        case 0:
        {
            s16 delta = B_CURRENT_HP(CURRENT_ACTION->priv[0]) - CURRENT_ACTION->priv[1];
            delta = MAX(delta, 0);
            // HP bar damage animation
            hp_anim_change(CURRENT_ACTION->priv[0], delta);
            super.multi_purpose_state_tracker++;
            break;
        }
        case 1:
            if (task_is_running(hpbar_apply_dmg))
                return;
            super.multi_purpose_state_tracker++;
            break;
        default:
            if (B_CURRENT_HP(CURRENT_ACTION->priv[0]) < 1) {
                B_FAINTED(CURRENT_ACTION->priv[0]) = true;
            }
            end_action(CURRENT_ACTION);
            set_callback1(battle_loop);
            break;
        };
}

void init_damage(struct action* a)
{
    super.multi_purpose_state_tracker = 0;
    set_callback1(apply_damage);
    return;
}

void do_damage(u8 bank_index, u16 dmg)
{
    struct action* a = prepend_action(bank_index, NULL, ActionDamage, EventDamageAnim);
    a->priv[0] = bank_index;
    a->priv[1] = dmg;
}

void event_move_damage(struct action* current_action)
{
    u8 bank_index = current_action->action_bank;
    // get dmg
    u16 dmg = get_damage(bank_index, TARGET_OF(bank_index), CURRENT_MOVE(bank_index));
    if (dmg < 1) {
        CURRENT_ACTION->event_state++;
        return;
    }
    if (B_MOVE_EFFECTIVENESS(bank_index) == TE_IMMUNE) {
        // Todo: run move failed and delete current event
        damage_result_msg(bank_index);
        CURRENT_ACTION->event_state = EventMoveFailed;
        return;
    }

    // Log damage stats in structs
    B_MOVE_DMG(bank_index) = dmg;
    p_bank[TARGET_OF(bank_index)]->b_data.last_damage = dmg;
    p_bank[TARGET_OF(bank_index)]->b_data.last_attacked_by = bank_index;

    // HP bar damage
    do_damage(TARGET_OF(bank_index), dmg);
    damage_result_msg(bank_index);
    CURRENT_ACTION->event_state++;
}
