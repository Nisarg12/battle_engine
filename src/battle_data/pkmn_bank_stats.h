#ifndef PKMN_BANK_STAT_H_
#define PKMN_BANK_STAT_H_

#include <pokeagb/pokeagb.h>
#include "battle_data/pkmn_bank.h"
#include "battle_state.h"
#include "moves/moves.h"
#include "battle_actions/actions.h"
#include "battle_text/battle_pick_message.h"

extern bool has_volatile(u8 bank, enum Volatiles v);
extern void clear_volatile(u8 bank, enum Volatiles v);
extern void add_volatile(u8 bank, enum Volatiles v);

// this one is unrelated but I want to have it here
#define CPUFSCPY 0
#define CPUFSSET 1
#define CPUModeFS(size, mode) ((size >> 2) | (mode << 24))
#define OBJID_HIDE(objid) objects[objid].final_oam.affine_mode = 2
#define OBJID_SHOW(objid) objects[objid].final_oam.affine_mode = 0
#define OBJID_SHOW_AFFINE(objid) objects[objid].final_oam.affine_mode = 1

/* To be removed when species expansion hits */
#define SPECIES_ASH_GRENINJA 0
#define SPECIES_ALOLAN_DIGLETT 0x999
#define SPECIES_ALOLAN_DUGTRIO 0x999
#define SPECIES_SANDYGAST 0x999
#define SPECIES_PALOSSAND 0x999
#define SPECIES_MEGA_GENGAR 0x999
#define SPECIES_ARCEUS 1000
#define SPECIES_SILVALLY 1001

/*
 * Names of stats as IDs in stage_modify_stat
 */
#define ATTACK_MOD 0
#define DEFENSE_MOD 1
#define SPEED_MOD 2
#define SPATTACK_MOD 3
#define SPDEFENSE_MOD 4
#define ACCURACY_MOD 5
#define EVASION_MOD 6
#define CRIT_CHANCE_MOD 7

/*
 * Get Pokemon's current battle stats based on stage changes to stat
 *
 */
#define B_ATTACK_STAT(bank) stage_modify_stat(p_bank[bank]->b_data.attack_raw, p_bank[bank]->b_data.attack, 0, bank)
#define B_DEFENSE_STAT(bank) stage_modify_stat(p_bank[bank]->b_data.defense_raw, p_bank[bank]->b_data.defense, 1, bank)
#define B_SPEED_STAT(bank) stage_modify_stat(p_bank[bank]->b_data.speed_raw, p_bank[bank]->b_data.speed, 2, bank)
#define B_SPATTACK_STAT(bank) stage_modify_stat(p_bank[bank]->b_data.sp_atk_raw, p_bank[bank]->b_data.sp_atk, 3, bank)
#define B_SPDEFENSE_STAT(bank) stage_modify_stat(p_bank[bank]->b_data.sp_def_raw, p_bank[bank]->b_data.sp_def, 4, bank)
#define B_ACCURACY_STAT(bank) stage_modify_stat(0, p_bank[bank]->b_data.accuracy, 5, bank)
#define B_EVASION_STAT(bank) stage_modify_stat(0, p_bank[bank]->b_data.evasion, 6, bank)
#define B_CRITCHANCE_STAT(bank) stage_modify_stat(0, p_bank[bank]->b_data.crit_mod, 7, bank)

#define B_ATTACKING_STAT(attacker) (B_MOVE_CATEGORY(attacker) == MOVE_PHYSICAL) ? B_ATTACK_STAT(attacker) : B_SPATTACK_STAT(attacker)
#define B_DEFENDING_STAT(defender, category) (category == MOVE_PHYSICAL) ? B_DEFENSE_STAT(defender) : B_SPDEFENSE_STAT(defender)

#define B_ATTACK_BUFF(bank) (p_bank[bank]->b_data.attack)
#define B_DEFENSE_BUFF(bank) (p_bank[bank]->b_data.defense)
#define B_SPEED_BUFF(bank) (p_bank[bank]->b_data.speed)
#define B_SPATTACK_BUFF(bank) (p_bank[bank]->b_data.sp_atk)
#define B_SPDEFENSE_BUFF(bank) (p_bank[bank]->b_data.sp_def)
#define B_ACCURACY_BUFF(bank)  (p_bank[bank]->b_data.accuracy)
#define B_EVASION_BUFF(bank) (p_bank[bank]->b_data.evasion)
#define B_CRIT_BUFF(bank) (p_bank[bank]->b_data.crit_mod)

