#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern void dprintf(const char * str, ...);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);

u8 clear_smog_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (src != user) return true;
    u8 defender = TARGET_OF(user);
    p_bank[defender]->b_data.attack = 0;
    p_bank[defender]->b_data.defense = 0;
    p_bank[defender]->b_data.speed = 0;
    p_bank[defender]->b_data.sp_atk = 0;
    p_bank[defender]->b_data.sp_def = 0;
    p_bank[defender]->b_data.accuracy = 0;
    p_bank[defender]->b_data.evasion = 0;
    p_bank[defender]->b_data.crit_mod = 0;
    enqueue_message(move, user, STRING_CLEAR_SMOG, 0);
    return true;
}

u8 psych_up_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    u8 target = TARGET_OF(user);
    B_ATTACK_BUFF(user) = B_ATTACK_BUFF(target);
    B_DEFENSE_BUFF(user) = B_DEFENSE_BUFF(target);
    B_SPEED_BUFF(user) = B_SPEED_BUFF(target);
    B_SPATTACK_BUFF(user) = B_SPATTACK_BUFF(target);
    B_SPDEFENSE_BUFF(user) = B_SPDEFENSE_BUFF(target);
    B_ACCURACY_BUFF(user) = B_ACCURACY_BUFF(target);
    B_EVASION_BUFF(user) = B_EVASION_BUFF(target);
    B_CRIT_BUFF(user) = B_CRIT_BUFF(target);
    enqueue_message(move, user, STRING_COPIED_STATS, 0);
    return true;
}

u8 haze_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (user != src) return true;
    for (u8 i = 0; i < BANK_MAX; i++) {
        p_bank[i]->b_data.attack = 0;
        p_bank[i]->b_data.defense = 0;
        p_bank[i]->b_data.speed = 0;
        p_bank[i]->b_data.sp_atk = 0;
        p_bank[i]->b_data.sp_def = 0;
        p_bank[i]->b_data.accuracy = 0;
        p_bank[i]->b_data.evasion = 0;
        p_bank[i]->b_data.crit_mod = 0;
    }
    enqueue_message(move, user, STRING_STAT_ELIMINATED, 0);
    return true;
}


u8 heart_swap_on_effect(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (src != user) return true;
    u8 target = TARGET_OF(user);

    u8 temp = B_ATTACK_BUFF(user);
    B_ATTACK_BUFF(user) = B_ATTACK_BUFF(target);
    B_ATTACK_BUFF(target) = temp;

    temp = B_DEFENSE_BUFF(user);
    B_DEFENSE_BUFF(user) = B_DEFENSE_BUFF(target);
    B_DEFENSE_BUFF(target) = temp;

    temp = B_SPEED_BUFF(user);
    B_SPEED_BUFF(user) = B_SPEED_BUFF(target);
    B_SPEED_BUFF(target) = temp;

    temp = B_SPATTACK_BUFF(user);
    B_SPATTACK_BUFF(user) = B_SPATTACK_BUFF(target);
    B_SPATTACK_BUFF(target) = temp;

    temp = B_SPDEFENSE_BUFF(user);
    B_SPDEFENSE_BUFF(user) = B_SPDEFENSE_BUFF(target);
    B_SPDEFENSE_BUFF(target) = temp;

    temp = B_ACCURACY_BUFF(user);
    B_ACCURACY_BUFF(user) = B_ACCURACY_BUFF(target);
    B_ACCURACY_BUFF(target) = temp;

    temp = B_EVASION_BUFF(user);
    B_EVASION_BUFF(user) = B_EVASION_BUFF(target);
    B_EVASION_BUFF(target) = temp;

    enqueue_message(NULL, user, STRING_HEART_SWAP, NULL);
    return true;
}
