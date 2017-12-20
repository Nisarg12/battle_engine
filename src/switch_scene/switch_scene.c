#include <pokeagb/pokeagb.h>
#include "battle_data/battle_state.h"
#include "battle_data/pkmn_bank.h"
#include "battle_data/pkmn_bank_stats.h"
#include "battle_text/battle_textbox_gfx.h"
#include "switch_scene.h"

// images
#include "../generated/images/switch/slider_bot.h"
#include "../generated/images/switch/slider_mid.h"
#include "../generated/images/switch/slider_top.h"
#include "../generated/images/PSS_icons.h"
#include "../generated/images/battle_terrains/grass/grass_bg.h"
#include "../generated/images/switch/switch_bg.h"
#include "../generated/images/type_icons.h"

extern u8 get_ability(struct Pokemon *p);
extern const struct BgConfig bg_config_data[4];
extern void CpuFastSet(void* src, void* dst, u32 mode);
extern void return_to_battle(void);
extern void reset_boxes(void);

const struct Frame (**nullframe)[] = (const struct Frame (**)[])0x8231CF0;
const struct RotscaleFrame (**nullrsf)[] = (const struct RotscaleFrame (**)[])0x8231CFC;
const struct OamData icon_oam = {
    .y = 0,
    .affine_mode = 1,
    .obj_mode = 0,
    .mosaic = 0,
    .bpp = 0,
    .shape = 0,
    .x = 0,
    .matrix_num = 0,
    .size = 2, // 32x32 square
    .tile_num = 0,
    .priority = 0, /*above the rest*/
    .palette_num = 0,
    .affine_param = 0,
};

// the difference is that the slider doesn't have affine mode turned on
const struct OamData slider_oam = {
    .y = 0,
    .affine_mode = 0,
    .obj_mode = 0,
    .mosaic = 0,
    .bpp = 0,
    .shape = 0,
    .x = 0,
    .matrix_num = 0,
    .size = 2, // 32x32 square
    .tile_num = 0,
    .priority = 0, /*above the rest*/
    .palette_num = 0,
    .affine_param = 0,
};

static const struct RotscaleFrame switch_scale_down[] = {
    {-0xA0, 0xA0, 0, 0, 0},
    {0x7FFE, 0, 0, 0, 0},
};

static const struct RotscaleFrame switch_scale_full[] = {
    {-0x100, 0x100, 0, 0, 0},
    {0x7FFE, 0, 0, 0, 0},
};


static const struct RotscaleFrame *switch_scale_table[] = {
    switch_scale_down,
};

static const struct RotscaleFrame *switch_scale_table_full[] = {
    switch_scale_full,
};


void switch_fetch_all_data()
{
    /* Log data into struct for every Pokemon in player's party */
    struct switch_data* sd = (struct switch_data*)malloc_and_clear(sizeof(struct switch_data));
    SWM_LOG = sd;
    sd->list_count = count_pokemon();
    for (u32 i = 0; i < sd->list_count; i++) {
        // log current HP and misc data
        sd->s_pkmn_data[i].current_hp = pokemon_getattr(&party_player[i], REQUEST_CURRENT_HP, NULL);
        sd->s_pkmn_data[i].total_hp = pokemon_getattr(&party_player[i], REQUEST_TOTAL_HP, NULL);
        sd->s_pkmn_data[i].ability = get_ability(&party_player[i]);
        sd->s_pkmn_data[i].item = pokemon_getattr(&party_player[i], REQUEST_HELD_ITEM, NULL);
        sd->s_pkmn_data[i].species = pokemon_getattr(&party_player[i], REQUEST_SPECIES, NULL);
        sd->s_pkmn_data[i].PID = pokemon_getattr(&party_player[i], REQUEST_PID, NULL);
        pokemon_getattr(&party_player[i], REQUEST_NICK, (pchar*)&sd->s_pkmn_data[i].nickname[0]);
        for (u32 j = 0; j < 5; j++) {
            // in get_attr, speed comes after Def. Here we want it as the last member
            if (j == 2) {
                sd->s_pkmn_data[i].stats[4] = pokemon_getattr(&party_player[i], REQUEST_ATK + j, NULL);
                sd->s_pkmn_data[i].nature_boosted[4] = STAT_COLOR(j, &party_player[i]);
            } else if (j == 4) {
                sd->s_pkmn_data[i].stats[2] = pokemon_getattr(&party_player[i], REQUEST_ATK + j, NULL);
                sd->s_pkmn_data[i].nature_boosted[2] = STAT_COLOR(j, &party_player[i]);
            } else {
                sd->s_pkmn_data[i].stats[j] = pokemon_getattr(&party_player[i], REQUEST_ATK + j, NULL);
                sd->s_pkmn_data[i].nature_boosted[j] = STAT_COLOR(j, &party_player[i]);
            }
        }
        // log moves and remaining PP counts
        for (u32 j = 0; j < 4; j++) {
            sd->s_pkmn_data[i].move[j] = pokemon_getattr(&party_player[i], REQUEST_MOVE1 + j, NULL);
            sd->s_pkmn_data[i].pp[j] = pokemon_getattr(&party_player[i], REQUEST_PP1 + j, NULL);
        }
    }
}