#define B_WEIGHT(bank) (p_bank[bank]->b_data.weight)

/*
 * Umodified battle sats
 */
#define B_ATTACK_STAT_UMOD(bank) (p_bank[bank]->b_data.attack_raw)
#define B_DEFENSE_STAT_UMOD(bank) (p_bank[bank]->b_data.defense_raw)
#define B_SPEED_STAT_UMOD(bank) (p_bank[bank]->b_data.speed_raw)
#define B_SPATTACK_STAT_UMOD(bank) (p_bank[bank]->b_data.sp_atk_raw)
#define B_SPDEFENSE_STAT_UMOD(bank) (p_bank[bank]->b_data.sp_def_raw)

#define NUM_MOD(number, percent) ((number * percent) / 100)

/*
 * P_bank b_data volatile macros
 */
#define ADD_VOLATILE(bank, v) (add_volatile(bank, v))
// clear the bit, even when not set.
#define CLEAR_VOLATILE(bank, v) (clear_volatile(bank, v))
#define HAS_VOLATILE(bank, v) (has_volatile(bank, v))

/*
 * P_bank b_data set and fetch macros
 *
 */
#define B_SPECIES(bank) (p_bank[bank]->b_data.species)
#define CURRENT_MOVE(bank) (p_bank[bank]->b_data.current_move)
#define LAST_MOVE(bank) (p_bank[bank]->b_data.last_move)
#define BANK_ABILITY(bank) (p_bank[bank]->b_data.ability)
#define B_PKMN(bank) (p_bank[bank]->this_pkmn)
#define B_FLINCH(bank) (p_bank[bank]->b_data.flinch)
#define B_MOVE_FLINCH(bank) (battle_master->b_moves[bank].flinch)
#define B_MOVE_TYPE(bank, i) (battle_master->b_moves[bank].type[i])
#define B_MOVE_POWER(bank) (battle_master->b_moves[bank].power)
#define B_MOVE_STAB(bank) (battle_master->b_moves[bank].stab)
#define B_MOVE_ACCURACY(bank) (battle_master->b_moves[bank].accuracy)
#define B_MOVE_PRIORITY(bank) (battle_master->b_moves[bank].priority)
#define B_MOVE_REMOVE_CONTACT(bank) (battle_master->b_moves[bank].remove_contact)
#define B_MOVE_IGNORING_ABILITIES(bank) (battle_master->b_moves[bank].ignore_abilities)
#define B_MOVE_HAS_TYPE(bank, type) ((B_MOVE_TYPE(bank, 0) == type) || (B_MOVE_TYPE(bank, 1) == type))
#define B_MOVE_IS_STATUS(bank) (battle_master->b_moves[bank].category == MOVE_STATUS)
#define B_MOVE_IS_SPECIAL(bank) (battle_master->b_moves[bank].category == MOVE_SPECIAL)
#define B_MOVE_IS_PHYSICAL(bank) (battle_master->b_moves[bank].category == MOVE_PHYSICAL)
#define B_MOVE_CATEGORY(bank) (battle_master->b_moves[bank].category)
#define B_MOVE_WILL_CRIT(bank) (battle_master->b_moves[bank].will_crit)
#define B_MOVE_CAN_CRIT(bank) (battle_master->b_moves[bank].can_crit)
#define B_MOVE_DMG(bank) (battle_master->b_moves[bank].dmg)
#define B_MOVE_EFFECTIVENESS(bank) (battle_master->b_moves[bank].effectiveness)
#define B_MOVE_IGNORE_ATK(bank) (battle_master->b_moves[bank].ignore_target_atk)
#define B_MOVE_IGNORE_DEF(bank) (battle_master->b_moves[bank].ignore_target_def)
#define B_MOVE_IGNORE_EVASION(bank) (battle_master->b_moves[bank].ignore_target_evasion)
#define B_MOVE_MULTI(bank) ((battle_master->b_moves[bank].hit_times > 1) || (battle_master->b_moves[bank].hit_counter > 1))
#define PROTECTION_TURNS(bank) (p_bank[bank]->b_data.protect_used_turns)
#define B_GENDER(bank) (p_bank[bank]->b_data.gender)
#define B_REDUCE_PP(bank) (p_bank[bank]->b_data.reduce_pp)
#define B_IS_TRAPPED(bank) (p_bank[bank]->b_data.is_trapped)
#define B_PKMN_TYPE(bank, index) (p_bank[bank]->b_data.type[index])
#define B_CURRENT_HP(bank) (p_bank[bank]->b_data.current_hp)
#define B_LEVEL(bank) (p_bank[bank]->b_data.level)
#define TOTAL_HP(bank) (p_bank[bank]->b_data.total_hp)
#define B_STATUS(bank) (p_bank[bank]->b_data.status)
#define B_PSTATUS(bank) (p_bank[bank]->b_data.pseudo_ailment)
#define TARGET_OF(bank) (p_bank[bank]->b_data.my_target)
#define SET_CONFUSION_TURNS(bank, v) (p_bank[bank]->b_data.confusion_turns = v)
#define B_IS_GROUNDED(bank) (p_bank[bank]->b_data.is_grounded)
#define B_INFILTRATES(bank) (battle_master->b_moves[bank].infiltrates)
#define B_HEAL(bank) (battle_master->b_moves[bank].heal)
#define B_GET_ITEM(bank) (p_bank[bank]->b_data.item)
#define B_MOVE_FAILED(bank) (p_bank[bank]->b_data.move_failed)
#define B_LAST_MOVE_FAILED(bank) (p_bank[bank]->b_data.last_move_failed)
#define B_IS_FAINTED(bank) ((p_bank[bank]->b_data.fainted) || (B_CURRENT_HP(bank) < 1))
#define B_FAINTED(bank) (p_bank[bank]->b_data.fainted)
#define B_GET_MOVE(bank, id) (p_bank[bank]->b_data.moves[id])
#define B_GET_MOVE_PP(bank, id) (p_bank[bank]->b_data.move_pp[id])

