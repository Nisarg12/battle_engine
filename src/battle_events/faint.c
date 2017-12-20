#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../moves/moves.h"
#include "../battle_text/battle_pick_message.h"
#include "battle_events/battle_events.h"

extern void battle_loop(void);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern void dprintf(const char * str, ...);
extern void give_exp(u8 fainted, u8 reciever);
extern void set_active_movement(u8 tid);
extern void do_damage(u8 bank_index, u16 dmg);
extern void CpuFastSet(void* src, void* dst, u32 mode);

bool on_faint_callbacks(u8 bank)
{
    // back up cbs
    u8 old_index = CB_EXEC_INDEX;
    u32* old_execution_array = push_callbacks();
    bool old_execution_status = battle_master->executing;
    // callbacks for effectiveness of moves
    build_execution_order(CB_ON_FAINT_CHECK);
    battle_master->executing = true;
    while (battle_master->executing) {
        pop_callback(bank, CURRENT_MOVE(bank));
    }
    // restore callbacks
    restore_callbacks(old_execution_array);
    CB_EXEC_INDEX = old_index;
    battle_master->executing = old_execution_status;
    return true;
}

void obj_battler_fall_through(struct Object* obj)
{
    obj->priv[0] += 8;
    if ((obj->priv[0] < 40)) {
        obj->priv[1]++;
        obj->pos1.y += 8;

        if (obj->priv[6]) {
            // remove a tile layer from the bottom
            void* dst = (void*)((obj->final_oam.tile_num * 32) + 0x6010000);
            dst += 32 * 8 * (8 - obj->priv[1]);
            u32 set = 0;
            CpuFastSet((void*)&set, (void*)dst, CPUModeFS(32 * 8 * obj->priv[1], CPUFSSET));
        }
    } else {
        // free the hp bars too
        obj_free(&objects[obj->priv[2]]);
        obj_free(&objects[obj->priv[3]]);
        obj_free(&objects[obj->priv[4]]);
        obj_free(&objects[obj->priv[5]]);
        obj_free(obj);
        super.multi_purpose_state_tracker++;
    }
}

void do_faint()
{
    u8 bank = CURRENT_ACTION->action_bank;
    switch (super.multi_purpose_state_tracker) {
        case 0:
            // terminate the task that makes the player bob up and down
            if (!SIDE_OF(bank)) {
                task_del(task_find_id_by_functpr(set_active_movement));
                objects[p_bank[bank]->objid].priv[6] = false;
            } else {
                objects[p_bank[bank]->objid].priv[6] = ACTIVE_BANK(0) || ACTIVE_BANK(1);
            }
            // fall through the platform animation
            objects[p_bank[bank]->objid].callback = obj_battler_fall_through;
            objects[p_bank[bank]->objid].priv[0] = 0;
            objects[p_bank[bank]->objid].priv[1] = 0;
            objects[p_bank[bank]->objid].priv[2] = p_bank[bank]->objid_hpbox[0];
            objects[p_bank[bank]->objid].priv[3] = p_bank[bank]->objid_hpbox[1];
            objects[p_bank[bank]->objid].priv[4] = p_bank[bank]->objid_hpbox[2];
            objects[p_bank[bank]->objid].priv[5] = p_bank[bank]->objid_hpbox[3];


            enqueue_message(0, bank, STRING_FAINTED, 0);
            super.multi_purpose_state_tracker++;
            break;
        case 1:
            // wait for callback to terminate
            break;
        case 2:
            on_faint_callbacks(bank);
            super.multi_purpose_state_tracker++;
            break;
        case 3:
            prepend_action(bank, NULL, ActionHighPriority, EventInactive);
            end_action(CURRENT_ACTION);
            set_callback1(battle_loop);
            super.multi_purpose_state_tracker = 0;
            break;
    };
}


void event_faint(struct action* current_action)
{
    super.multi_purpose_state_tracker = 0;
    set_callback1(do_faint);
    return;
}


void event_fainted(struct action* current_action)
{
    for (u8 i = 0; i < BANK_MAX; i++) {
        if (!ACTIVE_BANK(i)) continue;
        if (B_IS_FAINTED(i)) {
            // faint this bank
            B_FAINTED(i) = true;
            do_damage(i, B_CURRENT_HP(i));
            prepend_action(i, NULL, ActionFaint, EventFaint);
        }
    }
    end_action(CURRENT_ACTION);
}

void event_set_inactive(struct action* current_action)
{
    ACTIVE_BANK(current_action->action_bank) = false;
    end_action(CURRENT_ACTION);
}
