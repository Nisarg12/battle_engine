#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/battle_state.h"
#include "../../generated/images/selection_cursor.h"
#include "../../generated/images/move_selection_cursor.h"

#define SELECTION_CURSOR_TAG 0x200

const struct OamData cursor_oam = {
                                .y = 0,
                                .affine_mode = 0,
                                .obj_mode = 0,
                                .mosaic = 0,
                                .bpp = 0,
                                .shape = 0,
                                .x = 0,
                                .matrix_num = 0,
                                .size = 0, //8x8
                                .tile_num = 0,
                                .priority = 0, // above BG layers
                                .palette_num = 0,
                                .affine_param = 0
};

const struct OamData move_cursor_oam = {
                                .y = 0,
                                .affine_mode = 0,
                                .obj_mode = 0,
                                .mosaic = 0,
                                .bpp = 0,
                                .shape = 1,
                                .x = 0,
                                .matrix_num = 0,
                                .size = 3, //64x32
                                .tile_num = 0,
                                .priority = 0, // above BG layers
                                .palette_num = 0,
                                .affine_param = 0
};

static const struct Frame (**nullframe)[] = (const struct Frame (**)[])0x8231CF0;
static const struct RotscaleFrame (**nullrsf)[] = (const struct RotscaleFrame (**)[])0x8231CFC;

void update_cursor_action_select(u8 t_id)
{
    switch (super.buttons_new_remapped & (KEY_DPAD | KEY_A)) {
        case KEY_LEFT:
            battle_master->battle_cursor.cursor_pos += (!battle_master->battle_cursor.cursor_pos) ? 3 : -1;
            break;
        case KEY_RIGHT:
            battle_master->battle_cursor.cursor_pos += (battle_master->battle_cursor.cursor_pos > 2) ? -3 : 1;
            break;
        case KEY_A:
            // pick selected move
            task_del(t_id);
            battle_master->selected_option = battle_master->battle_cursor.cursor_pos;
            obj_free(&objects[battle_master->battle_cursor.objid]);
            bs_anim_status = 0;
            break;
        default:
            // up and down do nothing
            break;
    };
    objects[battle_master->battle_cursor.objid].pos1.x = battle_master->battle_cursor.cursor_x[battle_master->battle_cursor.cursor_pos];
    objects[battle_master->battle_cursor.objid].pos1.y = battle_master->battle_cursor.cursor_y[battle_master->battle_cursor.cursor_pos];
}

u8 move_count()
{
    u8 count = 0;
    u8 i;
    for (i = 0; i < 4; i++) {
        if (battle_master->type_objid[i] != 0x3F)
            count++;
    }
    return count;
}