/*
 *
 * Secondary status macros from battle state
 */
#define B_USER_STAT_MOD_CHANCE(bank, stat) (battle_master->b_moves[bank].b_procs.chance_user[stat])
#define B_USER_STAT_MOD_AMOUNT(bank, stat) (battle_master->b_moves[bank].b_procs.user_amounts[stat])

#define B_TARGET_STAT_MOD_CHANCE(bank, stat) (battle_master->b_moves[bank].b_procs.chance_target[stat])
#define B_TARGET_STAT_MOD_AMOUNT(bank, stat) (battle_master->b_moves[bank].b_procs.target_amounts[stat])

#define B_AILMENT_PROCS_CHANCE_USER(bank) (battle_master->b_moves[bank].b_procs.ailment_user_chance)
#define B_AILMENT_PROCS_CHANCE_TARGET(bank) (battle_master->b_moves[bank].b_procs.ailment_target_chance)
#define B_AILMENT_PROCS_USER(bank) (battle_master->b_moves[bank].b_procs.ailment_user)
#define B_AILMENT_PROCS_TARGET(bank) (battle_master->b_moves[bank].b_procs.ailment_target)


/*
 * General Pbank macros
 */
#define PLAYER_SIDE 0
#define OPPONENT_SIDE 1

#define SIDE_OF(bank) ((bank > 1) ? 1 : 0)
#define ACTIVE_BANK(bank) (p_bank[bank]->b_data.is_active_bank)

#define ACTION_PRANKSTER (CURRENT_ACTION->prankstered)
#define ACTION_BOUNCED (CURRENT_ACTION->has_bounced)

#define MOVE_EFFECTIVENESS(target_type, dmg_type) (effectiveness_chart[((target_type * 19) + (dmg_type))])

extern s8 ability_priority_mod(u8 bank, u16 move_id);
extern s8 ability_attack_mod(u8 bank, u16 stat_total);
extern s8 ability_defense_mod(u8 bank, u16 stat_total);
extern s8 ability_speed_mod(u8 bank, u16 stat_total);
extern s8 ability_sp_defense_mod(u8 bank, u16 stat_total);
extern s8 ability_sp_attack_mod(u8 bank, u16 stat_total);
extern s8 ability_critchance_mod(u8 bank, u16 stat_total);
extern s8 ability_accuracy_mod(u8 bank, u16 stat);
extern s8 ability_evasion_mod(u8 bank, u16 stat);
extern u16 stage_modify_stat(u16 stat, s8 mod, u8 id, u8 bank);
extern void ability_on_before_switch(u8 bank);
extern void ability_on_switch(u8 bank);
extern void ability_on_modify_move(u8 bank, u8 target, u16 move);
extern u8 ability_on_tryhit(u8 attacker, u8 defender, u16 move);


#endif /* PKMN_BANK_STAT_H_ */
