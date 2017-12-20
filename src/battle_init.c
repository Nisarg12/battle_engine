#include <pokeagb/pokeagb.h>
#include "battle_data/pkmn_bank.h"
#include "battle_data/pkmn_bank_stats.h"
#include "battle_slide_in_data/battle_obj_sliding.h"
#include "battle_data/battle_state.h"
#include "battle_text/battle_textbox_gfx.h"
#include "battle_actions/actions.h"

extern void setup(void);
extern void test_battle_data(void);
extern void allocate_battle_structs(void);
extern void set_active_banks(enum BattleTypes b_flag);
extern void intro_wild_battle(void);


/* Battle Entry point */
void init_battle_elements()
{
    // reset game screen and callbacks
    setup();
    allocate_battle_structs();
    // battle testing presets initialize
    test_battle_data();
    battle_type_flag = BATTLE_MODE_WILD;
    set_callback1(intro_wild_battle);
    super.multi_purpose_state_tracker = 0;
}


void nullify_object_trackers()
{
    // initialization finished, set c1 option selection
    for(u8 i = 0; i < 10; ++i) {
        battle_master->switch_main.type_objid[i] = 0x3F;
    }

    battle_master->battle_cursor.objid_mv_crsr[0] = 0x3F;
    battle_master->battle_cursor.objid_mv_crsr[1] = 0x3F;

    for (u8 i = 0; i < 4; i++) {
        battle_master->type_objid[i] = 0x3F;
        battle_master->move_pss_objid[i] = 0x3F;
        battle_master->move_pp_objid[i] = 0x3F;
        battle_master->move_name_objid[i] = 0x3F;
    }
}


void allocate_battle_structs()
{
    // All 4 banks
    void* p_bank_data = malloc_and_clear(sizeof(struct pkmn_bank) * 4);
    for(u8 i = 0; i < BANK_MAX; i++) {
        p_bank[i] = (struct pkmn_bank*)(p_bank_data + (sizeof(struct pkmn_bank) * i));
        for(u8 j = 0; j < 4; j++) {
            p_bank[i]->objid_hpbox[j] = 0x3F;
        }
        p_bank[i]->objid = 0x3F;
    }
    // Battle entrance tracker
    bs_env_windows = (struct bs_elements_positions*)malloc_and_clear(sizeof(struct bs_elements_positions));
    // Battle state master initialize
    battle_master = (struct battle_main*)malloc_and_clear(sizeof(struct battle_main));

    // actions linked list head nullify
    battle_master->action_head = NULL;
    nullify_object_trackers();
}
