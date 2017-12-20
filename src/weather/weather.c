#include <pokeagb/pokeagb.h>
#include "../battle_data/pkmn_bank.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/battle_state.h"

extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern void clear_other_weather(void);
extern void dprintf(const char * str, ...);
extern bool b_pkmn_has_type(u8 bank, enum PokemonType type);
extern void do_damage(u8 bank_index, u16 dmg);
extern bool b_pkmn_pure_type(u8 bank, enum PokemonType type);

/* Rain */
u16 rain_dmg_mod(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if ((battle_master->field_state.is_raining) && (B_MOVE_HAS_TYPE(user, MTYPE_WATER))) {
        return 150;
    } else if (B_MOVE_HAS_TYPE(user, MTYPE_FIRE)) {
        return 50;
    } else {
        return 100;
    }
}

u16 rain_on_residual(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (acb->data_ptr) {
        acb->data_ptr = 0;
    } else {
        acb->data_ptr = 1;
        return true;
    }
    if (acb->duration == 0) {
        enqueue_message(NULL, NULL, STRING_RAIN_STOPPED, MOVE_RAIN_DANCE);
    } else {
        enqueue_message(NULL, NULL, STRING_RAIN_FALLING, MOVE_RAIN_DANCE);
    }
    return true;
}

void rain_init_effect()
{
    battle_master->field_state.is_raining = true;
    add_callback(CB_ON_RESIDUAL, 1, 5, NULL, (u32)rain_on_residual);
    add_callback(CB_ON_WEATHER_DMG, 0, 5, 0, (u32)rain_dmg_mod);
    enqueue_message(0, 0, STRING_RAINING, MOVE_RAIN_DANCE);
    return true;
}

void clear_rain()
{
    battle_master->field_state.is_raining = false;
    delete_callback((u32)rain_dmg_mod);
    delete_callback((u32)rain_on_residual);
}


/* Primordial sea */
u16 primordial_sea_dmg_mod(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    // fire type failing handled in tryhit
    if ((battle_master->field_state.is_primordial_sea) && (B_MOVE_HAS_TYPE(user, MTYPE_WATER))) {
        return 150;
    } else {
        return 100;
    }
}

u8 primordial_sea_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (B_MOVE_HAS_TYPE(user, MTYPE_FIRE)) {
        enqueue_message(0, 0, STRING_HEAVY_RAIN_FIZZLE, 0);
        return 3;
    }
    return true;
}

void primordial_sea_end()
{
    u8 id = id_by_func((u32)primordial_sea_dmg_mod);
    CB_MASTER[id].in_use = false;
    id = id_by_func((u32)primordial_sea_tryhit);
    CB_MASTER[id].in_use = false;

}

void primordial_sea_init_effect()
{
    battle_master->field_state.is_primordial_sea = true;
    add_callback(CB_ON_WEATHER_DMG, 0, 0xFF, NULL, (u32)primordial_sea_dmg_mod);
    add_callback(CB_ON_TRYHIT_MOVE, 0, 0xFF, NULL, (u32)primordial_sea_tryhit);
    enqueue_message(0, 0, STRING_PRIMORDIAL_SEA, 0);
}

void clear_primordial_sea()
{
    if (battle_master->field_state.is_primordial_sea) {
        enqueue_message(0, 0, STRING_HEAVY_RAIN_LIFTED, 0);
        delete_callback((u32)primordial_sea_dmg_mod);
        delete_callback((u32)primordial_sea_tryhit);
    }
}


/* Sun */
u16 sun_dmg_mod(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (B_MOVE_HAS_TYPE(user, MTYPE_FIRE)) {
        return 150;
    } else if (B_MOVE_HAS_TYPE(user, MTYPE_WATER)) {
        return 50;
    } else {
        return 100;
    }
}

u16 sun_on_residual(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (acb->data_ptr) {
        acb->data_ptr = 0;
    } else {
        acb->data_ptr = 1;
        return true;
    }
    if (acb->duration == 0) {
        enqueue_message(NULL, NULL, STRING_SUNLIGHT_FADE, NULL);
    } else {
        enqueue_message(NULL, NULL, STRING_SUNLIGHT_STRONG, NULL);
    }
    return true;
}

void sun_init_effect()
{
    battle_master->field_state.is_sunny = true;
    add_callback(CB_ON_WEATHER_DMG, 1, 5, NULL, (u32)sun_dmg_mod);
    add_callback(CB_ON_RESIDUAL, 1, 5, NULL, (u32)sun_on_residual);
    enqueue_message(0, 0, STRING_SUNLIGHT_HARSH, 0);
}