void update_cursor_move_select(u8 t_id)
{
    u8 cursor_old =  battle_master->battle_cursor.cursor_pos;
    switch (super.buttons_new_remapped & (KEY_DPAD | KEY_A | KEY_B)) {
        case KEY_B:
        {
            // return to previous fight menu
            task_del(t_id);
            /* obj_free(&objects[battle_master->battle_cursor.objid_mv_crsr[0]]);
            obj_free(&objects[battle_master->battle_cursor.objid_mv_crsr[1]]);
            obj_free(&objects[battle_master->battle_cursor.objid]);
            u8 i;
            for (i = 0; i < 4; i++) {
                if (battle_master->type_objid[i] != 0x3F)
                    obj_free(&objects[battle_master->type_objid[i]]);
                obj_free(&objects[battle_master->move_name_objid[i]]);
            } */
            obj_free(&objects[battle_master->battle_cursor.objid_mv_crsr[0]]);
            obj_free(&objects[battle_master->battle_cursor.objid_mv_crsr[1]]);
            obj_free(&objects[battle_master->battle_cursor.objid]);
            objects[battle_master->battle_cursor.objid].final_oam.affine_mode = 2;
            for (u8 i = 0; i < 4; i++) {
                if (battle_master->type_objid[i] != 0x3F) {
                    objects[battle_master->type_objid[i]].final_oam.affine_mode = 2;
                    objects[battle_master->move_pss_objid[i]].final_oam.affine_mode = 2;
                    objects[battle_master->move_pp_objid[i]].final_oam.affine_mode = 2;
                }
                objects[battle_master->move_name_objid[i]].final_oam.affine_mode = 2;
            }
            bs_anim_status = 0;
            super.multi_purpose_state_tracker = 0;
            return;
            break;
        }
        case KEY_A:
            // battle loop enter time
            obj_free(&objects[battle_master->battle_cursor.objid_mv_crsr[0]]);
            obj_free(&objects[battle_master->battle_cursor.objid_mv_crsr[1]]);
            obj_free(&objects[battle_master->battle_cursor.objid]);
            for (u8 i = 0; i < 4; i++) {
                if (battle_master->type_objid[i] != 0x3F) {
                    obj_free(&objects[battle_master->type_objid[i]]);
                    obj_free(&objects[battle_master->move_pss_objid[i]]);
                    obj_free(&objects[battle_master->move_pp_objid[i]]);
                }
                obj_free(&objects[battle_master->move_name_objid[i]]);
            }
            bs_anim_status = 0;
            task_del(t_id);
            super.multi_purpose_state_tracker = 8;
            break;
        case KEY_LEFT:
            if (!battle_master->battle_cursor.cursor_pos) {
                battle_master->battle_cursor.cursor_pos = 4;
            } else if (battle_master->battle_cursor.cursor_pos == 1) {
                battle_master->battle_cursor.cursor_pos = 5;
            } else {
                battle_master->battle_cursor.cursor_pos -= 2;
            }
            break;
        case KEY_RIGHT:
            if (battle_master->battle_cursor.cursor_pos == 4) {
                battle_master->battle_cursor.cursor_pos = 0;
            } else if (battle_master->battle_cursor.cursor_pos == 5) {
                battle_master->battle_cursor.cursor_pos = 1;
            } else {
                battle_master->battle_cursor.cursor_pos += 2;
            }
            break;
        case KEY_DOWN:
        case KEY_UP:
            // avoid moving up and down on mega symbol
            if (battle_master->battle_cursor.cursor_pos > 3)
                break;
            battle_master->battle_cursor.cursor_pos += (battle_master->battle_cursor.cursor_pos % 2) ? -1 : 1;
            break;
        default:
            break;
    };
    u8 available_moves = move_count();
    switch (available_moves) {
        case 1:
            if ((battle_master->battle_cursor.cursor_pos) && (battle_master->battle_cursor.cursor_pos < 4)) {
                battle_master->battle_cursor.cursor_pos = cursor_old;
            }
            break;
        case 2:
            if ((battle_master->battle_cursor.cursor_pos % 2) && (battle_master->battle_cursor.cursor_pos < 4)) {
                battle_master->battle_cursor.cursor_pos = cursor_old;
            }
            break;
        case 3:
            if (battle_master->battle_cursor.cursor_pos == 3)
                battle_master->battle_cursor.cursor_pos = cursor_old;
            break;
        default:
            break;
    };

    u8 i;
    for (i = 0; i < 4; i++) {
        u8 adjust = (battle_master->battle_cursor.cursor_pos == 1) ? 2 : battle_master->battle_cursor.cursor_pos;
        adjust = (battle_master->battle_cursor.cursor_pos == 2) ? 1 : adjust;
        if (i == adjust) {
            objects[battle_master->type_objid[i]].final_oam.affine_mode = 2;
            objects[battle_master->move_pss_objid[i]].final_oam.affine_mode = 0;
            objects[battle_master->move_pp_objid[i]].final_oam.affine_mode = 0;
        } else {
            objects[battle_master->type_objid[i]].final_oam.affine_mode = 0;
            objects[battle_master->move_pss_objid[i]].final_oam.affine_mode = 2;
            objects[battle_master->move_pp_objid[i]].final_oam.affine_mode = 2;
        }
    }
    if (battle_master->battle_cursor.cursor_pos > 3) {
        objects[battle_master->battle_cursor.objid].final_oam.affine_mode = 0;
        objects[battle_master->battle_cursor.objid_mv_crsr[0]].final_oam.affine_mode = 2;
        objects[battle_master->battle_cursor.objid_mv_crsr[1]].final_oam.affine_mode = 2;
    } else {
        objects[battle_master->battle_cursor.objid].final_oam.affine_mode = 2;
        objects[battle_master->battle_cursor.objid_mv_crsr[0]].final_oam.affine_mode = 0;
        objects[battle_master->battle_cursor.objid_mv_crsr[1]].final_oam.affine_mode = 0;
        objects[battle_master->battle_cursor.objid_mv_crsr[0]].pos1.x = battle_master->battle_cursor.cursor_x[battle_master->battle_cursor.cursor_pos];
        objects[battle_master->battle_cursor.objid_mv_crsr[0]].pos1.y = battle_master->battle_cursor.cursor_y[battle_master->battle_cursor.cursor_pos];
        objects[battle_master->battle_cursor.objid_mv_crsr[1]].pos1.x = battle_master->battle_cursor.cursor_x[battle_master->battle_cursor.cursor_pos] + 64;
        objects[battle_master->battle_cursor.objid_mv_crsr[1]].pos1.y = battle_master->battle_cursor.cursor_y[battle_master->battle_cursor.cursor_pos];
    }

}

