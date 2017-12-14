#include <pokeagb/pokeagb.h>
#include "battle_data/pkmn_bank.h"
#include "battle_data/pkmn_bank_stats.h"
#include "battle_slide_in_data/battle_obj_sliding.h"
#include "battle_data/battle_state.h"
#include "battle_text/battle_textbox_gfx.h"

extern void pick_encounter_message(enum BattleFlag);
extern void setup(void);
extern void battle_slidein(void);
extern void player_throwball_and_moveout_scene(struct Object*);
extern void player_hpbar_slidin_slow(u8 t_id);
extern void set_active_movement(u8 task_id);
extern void option_selection(void);
extern void dprintf(const char * str, ...);
extern void residual_cbs_init(void);
extern void test_battle_data(void);
extern void update_pbank(u8 bank, struct update_flags* flags);

void init_battle_elements()
{
    // allocate battle specific resources
    setup();
    void* p_bank_data = malloc_and_clear(sizeof(struct pkmn_bank) * 4);
    for(u8 i = 0; i < BANK_MAX; i++) {
        p_bank[i] = (struct pkmn_bank*)(p_bank_data + (sizeof(struct pkmn_bank) * i));
        for(u8 j = 0; j < 4; j++) {
            p_bank[i]->objid_hpbox[j] = 0x3F;
        }

    }
    //p_bank = (struct pkmn_bank(*)[4])malloc_and_clear(sizeof(struct pkmn_bank) * 4);
    bs_env_windows = (struct bs_elements_positions*)malloc_and_clear(sizeof(struct bs_elements_positions));
    battle_master = (struct battle_main*)malloc_and_clear(sizeof(struct battle_main));
    for(u8 i = 0; i < 10; ++i) {
        battle_master->switch_main.type_objid[i] = 0x3F;
    }

    help_system_disable__sp198();
    super.multi_purpose_state_tracker = 0;
    set_callback1((SuperCallback)battle_slidein);
    battle_type_flags = BATTLE_FLAG_WILD;
    p_bank[PLAYER_SINGLES_BANK]->b_data.is_active_bank = true;
    p_bank[OPPONENT_SINGLES_BANK]->b_data.is_active_bank = true;
    test_battle_data();
}


/* Called right after sliding in effects have finished executing. */
void init_battle()
{
    switch (super.multi_purpose_state_tracker) {
        case 0:
            gpu_pal_apply((void*)stdpal_get(0), 16*rboxes[0x18 & 0x3F].pal_id, 32);
            pick_encounter_message(battle_type_flags);
            battle_show_message((u8*)string_buffer, 0x18);
            super.multi_purpose_state_tracker++;
            break;
        case 1:
            if (!dialogid_was_acknowledged(0x18 & 0x3F)) {
                objects[bs_env_windows->player_trainer_objid].anim_number++;
                objects[bs_env_windows->player_trainer_objid].callback = player_throwball_and_moveout_scene;
                objects[bs_env_windows->player_trainer_objid].priv[2] = PLAYER_SINGLES_BANK;
                bs_anim_status = 1;
                super.multi_purpose_state_tracker++;
            }
            break;
        case 2:
            if (bs_anim_status)
                return;
            // create and slide into place HPbox
            task_add(player_hpbar_slidin_slow, 1);
            bs_anim_status = 1;
            super.multi_purpose_state_tracker++;
            break;
        case 3:
        {
            // wait for sliding animations to finish
            if (bs_anim_status)
                return;

            // up and down movement of the active moving Pokemon
            u8 t_id = task_add(set_active_movement, 1);
            tasks[t_id].priv[0] = PLAYER_SINGLES_BANK;

            // build p_bank data once animation is finished
            struct update_flags* flags = (struct update_flags*)malloc_and_clear(sizeof(struct update_flags));
            flags->pass_status = true;
            flags->pass_stats = false;
            flags->pass_atk_history = false;
            flags->pass_disables = false;
            update_pbank(PLAYER_SINGLES_BANK, flags);
            update_pbank(OPPONENT_SINGLES_BANK, flags);
            free(flags);
            free(bs_env_windows);
            super.multi_purpose_state_tracker++;
        }
        case 4:
        {
            // initialization finished, set c1 option selection
            super.multi_purpose_state_tracker = 0;
            set_callback1(option_selection);
            break;
        }
        default:
            break;
    };
}
