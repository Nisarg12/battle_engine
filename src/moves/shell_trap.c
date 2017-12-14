#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern void dprintf(const char * str, ...);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern void set_status(u8 bank, enum Effect status);


/* Shell trap */
u8 shell_trap_ontryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    return false;
}

void shell_trap_on_damage(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (TARGET_OF(user) != src) return;
    if (B_MOVE_IS_PHYSICAL(user) && (B_MOVE_DMG(user) > 0)) {
        delete_callback_src((u32)shell_trap_ontryhit, src);
    }
}

void shell_trap_before_turn(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return;
    add_callback(CB_ON_DAMAGE_MOVE, 0, 0, user, (u32)shell_trap_on_damage);
    add_callback(CB_ON_TRYHIT_MOVE, 0, 0, user, (u32)shell_trap_ontryhit);
    enqueue_message(NULL, user, STRING_SETUP_SHELL_TRP, NULL);
}


/* Beak Blast */
void beak_blast_on_damage(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (TARGET_OF(user) != src) return;
    if (IS_CONTACT(move)) {
        set_status(user, AILMENT_BURN);
    }
}

void beak_blast_before_turn(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return;
    add_callback(CB_ON_DAMAGE_MOVE, 0, 0, user, (u32)beak_blast_on_damage);
    enqueue_message(NULL, user, STRING_BEAK_BLAST, NULL);
}


/* Focus Punch */
u8 focus_punch_ontryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    return false;
}

void focus_punch_on_damage(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (TARGET_OF(user) != src) return;
    if (B_MOVE_DMG(user) > 0) {
        add_callback(CB_ON_TRYHIT_MOVE, 0, 0, src, (u32)shell_trap_ontryhit);
    }
}

void focus_punch_before_turn(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return;
    add_callback(CB_ON_DAMAGE_MOVE, -100, 0, user, (u32)focus_punch_on_damage);
    enqueue_message(NULL, user, STRING_TIGHTEN_FOCUS, NULL);
}
