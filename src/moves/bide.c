#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern void dprintf(const char * str, ...);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);

/* bide */
u8 bide_on_modify_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    TARGET_OF(user) = p_bank[user]->b_data.last_attacked_by;
    B_MOVE_DMG(user) = acb->data_ptr;
    p_bank[src]->b_data.skip_move_select = false;
    enqueue_message(MOVE_BIDE, user, STRING_UNLEASHED_ENERGY, NULL);
    B_REDUCE_PP(src) = false;
    return true;
}

void bide_on_damage_record(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (TARGET_OF(user) != src) return;
    // record dmg taken
    if (HAS_VOLATILE(src, VOLATILE_BIDE)) {
        acb->data_ptr+= B_MOVE_DMG(user);
    }
    return;
}

u8 bide_before_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (!HAS_VOLATILE(user, VOLATILE_BIDE)) return true;
    if (acb->duration > 1) {
        enqueue_message(MOVE_BIDE, user, STRING_BIDE_CHARGE, NULL);
    } else {
        u8 id = add_callback(CB_ON_MODIFY_MOVE, 1, 0, src, (u32)bide_on_modify_move);
        u8 id2 = get_callback_src((u32)bide_on_damage_record, user);
        CB_MASTER[id].data_ptr = (CB_MASTER[id2].data_ptr << 1);
        CB_MASTER[id2].in_use = false;
        p_bank[src]->b_data.skip_move_select = false;
        acb->in_use = false;
        return 4;
    }
    return 3;
}

void bide_on_after_move(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return;
    CLEAR_VOLATILE(user, VOLATILE_BIDE);
    acb->in_use = false;
}

u8 bide_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    if (HAS_VOLATILE(user, VOLATILE_BIDE)) return true;
    enqueue_message(MOVE_BIDE, user, STRING_BIDE_CHARGE, NULL);
    ADD_VOLATILE(user, VOLATILE_BIDE);
    add_callback(CB_ON_DAMAGE_MOVE, 0, 2, user, (u32)bide_on_damage_record);
    add_callback(CB_ON_BEFORE_MOVE, 0, 3, user, (u32)bide_before_move);
    u8 id = add_callback(CB_ON_AFTER_MOVE, 0, 2, user, (u32)bide_on_after_move);
    CB_MASTER[id].delay_before_effect = 1;
    p_bank[src]->b_data.skip_move_select = true;
    B_REDUCE_PP(src) = true;
    return true;
}
