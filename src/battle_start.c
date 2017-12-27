#include <pokeagb/pokeagb.h>
#include "battle_data/pkmn_bank.h"
#include "battle_data/pkmn_bank_stats.h"
#include "battle_slide_in_data/battle_obj_sliding.h"
#include "battle_data/battle_state.h"
#include "battle_text/battle_textbox_gfx.h"
#include "abilities/battle_abilities.h"

extern void update_pbank(u8 bank, struct update_flags* flags);
extern void player_throwball_and_moveout_scene(struct Object*);
extern void player_hpbar_slidin_slow(u8 t_id);
extern void set_active_movement(u8 task_id);
extern void option_selection(u8 bank);
extern void pick_encounter_message(enum BattleTypes);



void start_wild_battle()
{
    switch (super.multi_purpose_state_tracker) {
        case 0:
            gpu_pal_apply((void*)stdpal_get(0), 16*rboxes[0x18 & 0x3F].pal_id, 32);
            pick_encounter_message(battle_type_flag);
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
            // wait for hp sliding animations to finish
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
            u8 active_banks[4] = {0x3F, 0x3F, 0x3F, 0x3F};
            u8 index = 0;
            for (u8 i = 0 ; i < BANK_MAX; i++) {
                if (p_bank[i]->b_data.is_active_bank) {
                    active_banks[index] = i;
                    index++;
                }
            }
            extern void sort_active_banks(u8* active_banks, u8 index);
            sort_active_banks(&active_banks[0], index);

            for (u8 i = 0; i < index; i++) {
                if (ACTIVE_BANK(active_banks[i])) {
                    u8 ability = p_bank[active_banks[i]]->b_data.ability;
                    if (abilities[ability].on_start) {
                        abilities[ability].on_start(NULL, i, NULL, NULL);
                    }
                }
            }
            super.multi_purpose_state_tracker = 0;
            option_selection(PLAYER_SINGLES_BANK);
            break;
        }
        default:
            break;
    };
}


/* Called right after sliding in effects have finished executing. */
void start_battle()
{
    switch (battle_type_flag) {
        case BATTLE_MODE_WILD:
            set_callback1(start_wild_battle);
            break;
        case BATTLE_MODE_WILD_DOUBLE:
        case BATTLE_MODE_TRAINER:
        case BATTLE_MODE_TRAINER_DOUBLE:
        case BATTLE_MODE_PARTNER:
        default:
            dprintf("FAILED to select a valid battle mode.\n");
            break;
    };
    super.multi_purpose_state_tracker = 0;
}