void switch_setup()
{
    vblank_handler_set(vblank_cb_no_merge);
    set_callback2(c2_switch_menu);

    bgid_mod_x_offset(0, 0, 0);
    bgid_mod_y_offset(0, 0, 0);
    bgid_mod_x_offset(1, 0, 0);
    bgid_mod_y_offset(1, 0, 0);
    gpu_tile_bg_drop_all_sets(0);
    bg_vram_setup(0, (struct BgConfig *)&bg_config_data, 4);
    u32 set = 0;
    CpuFastSet((void*)&set, (void*)ADDR_VRAM, CPUModeFS(0x10000, CPUFSSET));

    /* Hide all the objects on screen */
    for (u8 i = 0; i < BANK_MAX; i++) {
        //  pokemon OAMs
        if (p_bank[i]->objid < 0x3F)
            OBJID_HIDE(p_bank[0]->objid);
        // hide HP box OAMs
        for (u8 j = 0; j < 4; j++) {
            if (p_bank[i]->objid_hpbox[j] < 0x3F)
                OBJID_HIDE(p_bank[i]->objid_hpbox[j]);
        }
    }
    gpu_sync_bg_hide(1);
    gpu_sync_bg_hide(0);
}

void switch_load_background()
{
    /* load menu */
    void *sw_bgbackbuffer = malloc(0x800);
    battle_master->switch_main.back_buffer = sw_bgbackbuffer;
    gpu_pal_apply_compressed((void *)switch_bgPal, 0, 32);
    gpu_pal_apply((void *)(&switch_text_pal), 15 * 16, 32);
    LZ77UnCompWram((void *)switch_bgMap, (void *)sw_bgbackbuffer);
    lz77UnCompVram((void *)switch_bgTiles, (void *)0x06008000);
    bgid_set_tilemap(1, sw_bgbackbuffer);
    bgid_mark_for_sync(1);
    bgid_mark_for_sync(0);
}

void switch_type_update_icon(u8 objid, enum MoveTypes type)
{
    void *vram = (void *)((0x06010000) + objects[objid].final_oam.tile_num * 32);
    CpuFastSet((void*)((type * 256) + type_iconsTiles), vram, CPUModeFS(256, CPUFSCPY));
}

void switch_category_update_icon(u8 objid, u8 category)
{
    void *vram = (void *)((0x06010000) + objects[objid].final_oam.tile_num * 32);
    CpuFastSet((void*)((category * 128) + PSS_iconsTiles), vram, CPUModeFS(128, CPUFSCPY));
}

void switch_type_icon_load(u8 type, s16 x, s16 y, u8 id)
{
    if (battle_master->switch_main.type_objid[id] != 0x3F) {
        switch_type_update_icon(battle_master->switch_main.type_objid[id], type);
    } else {
        battle_master->switch_main.type_objid[id] = load_dmg_type_icon(type, x, y, id + 4);
    }
    OBJID_SHOW(battle_master->switch_main.type_objid[id]);
}

void switch_cat_icon_load(u8 category, s16 x, s16 y, u8 id)
{
    u8 array_idx = id + 6;
    if (battle_master->switch_main.type_objid[array_idx] != 0x3F) {
        switch_category_update_icon(battle_master->switch_main.type_objid[array_idx], category);
    } else {
        battle_master->switch_main.type_objid[array_idx] = load_dmg_category_icon(category, x, y, id + 4);
    }
    OBJID_SHOW(battle_master->switch_main.type_objid[array_idx]);
}