void spawn_pointer_arrow(u8 tag)
{
    struct SpriteTiles cursor_gfx = {(void*)selection_cursorTiles, 32, SELECTION_CURSOR_TAG + tag};
    struct SpritePalette cursor_pal = {(void*)selection_cursorPal, SELECTION_CURSOR_TAG + tag};
    struct Template cursor_temp = {SELECTION_CURSOR_TAG + tag, SELECTION_CURSOR_TAG + tag, &cursor_oam,
                                    nullframe, &cursor_gfx, nullrsf, (ObjectCallback)oac_nullsub};

    // uncompressed
    gpu_tile_obj_alloc_tag_and_upload(&cursor_gfx);
    gpu_pal_obj_alloc_tag_and_apply(&cursor_pal);
    battle_master->battle_cursor.objid = template_instanciate_forward_search(
                                        &cursor_temp,
                                        battle_master->battle_cursor.cursor_x[battle_master->battle_cursor.cursor_pos],
                                        battle_master->battle_cursor.cursor_y[battle_master->battle_cursor.cursor_pos], 0
                                        );
}

void spawn_square_pointer(u8 tag)
{
    struct SpriteTiles move_cursor_gfx = {(void*)move_selection_cursorTiles, 2048, SELECTION_CURSOR_TAG + tag};
    struct SpritePalette move_cursor_pal = {(void*)move_selection_cursorPal, SELECTION_CURSOR_TAG + tag};
    struct Template move_cursor_temp = {SELECTION_CURSOR_TAG + tag, SELECTION_CURSOR_TAG + tag, &move_cursor_oam,
                                    nullframe, &move_cursor_gfx, nullrsf, (ObjectCallback)oac_nullsub};

    // uncompressed
    gpu_tile_obj_alloc_tag_and_upload(&move_cursor_gfx);
    gpu_pal_obj_alloc_tag_and_apply(&move_cursor_pal);

    battle_master->battle_cursor.objid_mv_crsr[0] = template_instanciate_forward_search(
                                        &move_cursor_temp,
                                        battle_master->battle_cursor.cursor_x[battle_master->battle_cursor.cursor_pos],
                                        battle_master->battle_cursor.cursor_y[battle_master->battle_cursor.cursor_pos], 0
                                        );
    battle_master->battle_cursor.objid_mv_crsr[1] = template_instanciate_forward_search(
                                                &move_cursor_temp,
                                                battle_master->battle_cursor.cursor_x[battle_master->battle_cursor.cursor_pos] + 64,
                                                battle_master->battle_cursor.cursor_y[battle_master->battle_cursor.cursor_pos], 0
                                                );
    objects[battle_master->battle_cursor.objid_mv_crsr[1]].final_oam.tile_num += 32;
}

