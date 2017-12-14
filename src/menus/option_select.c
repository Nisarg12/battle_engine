#include <pokeagb/pokeagb.h>
#include "battle_data/pkmn_bank.h"
#include "battle_data/pkmn_bank_stats.h"
#include "battle_data/battle_state.h"
#include "battle_text/battle_textbox_gfx.h"

extern void battle_loop(void);
extern void vblank_cb_no_merge(void);
extern void vblank_cb_merge_move_select(void);
extern void load_icons_moves(u8 bank);
extern void update_cursor_move_select(u8 task_id);
extern void show_move_data(void);
extern void init_selection_cursor(u8 mode, u8 bank);
extern void dprintf(const char * str, ...);

/* Fight menu and move menu selection. Preperation to go into battle loop*/
void option_selection()
{
    switch (super.multi_purpose_state_tracker) {
        case 0:
        {
            // if move is charging up, then option selection is skipped
            if ((HAS_VOLATILE(PLAYER_SINGLES_BANK, VOLATILE_CHARGING))||
                (p_bank[PLAYER_SINGLES_BANK]->b_data.skip_move_select) ||
                (HAS_VOLATILE(PLAYER_SINGLES_BANK, VOLATILE_RECHARGING))) {
                set_callback1(battle_loop);
                return;
            }
            // initialize fight menu selection
            vblank_handler_set(vblank_cb_no_merge);
            void* map_base = (void *)0x600F800;
            memcpy(map_base, battle_textbox_action_selectMap, sizeof(battle_textbox_action_selectMap));
            init_selection_cursor(1, 0);

            // next state
            bs_anim_status = 1;
            super.multi_purpose_state_tracker++;
            break;
        }
        case 1:
            // wait for input selection from fight menu
            if (bs_anim_status)
                return;
            // interpret selection
            switch (battle_master->selected_option) {
                case OPTION_FIGHT:
                    super.multi_purpose_state_tracker = (battle_master->fight_menu_content_spawned) ? 7 : 2;
                    break;
                case OPTION_POKEMON:
                    super.multi_purpose_state_tracker = 3;
                    break;
                case OPTION_BAG:
                    super.multi_purpose_state_tracker = 4;
                    break;
                case OPTION_RUN:
                    super.multi_purpose_state_tracker = 5;
                    break;

            };
            break;
        case 2:
            /* FIGHT selected from fight menu */

            // update tilemap
            vblank_handler_set(vblank_cb_merge_move_select);
            void* map_base = (void *)0x600F800;
            memcpy(map_base, battle_textbox_move_selectMap, sizeof(battle_textbox_action_selectMap));

            // init cursor
            init_selection_cursor(0, 0);

            // init move types
            load_icons_moves(PLAYER_SINGLES_BANK);
            // set into pause state
            bs_anim_status = 1;
            super.multi_purpose_state_tracker = 6;
            break;
        case 3:
            // POKEMON selection from fight menu
            fade_screen(0xFFFFFFFF, 0,0,16, 0x0000);
            extern void switch_scene_main(void);
            super.multi_purpose_state_tracker = 0;
            set_callback1(switch_scene_main);
            break;
        case 4:
            // BAG selected from fight menu
            break;
        case 5:
        {
            // RUN selected from fight menu
            extern void free_unused_objs(void);
            free_unused_objs();
            p_bank[PLAYER_SINGLES_BANK]->b_data.is_running = true;
            super.multi_purpose_state_tracker = 8;
        }
            break;
        case 6:
            break;
        case 7:
            {
            vblank_handler_set(vblank_cb_merge_move_select);
            void* map_base = (void *)0x600F800;
            memcpy(map_base, battle_textbox_move_selectMap, sizeof(battle_textbox_action_selectMap));
            show_move_data();
            tasks[task_add(update_cursor_move_select, 1)].priv[0] = 0;
            bs_anim_status = 1;
            super.multi_purpose_state_tracker = 6;
            break;
            }
        case 8:
        {
            set_callback1(battle_loop);
            void* map_base = (void *)0x600F800;
            memcpy(map_base, battle_textboxMap, sizeof(battle_textboxMap));
            super.multi_purpose_state_tracker = 0;
            break;
        }
    };
}
