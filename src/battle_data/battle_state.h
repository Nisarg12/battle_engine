#ifndef BATTLE_STATE_H_
#define BATTLE_STATE_H_

#include "pkmn_bank.h"
#include "../moves/moves.h"
#include "../anonymous_callbacks/anonymous_callbacks.h"
#include <pokeagb/pokeagb.h>

#define GAME_STATE super.multi_purpose_state_tracker
typedef void (*ResidualEffectCallback)(u8);

enum Effect {
    EFFECT_NONE,
    EFFECT_SLEEP,
    EFFECT_POISON,
    EFFECT_BURN,
    EFFECT_FREEZE,
    EFFECT_PARALYZE,
    EFFECT_BAD_POISON,
    EFFECT_CONFUSION,
    EFFECT_CURE,
    EFFECT_INFACTUATION,
};

enum WeatherTypes {
    WEATHER_CLEAR,
    WEATHER_RAIN,
    WEATHER_SUN,
    WEATHER_SANDSTORM,
    WEATHER_HAIL,
    WEATHER_HARSH_SUN,
    WEATHER_HARSH_RAIN,
    WEATHER_MYSTERIOUS_AIR_CURRENT,
};


enum TypeEffectiveness {
    TE_IMMUNE,
    TE_NOT_VERY_EFFECTIVE,
    TE_EFFECTIVE,
    TE_SUPER_EFFECTIVE,
    TE_OHKO,
};

struct battle_selection_cursor {
    u8 objid;
    u8 objid_mv_crsr[2];
    u8 cursor_pos; // 0-3
    u8 cursor_x[6];
    u8 cursor_y[6];
};

struct global_message {
    u16 move_id;
    u8 bank;
    u8 string_id;
    u16 effect;
};

struct battle_field_state {
    u8 payday_coins;
    u32 is_raining : 1;
    u32 is_sunny : 1;
    u32 is_sandstorm : 1;
    u32 is_hail : 1;
    u32 is_desolate_land : 1;
    u32 is_primordial_sea : 1;
    u32 is_delta_stream : 1;
    u32 suppress_weather : 1;
    u32 is_electric_terrain : 1;
    u32 is_grassy_terrain : 1;
    u32 is_misty_terrain : 1;
    u32 is_psychic_terrain : 1;
    u32 speed_inverse : 1;
    u8 weather_turns;
    u16 last_used_move;
};

struct move_used {
    u8 user_bank;
    u16 move_id;
    s8 priority;
    u8 stab;
    u16 power; //prevent clamp up for moves over effectively 255 bp
    u8 category;
    u8 type[2];
    u8 flinch;
    u8 accuracy; // over 100 = never miss

    u16 remove_contact : 1;
    u16 copied : 1;
    u16 ignore_abilities : 1;
    u16 prankstered : 1;
    u16 infiltrates : 1;
    u16 will_crit : 1;
    u16 can_crit : 1;
    u16 ignore_target_atk : 1;
    u16 ignore_target_def : 1;
    u16 ignore_target_evasion : 1;
    u16 has_bounced : 1;

    struct move_procs b_procs;

    u8 hit_times;
    u8 hit_counter;
    u16 dmg;
    enum TypeEffectiveness effectiveness;
    u16 heal;
    u16 after_dmg;
};

typedef u16 (*StatModifierCallback)(u16 stat, u8 id, u8 bank);

struct switch_menu {
    u8 type_objid[10];
    u8 slider_objid[3];
    u8 icon_objid[6];
    u8 position;
    bool will_switch;
};

struct battle_main {
    struct battle_field_state field_state;

    /* Main Battle callbacks */
    struct anonymous_callback anon_cb_master[ANON_CB_MAX];
    u8 cb_execution_order[ANON_CB_MAX];
    u8 current_cb_index : 7;
    u8 executing : 1;
    u8 bank_state;

    /* Object ids and positions */
    struct battle_selection_cursor battle_cursor;
    u8 selected_option;
    u8 type_objid[4];
    u8 move_name_objid[4];
    u8 move_pss_objid[4];
    u8 move_pp_objid[4];
    u8 fight_menu_content_spawned;

    /* Message system variables */
    struct global_message b_message[7]; // message queue depth is 5
    u8 queue_size;
    u8 queue_front_index;
    u8 state;
    SuperCallback c1;

    /* Battle turn order variables */
    u8 first_bank;
    u8 second_bank;
    u8 execution_index;
    u8 bank_hit_list[4];
    struct move_used b_moves[2];
    u8 repeat_move : 1;
    u8 move_completed : 1;
    SuperCallback c1_after_faint_check;
    u8 c1_prestate;
    struct switch_menu switch_main;

    /* Battle details */
    enum Effect effect[2];
    u8 status_state;
};

enum fight_menu { OPTION_FIGHT, OPTION_POKEMON, OPTION_BAG, OPTION_RUN };

extern struct battle_main* battle_master;
extern u8 bs_anim_status;
extern void play_bmessage(void);

#endif /* BATTLE_STATE_H_ */
