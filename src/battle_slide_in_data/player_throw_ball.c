#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "battle_obj_sliding.h"

extern void make_spinning_pokeball(s16 x, s16 y, u8 bank);


void player_throwball_and_moveout_scene(struct Object* obj)
{
    switch (obj->priv[0]) {
        case 0:
            if (obj->pos1.x == -494) {
                obj->pos1.x = -480;
                obj->priv[0]++;
            } else {
                obj->pos1.x -= 2;
            }
            break;
        case 1:
            // spawn ball
            if (obj->pos1.x == -492) {
                obj->priv[0]++;
                make_spinning_pokeball(53, 64, obj->priv[2]);
            }
            obj->pos1.x -= 2;
            break;
        case 2:
            if (obj->pos1.x < -530) {
                obj_free(obj);
            } else {
                obj->pos1.x -= 2;
            }
            break;
    };
}