void show_move_data(void)
{

    battle_master->battle_cursor.cursor_x[0] = 39;
    battle_master->battle_cursor.cursor_x[1] = 39;
    battle_master->battle_cursor.cursor_x[2] = 141;
    battle_master->battle_cursor.cursor_x[3] = 141;
    battle_master->battle_cursor.cursor_x[4] = 216;
    battle_master->battle_cursor.cursor_x[5] = 216;

    battle_master->battle_cursor.cursor_y[0] = 132;
    battle_master->battle_cursor.cursor_y[1] = 151;
    battle_master->battle_cursor.cursor_y[2] = 132;
    battle_master->battle_cursor.cursor_y[3] = 151;
    battle_master->battle_cursor.cursor_y[4] = 136;
    battle_master->battle_cursor.cursor_y[5] = 136;
    objects[battle_master->battle_cursor.objid].final_oam.affine_mode = 0;

    u8 i;
    for (i = 0; i < 4; i++) {
        if (battle_master->type_objid[i] != 0x3F)
            objects[battle_master->type_objid[i]].final_oam.affine_mode = 0;
        objects[battle_master->move_name_objid[i]].final_oam.affine_mode = 0;
    }

    spawn_square_pointer(5);
    spawn_pointer_arrow(10);
    objects[battle_master->battle_cursor.objid].pos1.x = battle_master->battle_cursor.cursor_x[4];
    objects[battle_master->battle_cursor.objid].pos1.y = battle_master->battle_cursor.cursor_y[4];
    objects[battle_master->battle_cursor.objid].final_oam.affine_mode = 2; // hide
}

void init_selection_cursor(u8 mode, u8 bank)
{
    battle_master->battle_cursor.cursor_pos = 0;
    if (mode) {
        // mode is action selection
        battle_master->battle_cursor.cursor_x[0] = 15;
        battle_master->battle_cursor.cursor_x[1] = 75;
        battle_master->battle_cursor.cursor_x[2] = 148;
        battle_master->battle_cursor.cursor_x[3] = 198;

        battle_master->battle_cursor.cursor_y[0] = 136;
        battle_master->battle_cursor.cursor_y[1] = 136;
        battle_master->battle_cursor.cursor_y[2] = 136;
        battle_master->battle_cursor.cursor_y[3] = 136;
        task_add(update_cursor_action_select, 1);
        spawn_pointer_arrow(0);

    } else {
        // mode is move selection
        battle_master->battle_cursor.cursor_x[0] = 39;
        battle_master->battle_cursor.cursor_x[1] = 39;
        battle_master->battle_cursor.cursor_x[2] = 141;
        battle_master->battle_cursor.cursor_x[3] = 141;
        battle_master->battle_cursor.cursor_x[4] = 216;
        battle_master->battle_cursor.cursor_x[5] = 216;

        battle_master->battle_cursor.cursor_y[0] = 132;
        battle_master->battle_cursor.cursor_y[1] = 151;
        battle_master->battle_cursor.cursor_y[2] = 132;
        battle_master->battle_cursor.cursor_y[3] = 151;
        battle_master->battle_cursor.cursor_y[4] = 136;
        battle_master->battle_cursor.cursor_y[5] = 136;
        u8 t_id = task_add(update_cursor_move_select, 1);
        tasks[t_id].priv[0] = bank;

        spawn_square_pointer(5);
        spawn_pointer_arrow(10);
        objects[battle_master->battle_cursor.objid].pos1.x = battle_master->battle_cursor.cursor_x[4];
        objects[battle_master->battle_cursor.objid].pos1.y = battle_master->battle_cursor.cursor_y[4];
        objects[battle_master->battle_cursor.objid].final_oam.affine_mode = 2; // hide
        battle_master->fight_menu_content_spawned = 1;
    }
    return;
}


void free_unused_objs()
{
    if (battle_master->battle_cursor.objid_mv_crsr[0] != 0x3F) {
        obj_free(&objects[battle_master->battle_cursor.objid_mv_crsr[0]]);
        obj_free(&objects[battle_master->battle_cursor.objid_mv_crsr[1]]);
    }
    for (u8 i = 0; i < 4; i++) {
        if (battle_master->type_objid[i] != 0x3F) {
            obj_free(&objects[battle_master->type_objid[i]]);
            obj_free(&objects[battle_master->move_pss_objid[i]]);
            obj_free(&objects[battle_master->move_pp_objid[i]]);
        }
        if (battle_master->move_name_objid[i] != 0x3F)
            obj_free(&objects[battle_master->move_name_objid[i]]);
    }
}