void clear_sun()
{
    battle_master->field_state.is_sunny = false;
    delete_callback((u32)sun_dmg_mod);
    delete_callback((u32)sun_on_residual);
}


/* Desolate land */
u16 desolate_land_dmg_mod(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    // fire type failing handled in tryhit
    if ((battle_master->field_state.is_desolate_land) && (B_MOVE_HAS_TYPE(user, MTYPE_FIRE))) {
        return 150;
    } else {
        return 100;
    }
}

u8 desolate_land_tryhit(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (B_MOVE_HAS_TYPE(user, MTYPE_WATER)) {
        enqueue_message(0, 0, STRING_HARSH_SUN_WATER, 0);
        return 3;
    }
    return true;
}

void desolate_land_init_effect()
{
    clear_other_weather();
    battle_master->field_state.is_desolate_land = true;
    add_callback(CB_ON_WEATHER_DMG, 0, 0xFF, NULL, (u32)desolate_land_dmg_mod);
    add_callback(CB_ON_TRYHIT_MOVE, 0, 0xFF, NULL, (u32)desolate_land_tryhit);
    enqueue_message(0, 0, STRING_DESOLATE_LAND, 0);
}

void clear_desolate_land()
{
    if (battle_master->field_state.is_desolate_land) {
        enqueue_message(0, 0, STRING_HARSH_SUN_END, 0);
        delete_callback((u32)desolate_land_dmg_mod);
        delete_callback((u32)desolate_land_tryhit);
    }
}


/* Sandstorm */
u16 sandstorm_stat_mod(u8 user, u8 src, u16 stat_id, struct anonymous_callback* acb)
{
    if (stat_id != SPDEFENSE_MOD) return (u32)acb->data_ptr;
    if (b_pkmn_has_type(user, MTYPE_ROCK)) {
        return NUM_MOD((u32)acb->data_ptr, 150);
    } else {
        return (u32)acb->data_ptr;
    }
}


u16 sandstorm_on_residual_buffet(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (acb->data_ptr) {
        acb->data_ptr = 0;
    } else {
        acb->data_ptr = 1;
    }
    if (acb->duration == 0) {
        if (acb->data_ptr)
            enqueue_message(NULL, NULL, STRING_SANDSTORM_END, NULL);
    } else {
        if (acb->data_ptr)
            enqueue_message(NULL, NULL, STRING_SANDSTORM_RAGE, NULL);
        if (b_pkmn_has_type(user, MTYPE_ROCK) || b_pkmn_has_type(user, MTYPE_STEEL) || b_pkmn_has_type(user, MTYPE_GROUND))
            return true;
        if (HAS_VOLATILE(user, VOLATILE_DIVE) || HAS_VOLATILE(user, VOLATILE_DIG))
            return true;
        enqueue_message(NULL, user, STRING_SANDSTORM_BUFFET, MOVE_SANDSTORM);
        do_damage(user, (TOTAL_HP(user) / 16));
    }
    return true;
}

void sandstorm_init_effect()
{
    battle_master->field_state.is_sandstorm = true;
    add_callback(CB_ON_RESIDUAL, 1, 5, NULL, (u32)sandstorm_on_residual_buffet);
    add_callback(CB_ON_STAT_MOD, 10, 5, NULL, (u32)sandstorm_stat_mod);
    enqueue_message(NULL, NULL, STRING_SANDSTORM_KICKED, NULL);
}

void clear_sandstorm()
{
    battle_master->field_state.is_sandstorm = false;
    delete_callback((u32)sandstorm_on_residual_buffet);
    delete_callback((u32)sandstorm_stat_mod);
}


/* hail */
u16 hail_on_residual_buffet(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (acb->data_ptr) {
        acb->data_ptr = 0;
    } else {
        acb->data_ptr = 1;
    }
    if (acb->duration == 0) {
        if (acb->data_ptr)
            enqueue_message(NULL, NULL, STRING_RAIN_FALLING, MOVE_HAIL);
    } else {
        if (acb->data_ptr)
            enqueue_message(NULL, NULL, STRING_RAIN_STOPPED, MOVE_HAIL);
        if (b_pkmn_has_type(user, MTYPE_ICE))
            return true;
        if (HAS_VOLATILE(user, VOLATILE_DIVE) || HAS_VOLATILE(user, VOLATILE_DIG))
            return true;
        enqueue_message(NULL, user, STRING_SANDSTORM_BUFFET, MOVE_HAIL);
        do_damage(user, (TOTAL_HP(user) / 16));
    }
    return true;
}

void hail_init_effect()
{
    battle_master->field_state.is_hail = true;
    add_callback(CB_ON_RESIDUAL, 1, 5, NULL, (u32)hail_on_residual_buffet);
    enqueue_message(0, 0, STRING_RAINING, MOVE_HAIL);
}