void icon_frame_change(struct Object* obj)
{
    if (obj->priv[0] == 20) {
        obj->final_oam.tile_num += 16;
        obj->priv[0]++;
    } else if (obj->priv[0] == 40) {
        obj->final_oam.tile_num -= 16;
        obj->priv[0] = 0;
    }
    obj->priv[0]++;
}


void switch_load_pokemon_icons() {
    /* allocate all pallettes */
    for (u32 i = 0; i < 3; i++) {
        struct SpritePalette pal = {&pokeicon_pals[i], ICON_PAL_TAG + i};
        gpu_pal_obj_alloc_tag_and_apply(&pal);
    }

    for (u32 i = 0; i < SWM_LOG->list_count; i++) {
        u16 species = SWM_LOG->s_pkmn_data[i].species;
        u32 pid = SWM_LOG->s_pkmn_data[i].PID;
        if (species > 0) {
            void *icon_gfx = load_party_icon_tiles_with_form(species, pid, false);
            u16 gfx_tag = ICON_GFX_TAG + i;
            u16 pal_tag = ICON_PAL_TAG + pokeicon_pal_indices[species];
            struct SpriteTiles icon_tiles = {icon_gfx, 4 * 8 * 32, gfx_tag};
            gpu_tile_obj_alloc_tag_and_upload(&icon_tiles);
            struct Template icon_template = {
                                            .tiles_tag = gfx_tag,
                                            .pal_tag = pal_tag,
                                            .oam = &icon_oam,
                                            .animation = nullframe,
                                            .graphics = &icon_tiles,
                                            .rotscale = ((i != 0) ? switch_scale_table : switch_scale_table_full),
                                            .callback = ((i != 0) ? oac_nullsub : icon_frame_change),
                                            };
            if (i == 0) {
                battle_master->switch_main.icon_objid[i] = template_instanciate_forward_search(&icon_template, 16, 10, 0);
            } else if (i == 5) {
                battle_master->switch_main.icon_objid[i] = template_instanciate_forward_search(&icon_template, 16, 10 + 23 * i, 0);
            } else {
                battle_master->switch_main.icon_objid[i] = template_instanciate_forward_search(&icon_template, 16, 10 + 24 * i, 0);
            }
        }
    }
}


void switch_load_scroll_box()
{
    struct SpritePalette scroll_pal = {(void *)slider_topPal, SLIDER_PAL_TAG};
    struct SpriteTiles top_gfx = {(void *)slider_topTiles, 512, SLIDER_GFX_TAG};
    struct SpriteTiles mid_gfx = {(void *)slider_midTiles, 512, SLIDER_GFX_TAG + 1};
    struct SpriteTiles bot_gfx = {(void *)slider_botTiles, 512, SLIDER_GFX_TAG + 2};

    struct Template top_template = {SLIDER_GFX_TAG, SLIDER_PAL_TAG, &slider_oam, nullframe, &top_gfx, nullrsf, oac_nullsub};
    struct Template mid_template = {SLIDER_GFX_TAG + 1, SLIDER_PAL_TAG, &slider_oam, nullframe, &mid_gfx, nullrsf, oac_nullsub};
    struct Template bot_template = {SLIDER_GFX_TAG + 2, SLIDER_PAL_TAG, &slider_oam, nullframe, &bot_gfx, nullrsf, oac_nullsub};
    gpu_pal_decompress_alloc_tag_and_upload(&scroll_pal);
    gpu_tile_obj_decompress_alloc_tag_and_upload(&top_gfx);
    gpu_tile_obj_decompress_alloc_tag_and_upload(&mid_gfx);
    gpu_tile_obj_decompress_alloc_tag_and_upload(&bot_gfx);
    battle_master->switch_main.slider_objid[0] = template_instanciate_forward_search(&top_template, 17, 17, 1);
    battle_master->switch_main.slider_objid[1] = template_instanciate_forward_search(&mid_template, 17, 40, 1);
    battle_master->switch_main.slider_objid[2] = template_instanciate_forward_search(&bot_template, 17, 128, 1);
    objects[battle_master->switch_main.slider_objid[0]].final_oam.affine_mode = 0;
    objects[battle_master->switch_main.slider_objid[1]].final_oam.affine_mode = 0;
    objects[battle_master->switch_main.slider_objid[2]].final_oam.affine_mode = 0;
    OBJID_HIDE(battle_master->switch_main.slider_objid[1]);
    OBJID_HIDE(battle_master->switch_main.slider_objid[2]);
}

