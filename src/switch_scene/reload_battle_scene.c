#include <pokeagb/pokeagb.h>
#include "../battle_slide_in_data/battle_obj_sliding.h"
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../battle_text/battle_textbox_gfx.h"

extern void c2_battle(void);
extern void vblank_cb_merge_tbox(void);
extern const struct BgConfig bg_config_data[4];
extern void pick_and_load_battle_bgs_no_entry(const void* txtbox);
extern void CpuFastSet(void*, void*, u32);
extern void create_sprites_battle_mons_wild(void);
extern void option_selection(u8 bank);
extern void handlers_clear(void);
extern void reset_bg_settings(void);

void return_to_battle()
{
    switch(super.multi_purpose_state_tracker) {
        case 0:
            if (!pal_fade_control.active) {
                gpu_tile_bg_drop_all_sets(0);
                u32 set = 0;
                CpuFastSet((void*)&set, (void*)ADDR_VRAM, CPUModeFS(0x10000, CPUFSSET));
                vblank_handler_set((SuperCallback)vblank_cb_merge_tbox);
                set_callback2((SuperCallback)c2_battle);
                bg_vram_setup(0, (struct BgConfig *)&bg_config_data, 4);
                rbox_init_from_templates((struct TextboxTemplate*)0x8248330);
                if (battle_master->switch_main.reason == ViewPokemon)
                    pick_and_load_battle_bgs_no_entry(battle_textboxMap);
                else
                    pick_and_load_battle_bgs_no_entry(battle_textbox_action_selectMap);
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
                // hide pokemon OAMs
                if (p_bank[i]->objid < 0x3F)
                    OBJID_SHOW(p_bank[i]->objid);
                // hide HP box OAMs
                for (u8 j = 0; j < 4; j++) {
                    if (p_bank[i]->objid_hpbox[j] < 0x3F)
                        OBJID_SHOW(p_bank[i]->objid_hpbox[j]);
                }
            }
            OBJID_SHOW(p_bank[OPPONENT_SINGLES_BANK]->objid);
            super.multi_purpose_state_tracker++;
            break;
        case 2:
            option_selection(0);
            break;
    };

}
