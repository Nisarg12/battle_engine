#include <pokeagb/pokeagb.h>
#include "battle_obj_sliding.h"
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/battle_state.h"

extern void c2_battle(void);
extern void spawn_hpboxes_wild(void);
extern void vblank_cb_merge_tbox(void);
extern void vblank_cb_merge_tbox_sliding(void);
extern void start_battle(void);
extern const struct BgConfig bg_config_data[4];
extern void pick_and_load_battle_bgs(void);
extern void battle_scene_intialize_sprites(void);
extern void battlers_move_into_scene(void);
extern void task_lightup_objs(u8);

void hblank_battle_bgsslide_win_open(void)
{
    // screen opening from middle, outwards
    if ((REG_VCOUNT > bs_env_windows->winbot) || (REG_VCOUNT < bs_env_windows->wintop)) {
        // turn off display
        REG_DISPCNT = 0x0;
    } else {
        // enable win0 and win 1
        REG_DISPCNT = 0x7B60;
    }

    // bg movement
    if (REG_VCOUNT > 79) {
        // move BG3 left
        REG_BG3HOFS = bs_env_windows->top_side;
    } else {
        // move BG3 right
        REG_BG3HOFS = bs_env_windows->bot_side;
    }

}

void hblank_battle_bgslide(void)
{
    // bg movement
    if (REG_VCOUNT > 79) {
        // move BG3 left
        REG_BG3HOFS = bs_env_windows->top_side;
    } else {
        // move BG3 right
        REG_BG3HOFS = bs_env_windows->bot_side;
    }
}


void intro_wild_battle()
{
    switch (super.multi_purpose_state_tracker) {
        case 0:
            // set callbacks and vblank interrupt
            vblank_handler_set((SuperCallback)vblank_cb_merge_tbox_sliding);
            set_callback2((SuperCallback)c2_battle);
            // put everything in the window
            REG_WININ = WININ_BUILD(WIN_BG0 | WIN_BG1 | WIN_BG2 | WIN_BG3 | WIN_OBJ | WIN_BLD,
            WIN_BG0 | WIN_BG1 | WIN_BG2 | WIN_BG3 | WIN_OBJ | WIN_BLD);
            super.multi_purpose_state_tracker++;
            break;
        case 1:
            bg_vram_setup(0, (struct BgConfig *)&bg_config_data, 4);
            // init textbox
            rbox_init_from_templates((struct TextboxTemplate*)0x8248330);
            super.multi_purpose_state_tracker++;
            break;
        case 2:
            pick_and_load_battle_bgs();
            super.multi_purpose_state_tracker++;
            break;
        case 3:
            /*
            darken sprites only
            This is a trick in which by putting BG3 in the destination, sprites which have semi-transparent
            mode activated will not be effected by the darken, since the lowest BG layer is lower in
            priority, thus the transparency will take place instead.
            */
            REG_BLDCNT = (BLDCNT_SPRITES_SRC |  BLDCNT_BG3_DST | BLDCNT_DARKEN);
            // darken factor of 7
            REG_BLDY = 0x7;
            battle_scene_intialize_sprites();
            super.multi_purpose_state_tracker++;
            break;
        case 4:
            // show bgs for background and entry image
            gpu_sync_bg_show(3);
            gpu_sync_bg_show(1);
            gpu_sync_bg_show(0);

            // enable wraparound for BG3 and BG1 (platform and entry image)
            REG_BG3CNT |= 0x2000;
            REG_BG1CNT |= 0x2000;

            // activate hblank interrupt
            interrupts_enable(2);
            hblank_handler_set((SuperCallback)hblank_battle_bgsslide_win_open);
            super.multi_purpose_state_tracker++;
            break;
        case 5:
            // set initial position for BG bottom and top half
            bs_env_windows->top_side = -260;
            bs_env_windows->bot_side = -254;

            // set initial position for center of open effect (160 / 2 = 80 = center)
            bs_env_windows->wintop = 80;
            bs_env_windows->winbot = 80;
            super.multi_purpose_state_tracker++;
            break;
        case 6:
            // there is a window in which the entry image and BG are moving
            bgid_mod_x_offset(1, 3 << 8, 1);
            bs_env_windows->wintop -= 1;
            bs_env_windows->winbot += 1;
            if (bs_env_windows->wintop < 20)
                super.multi_purpose_state_tracker++;
            break;
        case 7:
        {
            bgid_mod_x_offset(1, 4 << 8, 1);
            bgid_mod_y_offset(1, 1 << 7, 2);

            // slide in battlers for each side
            battlers_move_into_scene();
            // the Hblank is lightened since the window effect is unneeded at this point
            if (bs_env_windows->wintop <= 0) {
                hblank_handler_set((SuperCallback)hblank_battle_bgslide);
                REG_DISPCNT = 0x7B60;
            }
            // once the sliding is done, continue next state
            if (bs_env_windows->top_side > -3)
                super.multi_purpose_state_tracker++;
            break;
        }
        case 8:
        {
            interrupts_disable(2);
            hblank_handler_set(NULL);
            vblank_handler_set((SuperCallback)vblank_cb_merge_tbox);
            // light up opponents gradually and disable entry image
            u8 t_id = task_add(task_lightup_objs, 0x1);
            tasks[t_id].priv[0] = 0x7;
            gpu_sync_bg_hide(1);

            // spawn HP boxes
            spawn_hpboxes_wild();
            super.multi_purpose_state_tracker = 0;
            set_callback1((SuperCallback)start_battle);
        }
        case 10:
            break;
    };
}


void task_lightup_objs(u8 t_id)
{
    if (tasks[t_id].priv[0]) {
        // every other frame drop the darken value
        if (tasks[t_id].priv[1] % 2)
            tasks[t_id].priv[0]--;
        tasks[t_id].priv[1]++;
        REG_BLDY = tasks[t_id].priv[0];
    } else {
        // opponent side light up original palette
        REG_BLDCNT = 0;

        // player side remove blending
        objects[bs_env_windows->player_trainer_objid].final_oam.obj_mode = 0;
        objects[bs_env_windows->partner_trainer_objid].final_oam.obj_mode = 0;
        task_del(t_id);
    }
}