void str_int_padding(u8 number, u8 digits)
{
    /* 3 digit max support */
    u8 space = digits;
    u8 prefix[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    fmt_int_10(string_buffer, number, 0, 4);

    // faster than div or log...
    if (number < 10) {
        space -= 1;
    } else if (number < 100) {
        space -= 2;
    } else if (number >= 100) {
        space -= 3;
    }
    for (u32 i = 0; i < space; i++) {
        prefix[i] = 0;
    }
    pstrcat((pchar*)prefix, string_buffer);
    pstrcpy(string_buffer, prefix);
}

void switch_load_pokemon_data(u8 index)
{
    for (u32 i = SWB_ABILITY; i <= SWB_NAME; ++i) {
        rboxid_clear_pixels(i, 0);
    }

    rboxid_print(SWB_NAME, 0, 0, 4, &switch_color_bg, 0, &SWM_LOG->s_pkmn_data[index].nickname[0]);

    /* print the stats */
    str_int_padding(SWM_LOG->s_pkmn_data[index].stats[0], 3);
    rboxid_print(SWB_ATK, 0, 1, 7, SWM_LOG->s_pkmn_data[index].nature_boosted[0], 0, &string_buffer[0]);

    str_int_padding(SWM_LOG->s_pkmn_data[index].stats[1], 3);
    rboxid_print(SWB_DEF, 0, 2, 7, SWM_LOG->s_pkmn_data[index].nature_boosted[1], 0, &string_buffer[0]);

    str_int_padding(SWM_LOG->s_pkmn_data[index].stats[2], 3);
    rboxid_print(SWB_SPA, 0, 3, 7, SWM_LOG->s_pkmn_data[index].nature_boosted[2], 0, &string_buffer[0]);

    str_int_padding(SWM_LOG->s_pkmn_data[index].stats[3], 3);
    rboxid_print(SWB_SPD, 0, 4, 7, SWM_LOG->s_pkmn_data[index].nature_boosted[3], 0, &string_buffer[0]);

    str_int_padding(SWM_LOG->s_pkmn_data[index].stats[4], 3);
    rboxid_print(SWB_SPE, 0, 5, 7, SWM_LOG->s_pkmn_data[index].nature_boosted[4], 0, &string_buffer[0]);

    /* print the ability */

    rboxid_print(SWB_ABILITY, 0, 3, 1, &switch_color, 0, &pokemon_ability_names[SWM_LOG->s_pkmn_data[index].ability][0]);

    /* print the item */
//    u8 item = SWM_LOG->s_pkmn_data[index].item;
    //if (item == 0) {
        rboxid_print(SWB_ITEM, 0, 1, 1, &switch_color, 0, &str_no_item[0]);
//    } else {
        /* TODO Use item table */
        //rboxid_print(SWB_ITEM, 0, 1, 1, &switch_color, 0, &items[item].name[0]);
//    }

    for (u32 i = 0; i < 4; ++i) {
        u16 move = SWM_LOG->s_pkmn_data[index].move[i];
        if (move == 0) {
            if (battle_master->switch_main.type_objid[i + 2] != 0x3F) {
                OBJID_HIDE(battle_master->switch_main.type_objid[i + 2]);
            }
            if (battle_master->switch_main.type_objid[i + 6] != 0x3F) {
                OBJID_HIDE(battle_master->switch_main.type_objid[i + 6]);
            }
            continue;
        }
        rboxid_print(SWB_MOVES, 0, 0, (4 + 14 * i), &switch_color, 0, &moves[move].name[0]);

        if (moves[move].base_power)
            str_int_padding(moves[move].base_power, 3);
        else
            memcpy(string_buffer, str_invalid_num, 4);
        rboxid_print(SWB_POW, 0, 1, (4 + 14 * i), &switch_color, 0, &string_buffer[0]);

        if (moves[move].accuracy > 100)
            memcpy(string_buffer, str_invalid_num, 4);
        else
            str_int_padding(moves[move].accuracy, 3);
        rboxid_print(SWB_ACC, 0, 5, (4 + 14 * i), &switch_color, 0, &string_buffer[0]);

        str_int_padding(SWM_LOG->s_pkmn_data[index].pp[i], 2);
        rboxid_print(SWB_PP, 0, 2, (4 + 14 * i), &switch_color, 0, &string_buffer[0]);


        /*the move type icon*/
        switch_type_icon_load(moves[move].type, 49, 84 + (14 * i), i + 2);
        switch_cat_icon_load(moves[move].category, 226, 84 + (14 * i), i);
    }

    for (u32 i = SWB_ABILITY; i <= SWB_NAME; ++i) {
        rboxid_update(i, 3);
        rboxid_tilemap_update(i);
    }
    /* write those again, to overwrite empty space from stats */
    rboxid_update(SWB_ABILITY, 3);
    rboxid_tilemap_update(SWB_ABILITY);
    rboxid_update(SWB_ITEM, 3);
    rboxid_tilemap_update(SWB_ITEM);

    /* load the type icons */
    u16 species = SWM_LOG->s_pkmn_data[index].species;
    if ((enum MoveTypes)(pokemon_base_stats[species].type[0]) != MTYPE_EGG) {
        switch_type_icon_load(pokemon_base_stats[species].type[0], 47, 25, 0);
    }
    if (((enum MoveTypes)(pokemon_base_stats[species].type[1]) != MTYPE_EGG) &&
        (pokemon_base_stats[species].type[0] != pokemon_base_stats[species].type[1])) {
        switch_type_icon_load(pokemon_base_stats[species].type[1], 83, 25, 1);
    } else if (battle_master->switch_main.type_objid[1] != 0x3F) {
        OBJID_HIDE(battle_master->switch_main.type_objid[1]);
    }
}

void switch_obj_hide_all()
{
    for (u8 i = 0; i < 10; ++i) {
        if (battle_master->switch_main.type_objid[i] != 0x3F)
            OBJID_HIDE(battle_master->switch_main.type_objid[i]);
        if (i < SWM_LOG->list_count)
            OBJID_HIDE(battle_master->switch_main.icon_objid[i]);
        if (i < 3)
            OBJID_HIDE(battle_master->switch_main.slider_objid[0]);
    }
}

void switch_obj_show_all()
{
    for (u8 i = 0; i < 10; ++i) {
        if (battle_master->switch_main.type_objid[i] != 0x3F)
            OBJID_SHOW(battle_master->switch_main.type_objid[i]);
        if (i < SWM_LOG->list_count) {
            OBJID_SHOW(battle_master->switch_main.icon_objid[i]);
            objects[battle_master->switch_main.icon_objid[i]].final_oam.affine_mode = 1;
        }
        if (i < 3)
            OBJID_SHOW(battle_master->switch_main.slider_objid[0]);
    }
}


void switch_update_graphical(u8 cursor_position)
{
    // update size of icons based on cursor position
    for (u8 i = 0; i < SWM_LOG->list_count; i++) {
        if (i != cursor_position) {
            objects[battle_master->switch_main.icon_objid[i]].rotscale_table = switch_scale_table;
            objects[battle_master->switch_main.icon_objid[i]].callback = oac_nullsub;
        } else {
            objects[battle_master->switch_main.icon_objid[i]].rotscale_table = switch_scale_table_full;
            objects[battle_master->switch_main.icon_objid[i]].callback = icon_frame_change;
        }
    }

    // hide unused cursor icon baed on cursor position
    switch (cursor_position) {
        case 0:
            OBJID_HIDE(battle_master->switch_main.slider_objid[2]);
            OBJID_HIDE(battle_master->switch_main.slider_objid[1]);
            OBJID_SHOW(battle_master->switch_main.slider_objid[0]);
            break;
        case 1:
            objects[battle_master->switch_main.slider_objid[1]].pos1.y = Y_POS_MID_ONE;
            break;
        case 2:
            objects[battle_master->switch_main.slider_objid[1]].pos1.y = Y_POS_MID_TWO;
            break;
        case 3:
            objects[battle_master->switch_main.slider_objid[1]].pos1.y = Y_POS_MID_THREE;
            break;
        case 4:
            objects[battle_master->switch_main.slider_objid[1]].pos1.y = Y_POS_MID_FOUR;
            break;
        case 5:
            OBJID_HIDE(battle_master->switch_main.slider_objid[0]);
            OBJID_HIDE(battle_master->switch_main.slider_objid[1]);
            OBJID_SHOW(battle_master->switch_main.slider_objid[2]);
            break;
    };
    if ((cursor_position < 5) && (cursor_position > 0)) {
        OBJID_HIDE(battle_master->switch_main.slider_objid[0]);
        OBJID_SHOW(battle_master->switch_main.slider_objid[1]);
        OBJID_HIDE(battle_master->switch_main.slider_objid[2]);
    }
    // display information of cursor index pokemon
    switch_load_pokemon_data(cursor_position);
}

void switch_scene_main()
{
    switch (super.multi_purpose_state_tracker) {
    case 0:
        if (!pal_fade_control.active) {
            /* VRAM setup */
            rboxes_free();
            switch_setup();
            switch_fetch_all_data();
            super.multi_purpose_state_tracker++;
        }
        break;
    case 1:
        switch_load_background();
        rbox_init_from_templates(switch_boxes);
        switch_load_pokemon_data(0);
        switch_load_pokemon_icons();
        switch_load_scroll_box();
        switch_obj_hide_all();
        super.multi_purpose_state_tracker++;
        break;
    case 2:
        fade_screen(0xFFFFFFFF, 0, 16, 0, 0x0000);
        gpu_sync_bg_show(1);
        gpu_sync_bg_show(0);
        switch_obj_show_all();
        super.multi_purpose_state_tracker++;
        break;
    case 3:
        if (!pal_fade_control.active) {
            switch (super.buttons_new_remapped & (KEY_A | KEY_B | KEY_DOWN | KEY_UP)) {
            case KEY_A:
                /* Choose
                 * You only ever get to the switch menu and press A when you need to select a Pokemon.
                 * Need to display the confirmation text.
                 */
                break;
            case KEY_B:
                /* Exit the switch menu, unless you are forced to make a switch option */
                fade_screen(0xFFFFFFFF, 0, 0, 16, 0x0000);
                battle_master->switch_main.reason = ViewPokemon;
                super.multi_purpose_state_tracker = 5;
                break;
            case KEY_DOWN:
                if (battle_master->switch_main.position < (SWM_LOG->list_count-1)) {
                    battle_master->switch_main.position++;
                    switch_update_graphical(battle_master->switch_main.position);
                } else {
                    battle_master->switch_main.position = 0;
                    switch_update_graphical(0);
                }
                break;
            case KEY_UP:
                if (battle_master->switch_main.position > 0) {
                    battle_master->switch_main.position--;
                    switch_update_graphical(battle_master->switch_main.position);
                } else {
                    battle_master->switch_main.position = SWM_LOG->list_count - 1;
                    switch_update_graphical(battle_master->switch_main.position);
                }
                break;
            default:
                break;
            }
        }
        break;
    case 4:
        if (!pal_fade_control.active) {
            fade_screen(0xFFFFFFFF, 0, 16, 0, 0x0000);
        }
        break;
    case 5:
        if (!pal_fade_control.active) {
            fade_screen(0xFFFFFFFF, 0, 16, 0, 0x0000);
        }
        super.multi_purpose_state_tracker++;
        break;
    case 6:
        // free everything
        reset_boxes();
        free(battle_master->switch_main.sd);
        free(battle_master->switch_main.back_buffer);
        for (u8 i = 0; i < 10; ++i) {
            if (battle_master->switch_main.type_objid[i] != 0x3F) {
                obj_free(&objects[(battle_master->switch_main.type_objid[i])]);
                battle_master->switch_main.type_objid[i] = 0x3F;
            }
            if ((i < SWM_LOG->list_count) && (battle_master->switch_main.icon_objid[i] < 0x3F)) {
                obj_free(&objects[(battle_master->switch_main.icon_objid[i])]);
                battle_master->switch_main.icon_objid[i] = 0x3F;
            }
            if ((i < 3) && (battle_master->switch_main.slider_objid[i] < 0x3F)) {
                obj_free(&objects[(battle_master->switch_main.slider_objid[i])]);
                battle_master->switch_main.slider_objid[i] = 0x3F;
            }
        }
        super.multi_purpose_state_tracker++;
        break;
    case 7:
        // return to battle
        set_callback1(return_to_battle);
        super.multi_purpose_state_tracker = 0;
        return;
    };
}
