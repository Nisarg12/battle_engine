#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"


extern u8 spawn_pkmn_backsprite_obj_slot(u8 bank, u16 tag);
extern void dprintf(const char * str, ...);


static const struct RotscaleFrame shrink_grow[] = {
    {-200, -200, 0, 1, 0},
    {20, 20, 0, 10, 0},
    {0x7FFF, 0, 0, 0, 0}
};

static const struct RotscaleFrame* shrink_grow_ptr[] = {shrink_grow};


void pkmn_sendingout_objc(struct Object* obj)
{
    /* Send out objects grow from bottom up */
    if (obj->priv[5] < obj->pos1.y) {
        if ((obj->pos1.y - 6) < obj->priv[5]) {
            obj->pos1.y = obj->priv[5];
        } else {
            obj->pos1.y -= 6;
        }
    }
   obj->priv[2]++;
    if (obj->priv[2] % obj->priv[1])
        return;
    if (!obj->priv[0]) {
        REG_BLDCNT = 0;
        obj->callback = oac_nullsub;
        obj->final_oam.affine_mode = 1;
        bs_anim_status = 0;
        return;
    }
    REG_BLDY = obj->priv[0];
    obj->priv[0]--;
}

u8 send_out_backsprite(u8 bank)
{
    // send out pokemon's backsprite based on bank
    affine_reset_all();
    u8 objid = spawn_pkmn_backsprite_obj_slot(bank, 0x810);
    objects[objid].rotscale_table = shrink_grow_ptr;
    objects[objid].final_oam.affine_mode = 1;
    objects[objid].callback = pkmn_sendingout_objc;
    objects[objid].priv[0] = 10;
    objects[objid].priv[1] = 3; // delay
    objects[objid].priv[5] = objects[objid].pos1.y;
    objects[objid].pos1.y += 64;
    p_bank[0]->objid = objid;
    u8 pal_slot = objects[objid].final_oam.palette_num;
    u32 pal_fade = ((1 << (pal_slot + 16)));

    // flash white for 4 frames
    fade_screen(pal_fade , 10, 0x10, 0x0, 0x7ADF);
    REG_BLDCNT = (BLDCNT_BG1_SRC | BLDCNT_BG2_SRC | BLDCNT_BG3_SRC | BLDCNT_LIGHTEN | BLDCNT_BG1_DST | BLDCNT_BG2_DST | BLDCNT_BG3_DST);
    return objid;
}

static const struct RotscaleFrame spin[] = {
    {0, 0, 0x00, 8, 0x0}, // 8 frame wait
    {0, 0, 10, 3, 0x0}, // 10, 3 frames
    {0, 0, 20, 4, 0x0}, // 20, 4 frames
    {0, 0, 30, 255, 0x0}, // 30, 255 frame
    {0x7FFF, 0x0, 0x0, 0x0, 0x0} // end
};
static const struct RotscaleFrame (*spin_ptr)[] = (const struct RotscaleFrame(*)[])&spin;
void pokeball_player_throw_arc(struct Object* obj)
{
    if (obj->priv[6] < 170) {
        if (obj->priv[6] < 80) {
            obj->priv[6] += 2;
            if (obj->priv[6] % 3)
                obj->pos1.x += 1;
            obj->pos1.y = (get_pingpong(obj->priv[6], -30)) + 100;
        } else if (obj->priv[6] < 130) {
            obj->priv[6] += 3;
            if (obj->priv[6] % 2)
                obj->pos1.x += 1;
            obj->pos1.y = (get_pingpong(obj->priv[6], -30)) + 100;
        } else {
            if (obj->priv[6] % 2)
                obj->pos1.x += 1;
            obj->priv[6] += 5;
            obj->pos1.y = (get_pingpong(obj->priv[6], -30)) + 100;

        }

    } else {
        obj->final_oam.affine_mode = 0;
        send_out_backsprite(obj->priv[0]);
        obj_free(obj);
    }

}


void make_spinning_pokeball(s16 x, s16 y, u8 bank)
{
    // get pokeball index in table
    struct Pokemon* p = p_bank[bank]->this_pkmn;
    u16 ball_id = pokemon_getattr(p, REQUEST_POKEBALL, NULL);
    ball_id = ball_number_to_ball_processing_index(ball_id);
    gpu_tile_obj_decompress_alloc_tag_and_upload(&ball_tiles[ball_id]);
    gpu_pal_decompress_alloc_tag_and_upload(&ball_palettes[ball_id]);
    u8 objid = template_instanciate_forward_search((struct Template*)0x82606F4, x, y, 0);
    objects[objid].callback = pokeball_player_throw_arc;
    objects[objid].priv[0] = bank;

    objects[objid].priv[6] = 0x25; // Delay timer
    objects[objid].rotscale_table = (const struct RotscaleFrame(**)[])&spin_ptr;
    objects[objid].final_oam.affine_mode = 1;
}
