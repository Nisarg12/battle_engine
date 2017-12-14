#include <pokeagb/pokeagb.h>
#include "../../battle_data/pkmn_bank.h"
#include "../../battle_data/pkmn_bank_stats.h"
#include "../../battle_data/battle_state.h"
#include "../../moves/moves.h"
#include "../../battle_text/battle_pick_message.h"
#include "../move_chain_states.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern bool peek_message(void);
extern void run_decision(void);
extern void run_move(void);
extern void dprintf(const char * str, ...);
extern void give_exp(u8 fainted, u8 reciever);
extern void hpbar_apply_dmg(u8 task_id);


void obj_battler_fall_through(struct Object* obj)
{
    if ((obj->priv[0] < 64)) {
        obj->priv[1]++;
        obj->priv[0] += 8;
        obj->pos1.y += 8;

        if (obj->priv[6]) {
            // remove a tile layer from the bottom
            void* dst = (void*)((obj->final_oam.tile_num * 32) + 0x6010000);
            dst += 32 * 8 * (8 - obj->priv[1]);
            memset(dst, 0, 32 * 8 * obj->priv[1]);
        }
    } else {
        // free the hp bars too
        obj_free(&objects[obj->priv[2]]);
        obj_free(&objects[obj->priv[3]]);
        obj_free(&objects[obj->priv[4]]);
        obj_free(&objects[obj->priv[5]]);
        obj_free(obj);
    }
}

void faint(u8 bank)
{
    p_bank[bank]->b_data.fainted = true;
    if (!(SIDE_OF(bank))) {
        extern void set_active_movement(u8 tid);
        task_del(task_find_id_by_functpr(set_active_movement));
    }
    // fall through the platform animation
    objects[p_bank[bank]->objid].callback = obj_battler_fall_through;
    objects[p_bank[bank]->objid].priv[0] = 0;
    objects[p_bank[bank]->objid].priv[1] = 0;
    objects[p_bank[bank]->objid].priv[2] = p_bank[bank]->objid_hpbox[0];
    objects[p_bank[bank]->objid].priv[3] = p_bank[bank]->objid_hpbox[1];
    objects[p_bank[bank]->objid].priv[4] = p_bank[bank]->objid_hpbox[2];
    objects[p_bank[bank]->objid].priv[5] = p_bank[bank]->objid_hpbox[3];
    bool flag_mode = false;
    for (u8 i = 0; i < 4; i++) {
        if ((SIDE_OF(i) < 1) && (B_CURRENT_HP(i) > 0)) {
            flag_mode = true;
            break;
        }
    }
    objects[p_bank[bank]->objid].priv[6] = (flag_mode || (!SIDE_OF(bank)));
    enqueue_message(0, bank, STRING_FAINTED, 0);
}


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


void on_faint()
{
    while (peek_message())
        return;
    if (task_is_running(hpbar_apply_dmg))
        return;
    switch (super.multi_purpose_state_tracker) {
        case S_CHECK_BANK1_FAINT:
            if (!B_CURRENT_HP(battle_master->first_bank) || B_IS_FAINTED(battle_master->first_bank)) {
                faint(battle_master->first_bank);
                on_faint_callbacks(battle_master->first_bank);
            }
            super.multi_purpose_state_tracker = S_CHECK_BANK2_FAINT;
            break;
        case S_CHECK_BANK2_FAINT:
            if (B_IS_FAINTED(battle_master->second_bank) || (!B_CURRENT_HP(battle_master->second_bank))) {
                faint(battle_master->second_bank);
                on_faint_callbacks(battle_master->second_bank);
            }
            super.multi_purpose_state_tracker = S_RESOLVE_FAINTS;
            break;
        case S_RESOLVE_FAINTS:
            if (p_bank[PLAYER_SINGLES_BANK]->b_data.fainted) {
                // player has lost, exit battle TODO: Teleport to PC & whiteout checks. Else switch.
                super.multi_purpose_state_tracker = S_END_BATTLE;
                set_callback1(run_decision);
                return;
            } else if (p_bank[OPPONENT_SINGLES_BANK]->b_data.fainted) {
                // player wins, give exp
                give_exp(OPPONENT_SINGLES_BANK, PLAYER_SINGLES_BANK);
                super.multi_purpose_state_tracker = S_END_BATTLE;
                set_callback1(run_decision);
                return;
            } else {
                super.multi_purpose_state_tracker = battle_master->c1_prestate;
                set_callback1(battle_master->c1_after_faint_check);
                return;
            }

        break;
    };
}

bool is_fainted()
{
    if ((!B_CURRENT_HP(battle_master->first_bank)) || (!B_CURRENT_HP(battle_master->second_bank))) {
        return 1;
    } else {
        return 0;
    }
}
