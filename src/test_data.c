#include <pokeagb/pokeagb.h>
#include "battle_data/pkmn_bank.h"
#include "battle_data/pkmn_bank_stats.h"
#include "battle_data/battle_state.h"

// Switch
const static bool USE_TESTS = true; // Change to false to not execute the test environment

/* Player data */
const static u16 player_species = SPECIES_GASTLY;
const static u16 player_move_1 = MOVE_QUASH;
const static u16 player_move_2 = MOVE_RECOVER;
const static u16 player_move_3 = MOVE_ACID;
const static u16 player_move_4 = MOVE_DRILL_PECK;
const static u8 player_level = 15;
const u8 player_ability = ABILITY_COMATOSE;

/* Opponent data */
const static u16 opponent_species = SPECIES_GOLEM;
const static u16 opponent_move_1 = MOVE_PURSUIT;
const static u16 opponent_move_2 = MOVE_PURSUIT;
const static u16 opponent_move_3 = MOVE_PURSUIT;
const static u16 opponent_move_4 = MOVE_PURSUIT;
const static u8 opponent_level = 55;
const u8 opponent_ability = ABILITY_GUTS;



void set_side_battle_components(u8 side)
{
	struct Pokemon* p = (side) ? (&party_player[0]) : (&party_opponent[0]);
	u16 species = (side) ? (player_species) : (opponent_species);
	u16 move_1 = (side) ? (player_move_1) : (opponent_move_1);
	u16 move_2 = (side) ? (player_move_2) : (opponent_move_2);
	u16 move_3 = (side) ? (player_move_3) : (opponent_move_3);
	u16 move_4 = (side) ? (player_move_4) : (opponent_move_4);
	u8 level = (side) ? (player_level) : (opponent_level);

	pokemon_setattr(p, REQUEST_SPECIES, &species);
	pokemon_setattr(p, REQUEST_NICK, &pokemon_names[species]);
	u32 species_exp_index = (pokemon_base_stats[species].exp_growth * 0x194);
	u32 *exp_needed = (u32*) (0x8253AE4 + (species_exp_index + (level * 4)));
	pokemon_setattr(p, REQUEST_EXP_POINTS, exp_needed);
	recalculate_stats(p);
	pokemon_setattr(p, REQUEST_MOVE1, &move_1);
	pokemon_setattr(p, REQUEST_MOVE2, &move_2);
	pokemon_setattr(p, REQUEST_MOVE3, &move_3);
	pokemon_setattr(p, REQUEST_MOVE4, &move_4);
	pokemon_setattr(p, REQUEST_PP1, &moves[move_1].pp);
	pokemon_setattr(p, REQUEST_PP2, &moves[move_2].pp);
	pokemon_setattr(p, REQUEST_PP3, &moves[move_3].pp);
	pokemon_setattr(p, REQUEST_PP4, &moves[move_4].pp);
}

void test_battle_data()
{
	if (USE_TESTS) {
		set_side_battle_components(0);
		set_side_battle_components(1);
	}
}
