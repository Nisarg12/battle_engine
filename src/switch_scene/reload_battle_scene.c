#include <pokeagb/pokeagb.h>
#include "../battle_slide_in_data/battle_obj_sliding.h"
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../battle_text/battle_textbox_gfx.h"

extern void battle_loop(void);
extern void c2_battle(void);
extern void vblank_cb_merge_tbox(void);
extern const struct BgConfig bg_config_data[4];
extern void pick_and_load_battle_bgs_no_entry(const void* txtbox);
extern void CpuFastSet(void*, void*, u32);
extern void create_sprites_battle_mons_wild(void);
extern void option_selection(u8 bank);
extern void handlers_clear(void);
extern void reset_bg_settings(void);
extern void validate_player_selected_move(void);
extern void set_active_movement(u8 tid);

void return_to_battle()
{
    switch(super.multi_purpose_state_tracker) {
        case 0:
            if (!pal_fade_control.active) {
                gpu_tile_bg_drop_all_sets(0);
                // u32 set = 0;
                // CpuFastSet((void*)&set, (void*)ADDR_VRAM, CPUModeFS(0x10000, CPUFSSET));
                vblank_handler_set((SuperCallback)vblank_cb_merge_tbox);
                set_callback2((SuperCallback)c2_battle);
                bg_vram_setup(0, (struct BgConfig *)&bg_config_data, 4);
                rbox_init_from_templates((struct TextboxTemplate*)0x8248330);
                if (battle_master->switch_main.reason == ViewPokemon)
                    pick_and_load_battle_bgs_no_entry(battle_textbox_action_selectMap);
                else
                    pick_and_load_battle_bgs_no_entry(battle_textboxMap);
                super.multi_purpose_state_tracker++;
            }
            break;
        case 1:
            fade_screen(0xFFFFFFFF, 0, 16, 0, 0x0000);
            // show bgs for background and entry image
            gpu_sync_bg_show(3);
            gpu_sync_bg_show(0);
            // show oams
            for (u8 i = 0; i < BANK_MAX; i++) {
                // show pokemon OAMs
                if (p_bank[i]->objid < 0x3F) {
                    OBJID_SHOW(p_bank[i]->objid);
                    objects[p_bank[i]->objid].final_oam.h_flip = false;
                    objects[p_bank[i]->objid].final_oam.v_flip = false;
                }
                // show HP box OAMs
                for (u8 j = 0; j < 4; j++) {
                    if (p_bank[i]->objid_hpbox[j] < 0x3F)
                        OBJID_SHOW(p_bank[i]->objid_hpbox[j]);
                }
            }
            super.multi_purpose_state_tracker++;
            break;
        case 2:
            if (pal_fade_control.active) return;
            super.multi_purpose_state_tracker++;
        case 3:
            switch (battle_master->switch_main.reason) {
                case ViewPokemon:
                    tasks[task_add(set_active_movement, 1)].priv[0] = battle_master->option_selecting_bank;
                    option_selection(0);
                    return;
                case NormalSwitch:
                    tasks[task_add(set_active_movement, 1)].priv[0] = battle_master->option_selecting_bank;
                    p_bank[PLAYER_SINGLES_BANK]->b_data.is_switching = true;
                    p_bank[PLAYER_SINGLES_BANK]->this_pkmn = &party_player[battle_master->switch_main.position];
                    super.multi_purpose_state_tracker = 0;
                    validate_player_selected_move();
                    end_action(CURRENT_ACTION);
                    set_callback1(battle_loop);
                    return;
                case ForcedSwitch:
                    p_bank[PLAYER_SINGLES_BANK]->this_pkmn = &party_player[battle_master->switch_main.position];
                    super.multi_purpose_state_tracker = 0;
                    p_bank[PLAYER_SINGLES_BANK]->b_data.is_switching = false;
                    set_callback1(battle_loop);
                    return;
                case PokemonFainted:
                    p_bank[PLAYER_SINGLES_BANK]->this_pkmn = &party_player[battle_master->switch_main.position];
                    super.multi_purpose_state_tracker = 0;
                    CURRENT_ACTION->event_state++;
                    set_callback1(battle_loop);
                    return;
            };
            dprintf("INVALID SWITCH REASON WAS GIVEN\n");
            super.multi_purpose_state_tracker++;
            break;
        case 4:
            break;
    };

}
