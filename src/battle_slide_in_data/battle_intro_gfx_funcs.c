#include <pokeagb/pokeagb.h>
#include "battle_obj_sliding.h"
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"
#include "../generated/images/battle_terrains/grass/grass_bg.h"
#include "../../generated/images/battle_terrains/grass/grass_entry.h"
#include "../battle_text/battle_textbox_gfx.h"
#include "../libgba_assets/gba_dma.h"

extern void CpuFastSet(void*, void*, u32);
/* Standard BG configuration for battle start */
const struct BgConfig bg_config_data[4] = {
    {
        .padding = 0,
        .b_padding = 0,
        .priority = 0,
        .palette = 0,
        .size = 0,
        .map_base = 31,
        .character_base = 3,
        .bgid = 0,
    },
    {
        .padding = 0,
        .b_padding = 0,
        .priority = 1,
        .palette = 0,
        .size = 0,
        .map_base = 30,
        .character_base = 2,
        .bgid = 1,
    },
    {
        .padding = 0,
        .b_padding = 0,
        .priority = 2,
        .palette = 0,
        .size = 0,
        .map_base = 29,
        .character_base = 1,
        .bgid = 2,
    },
    {
        .padding = 0,
        .b_padding = 0,
        .priority = 3,
        .palette = 0,
        .size = 1,
        .map_base = 28,
        .character_base = 0,
        .bgid = 3,
    },
};


void pick_and_load_battle_bgs()
{
    /* TODO Make this change based on where you are. */
    // copy image BG background
    void* char_base = (void *)0x6000000;
    void* map_base = (void *)0x600E000;
    lz77UnCompVram((void *)grass_bgTiles, char_base);
    lz77UnCompVram((void *)grass_bgMap, map_base);

    // copy image BG entry image
    char_base = (void *)0x6008000;
    map_base = (void *)0x600F000;
    lz77UnCompVram((void *)grass_entryTiles, char_base);
    lz77UnCompVram((void *)grass_entryMap, map_base);

    // copy textbox image
    char_base = (void *)0x600C000;
    map_base = (void *)0x600F800;
    lz77UnCompVram((void *)bboxTiles, char_base);

    DMA3COPY(battle_textboxMap, map_base, sizeof(battle_textboxMap));

    // write palettes
    gpu_pal_apply_compressed((void *)grass_bgPal, 0, 64);
    gpu_pal_apply((void*)grass_entryPal, 16 * 4, 32);
    gpu_pal_apply((void*)bboxPal, 16 * 5, 32);
}

void pick_and_load_battle_bgs_no_entry(const void* textbox_map)
{
    /* TODO Make this change based on where you are. */
    // copy image BG background
    void* char_base = (void *)0x6000000;
    void* map_base = (void *)0x600E000;
    lz77UnCompVram((void *)grass_bgTiles, char_base);
    lz77UnCompVram((void *)grass_bgMap, map_base);

    // copy textbox image
    char_base = (void *)0x600C000;
    map_base = (void *)0x600F800;
    lz77UnCompVram((void *)bboxTiles, char_base);

    CpuFastSet((void*)textbox_map, (void*)map_base, CPUModeFS(0x800, CPUFSCPY));
    // write palettes
    gpu_pal_apply_compressed((void *)grass_bgPal, 0, 64);
    gpu_pal_apply((void*)bboxPal, 16 * 5, 32);
}

struct Pokemon* pick_first_usable_pokemon(struct Pokemon* p, u8 party_size)
{
    for (u8 i = 0; i < party_size; i++) {
        u16 species = pokemon_getattr(&p[i], REQUEST_SPECIES, NULL);
        bool is_egg = pokemon_getattr(&p[i], REQUEST_IS_EGG, NULL);
        u16 current_hp = pokemon_getattr(&p[i], REQUEST_CURRENT_HP, NULL);

        // valid if it's a valid species, isn't an egg, and is alive.
        if ((species < SPECIES_MAX) && (species > SPECIES_MISSINGNO) &&
         (!is_egg) && (current_hp > 0)) {
            return (&p[i]);
        }
    }
    return NULL; // failed to find an active battler. Maybe the battle should whiteout here.
}

/* Spawn battler objects. Set active banks based on battle type */
extern void create_sprites_wild_battlers(void);
void battle_scene_intialize_sprites()
{
    bs_env_windows->player_trainer_objid = 0x3F;
    bs_env_windows->partner_trainer_objid = 0x3F;
    switch (battle_type_flag) {
        case BATTLE_MODE_WILD:
            p_bank[PLAYER_SINGLES_BANK]->this_pkmn = pick_first_usable_pokemon(&party_player[0], 6);
            p_bank[OPPONENT_SINGLES_BANK]->this_pkmn = pick_first_usable_pokemon(&party_opponent[0], 6);
            p_bank[PLAYER_SINGLES_BANK]->b_data.is_active_bank = true;
            p_bank[OPPONENT_SINGLES_BANK]->b_data.is_active_bank = true;
            create_sprites_wild_battlers();
            break;
        case BATTLE_MODE_WILD_DOUBLE:
        case BATTLE_MODE_TRAINER:
        case BATTLE_MODE_TRAINER_DOUBLE:
        case BATTLE_MODE_PARTNER:
        default:
            dprintf("FAILED to select a valid battle mode.\n");
            break;
    };

}


void battlers_move_into_scene()
{
    switch (battle_type_flag) {
        case BATTLE_MODE_WILD:
            // player
            if (bs_env_windows->player_trainer_objid != 0x3F)
                objects[bs_env_windows->player_trainer_objid].pos1.x -= 3;
            // wild mon
             if (p_bank[OPPONENT_SINGLES_BANK]->objid != 0x3F)
                 objects[p_bank[OPPONENT_SINGLES_BANK]->objid].pos1.x += 3;
            break;
        case BATTLE_MODE_WILD_DOUBLE:
        case BATTLE_MODE_TRAINER:
        case BATTLE_MODE_TRAINER_DOUBLE:
        case BATTLE_MODE_PARTNER:
        default:
            dprintf("FAILED to select a valid battle mode.\n");
            break;
    };
}


extern void player_throwball_and_moveout_scene(struct Object*);
void player_sendout_animation_singles()
{
    objects[bs_env_windows->player_trainer_objid].anim_number++;
    objects[bs_env_windows->player_trainer_objid].callback = player_throwball_and_moveout_scene;
    objects[bs_env_windows->player_trainer_objid].priv[2] = PLAYER_SINGLES_BANK;
    bs_anim_status = 1;
}