void clear_hail()
{
    battle_master->field_state.is_hail = false;
    delete_callback((u32)hail_on_residual_buffet);
}


/* Delta stream */
u16 delta_stream_on_effect(u8 target_type, u8 src, u16 move_type, struct anonymous_callback* acb)
{
    enqueue_message(0, 0, STRING_DELTA_STREAM_EFFECT, 0);
    acb->in_use = false;
    return true;
}

u16 delta_stream_effectiveness(u8 target_type, u8 src, u16 move_type, struct anonymous_callback* acb)
{
    u16 effectiveness = ((acb->data_ptr << 16) >> 16);
    if ((target_type == MTYPE_FLYING) && (effectiveness > 100)) {
        if (!callback_exists((u32)delta_stream_on_effect))
            add_callback(CB_ON_EFFECT, 101, 0, NULL, (u32)delta_stream_on_effect);
        return 100;
    } else {
        return effectiveness;
    }
}

void delta_stream_init_effect()
{
    battle_master->field_state.is_delta_stream = true;
    add_callback(CB_ON_EFFECTIVENESS, 0, 0xFF, NULL, (u32)delta_stream_effectiveness);
    enqueue_message(0, 0, STRING_DELTA_STREAM, NULL);
}

void clear_dela_stream()
{
    if (battle_master->field_state.is_delta_stream) {
        delete_callback((u32)delta_stream_effectiveness);
        delete_callback((u32)delta_stream_on_effect);
        battle_master->field_state.is_delta_stream = false;
        enqueue_message(0, 0, STRING_DELTA_STREAM_END, 0);
    }
}

/* Setting and clearing weather done using below funcs */
bool check_alpha_weather()
{
    if (battle_master->field_state.is_primordial_sea) {
        enqueue_message(0, 0, STRING_PRIMORDIAL_SEA_IMM, 0);
        return true;
    } else if (battle_master->field_state.is_desolate_land) {
        enqueue_message(0, 0, STRING_DESOLATE_LAND_IMM, 0);
        return true;
    } else if (battle_master->field_state.is_delta_stream) {
        enqueue_message(0, 0, STRING_DELTA_STREAM_IMM, 0);
        return true;
    } else {
        return false;
    }
}

void clear_other_weather()
{
    // todo
    battle_master->field_state.is_raining = false;
    battle_master->field_state.is_sunny = false;
    battle_master->field_state.is_sandstorm = false;
    battle_master->field_state.is_hail = false;
    battle_master->field_state.is_desolate_land = false;
    battle_master->field_state.is_primordial_sea = false;
    battle_master->field_state.is_delta_stream = false;
    battle_master->field_state.suppress_weather = false;
    clear_rain();
    clear_sun();
    clear_sandstorm();
    clear_hail();
    clear_desolate_land();
    clear_primordial_sea();
    clear_dela_stream();
}

bool set_weather(enum WeatherTypes weather)
{
    if (weather < WEATHER_HARSH_SUN) {
        if (check_alpha_weather())
            return false;
    }
    clear_other_weather();
    switch (weather) {
        case WEATHER_RAIN:
            rain_init_effect();
            break;
        case WEATHER_SUN:
            sun_init_effect();
            break;
        case WEATHER_SANDSTORM:
            sandstorm_init_effect();
            break;
        case WEATHER_HAIL:
            hail_init_effect();
            break;
        case WEATHER_HARSH_SUN:
            clear_other_weather();
            desolate_land_init_effect();
            break;
        case WEATHER_HARSH_RAIN:
            clear_other_weather();
            primordial_sea_init_effect();
            break;
        case WEATHER_MYSTERIOUS_AIR_CURRENT:
            clear_other_weather();
            delta_stream_init_effect();
            break;
        default:
            return false;
    };
    return true;
}


enum WeatherTypes get_weather()
{
    if (battle_master->field_state.is_raining) return WEATHER_RAIN;
    if (battle_master->field_state.is_sunny) return WEATHER_SUN;
    if (battle_master->field_state.is_sandstorm) return WEATHER_SANDSTORM;
    if (battle_master->field_state.is_hail) return WEATHER_HAIL;
    if (battle_master->field_state.is_desolate_land) return WEATHER_HARSH_SUN;
    if (battle_master->field_state.is_primordial_sea) return WEATHER_HARSH_RAIN;
    if (battle_master->field_state.is_delta_stream) return WEATHER_MYSTERIOUS_AIR_CURRENT;
    if (battle_master->field_state.suppress_weather) return WEATHER_CLEAR;
    return WEATHER_CLEAR;
}
