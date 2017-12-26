#include <pokeagb/pokeagb.h>
#include "moves/moves.h"
#include "battle_abilities.h"
#include "../battle_data/pkmn_bank_stats.h"
#include "../battle_data/pkmn_bank.h"

extern void dprintf(const char * str, ...);
extern bool enqueue_message(u16 move, u8 bank, enum battle_string_ids id, u16 effect);
extern bool set_weather(enum WeatherTypes weather);

// None
u8 ability_none_on_effect(u8 user, u8 source, u16 move, struct anonymous_callback* acb)
{
    // This is just an example
    return true;
}

// Stench
void stench_on_damage(u8 user, u8 source,  struct anonymous_callback* acb)
{
    if (user != source) return;
    if(B_MOVE_DMG(user) > 0) {
       B_MOVE_FLINCH(user) = 10;
    }
    return;
}

// Drizzle [We need .on_start for this]
void drizzle_on_start(u8 user, u8 src, u16 move, struct anonymous_callback* acb)
{
    if (HAS_VOLATILE(VOLATILE_DRIZZLE, src)) return;
    ADD_VOLATILE(VOLATILE_DRIZZLE, src);
    if (battle_master->field_state.is_raining) return;
    set_weather(WEATHER_RAIN);
}

// SPEED BOOST

// BATTLEARMOR

// STURDY

// DAMP

// Limber
u8 limber_on_status(u8 user, u8 source, u16 ailment , struct anonymous_callback* acb)
{
    if (user != source) return true;
    if (ailment == AILMENT_PARALYZE) {
    	enqueue_message(NULL, user, STRING_IMMUNE_ABILITY, NULL);
       	return false;
    }
    return true;
}

// SANDVEIL

// STATIC

// VOLTABSORB

// WATERABSORB

// OBLIVIOUS

// CLOUDNINE

// COMPOUNDEYES

// Insomnia
u8 insomnia_on_status(u8 user, u8 source, u16 ailment , struct anonymous_callback* acb)
{
    if (user != source) return true;
    if (ailment == AILMENT_SLEEP) {
    	enqueue_message(NULL, user, STRING_IMMUNE_ABILITY, NULL);
    	return false;
    }
    return true;
}

// Color Change
void colorchange_on_after_move(u8 user, u8 source, u16 move, struct anonymous_callback* acb)
{
    if (user != source) return;
    if( B_MOVE_DMG(user) > 0 && !B_MOVE_HAS_TYPE(user, MTYPE_NONE)) {
	u8 type = B_MOVE_TYPE(TARGET_OF(user), 0);
	b_pkmn_set_type(TARGET_OF(user), type);
	enqueue_message(NULL, user, STRING_CONVERSION_TYPE, type);
    }
    return;
}

// Immunity
u8 immunity_on_status(u8 user, u8 source, u16 ailment , struct anonymous_callback* acb)
{
    if (user != source) return true;
    if (ailment == AILMENT_POISON) {
    	enqueue_message(NULL, user, STRING_IMMUNE_ABILITY, NULL);
    	return false;
    }
    return true;
}

// FLASHFIRE

// SHIELDDUST

// OWNTEMPO

// SUCTIONCUPS

// INTIMIDATE

// SHADOWTAG

// ROUGHSKIN

// WONDERGUARD

// LEVITATE

// EFFECTSPORE

// SYNCHRONIZE

// CLEARBODY

// NATURALCURE

// LIGHTNINGROD

// SERENEGRACE

// SWIFTSWIM

// CHLOROPHYLL

// ILLUMINATE

// TRACE

// HUGEPOWER

// POISONPOINT

// INNERFOCUS

// Magma Armor
u8 magmaarmor_on_status(u8 user, u8 source, u16 ailment , struct anonymous_callback* acb)
{
    if (user != source) return true;
    if (ailment == AILMENT_FREEZE) {
    	enqueue_message(NULL, user, STRING_IMMUNE_ABILITY, NULL);
    	return false;
    }
    return true;
}

// Water Veil
u8 waterveil_on_status(u8 user, u8 source, u16 ailment , struct anonymous_callback* acb)
{
    if (user != source) return true;
    if (ailment == AILMENT_BURN) {
    	enqueue_message(NULL, user, STRING_IMMUNE_ABILITY, NULL);
    	return false;
    }
    return true;
}

// MAGNETPULL

// SOUNDPROOF

// RAINDISH

// SANDSTREAM

// PRESSURE

// THICKFAT

// EARLYBIRD

// FLAMEBODY

// RUNAWAY

// KEENEYE

// HYPERCUTTER

// PICKUP

// TRUANT

// HUSTLE

// CUTECHARM

// PLUS

// MINUS

// FORECAST

// STICKYHOLD

// SHEDSKIN

// GUTS

// MARVELSCALE

// LIQUIDOOZE

// OVERGROW

// BLAZE

// TORRENT

// SWARM

// ROCKHEAD

// DROUGHT

// ARENATRAP

// Vital Spirit
u8 vitalspirit_on_status(u8 user, u8 source, u16 ailment , struct anonymous_callback* acb)
{
    if (user != source) return true;
    if (ailment == AILMENT_SLEEP) {
    	enqueue_message(NULL, user, STRING_IMMUNE_ABILITY, NULL);
    	return false;
    }
    return true;
}

// WHITESMOKE

// PUREPOWER

// SHELLARMOR

// AIRLOCK

// TANGLEDFEET

// MOTORDRIVE

// RIVALRY

// STEADFAST

// SNOWCLOAK

// GLUTTONY

// ANGERPOINT

// UNBURDEN

// HEATPROOF

// Simple
u8 simple_on_stat_boost_mod(u8 user, u8 source, u16 stat_id, struct anonymous_callback* acb)
{
    if (user != source) return 0;
    acb->in_use = false;
    return (acb->data_ptr + ((acb->data_ptr) & 0x7));
}

// DRYSKIN

// DOWNLOAD

// IRONFIST

// POISONHEAL

// ADAPTABILITY

// SKILLLINK

// HYDRATION

// SOLARPOWER

// QUICKFEET

// NORMALIZE

// SNIPER

// MAGICGUARD

// NOGUARD

// STALL

// Technician
void technician_on_base_power(u8 user, u8 source, struct anonymous_callback* acb)
{
    if (user != source) return;
	if (B_MOVE_POWER(user) <= 60) {
	   B_MOVE_POWER(user) = NUM_MOD(B_MOVE_POWER(user), 150);
	}
	return;
}

// LEAFGUARD

// KLUTZ

// MOLDBREAKER

// SUPERLUCK

// AFTERMATH

// ANTICIPATION

// FOREWARN

// UNAWARE

// TINTEDLENS

// FILTER

// SLOWSTART

// SCRAPPY

// STORMDRAIN

// ICEBODY

// SOLIDROCK

// SNOWWARNING

// HONEYGATHER

// FRISK

// RECKLESS

// MULTITYPE

// FLOWERGIFT

// BADDREAMS

// PICKPOCKET

// SHEERFORCE

// Contary
u8 contrary_on_stat_boost_mod(u8 user, u8 source, u16 stat_id, struct anonymous_callback* acb)
{
    if (user != source) return 0;
    acb->in_use = false;
    return ((acb->data_ptr) ^ 16);
}

// UNNERVE

// DEFIANT

// DEFEATIST

// CURSEDBODY

// HEALER

// FRIENDGUARD

// WEAKARMOR

// HEAVYMETAL

// LIGHTMETAL

// MULTISCALE

// TOXICBOOST

// FLAREBOOST

// HARVEST

// TELEPATHY

// MOODY

// OVERCOAT

// POISONTOUCH

// REGENERATOR

// BIGPECKS

// SANDRUSH

// WONDERSKIN

// ANALYTIC

// ILLUSION

// IMPOSTER

// INFILTRATOR

// MUMMY

// MOXIE

// JUSTIFIED

// RATTLED

// MAGICBOUNCE

// SAPSIPPER

// PRANKSTER

// SANDFORCE

// IRONBARBS

// ZENMODE

// VICTORYSTAR

// TURBOBLAZE

// TERAVOLT

// AROMAVEIL

// FLOWERVEIL

// CHEEKPOUCH

// PROTEAN

// FURCOAT

// MAGICIAN

// BULLETPROOF

// COMPETITIVE

// STRONGJAW

// REFRIGERATE

// SWEETVEIL

// STANCECHANGE

// GALEWINGS

// MEGALAUNCHER

// GRASSPELT

// SYMBIOSIS

// TOUGHCLAWS

// PIXILATE

// GOOEY

// AERILATE

// PARENTALBOND

// DARKAURA

// FAIRYAURA

// AURABREAK

// PRIMORDIALSEA

// DESOLATELAND

// DELTASTREAM

// STAMINA

// WIMPOUT

// EMERGENCYEXIT

// WATERCOMPACTION

// MERCILESS

// SHIELDSDOWN

// STAKEOUT

// WATERBUBBLE

// Steel Worker
void steelworker_on_base_power(u8 user, u8 source, u16 move, struct anonymous_callback* acb)
{
    if (user != source) return;
	if(B_MOVE_HAS_TYPE(user, MTYPE_STEEL)) {
	   B_MOVE_POWER(user) = NUM_MOD(B_MOVE_POWER(user), 150);
	}
	return;
}

// BERSERK

// SLUSHRUSH

// LONGREACH

// LIQUIDVOICE

// TRIAGE

// GALVANIZE

// SURGESURFER

// SCHOOLING

// DISGUISE

// BATTLEBOND

// POWERCONSTRUCT

// CORROSION

// COMATOSE

// QUEENLYMAJESTY

// INNARDSOUT

// DANCER

// BATTERY

// FLUFFY

// DAZZLING

// SOUL

// TANGLINGHAIR

// RECEIVER

// POWEROFALCHEMY

// BEASTBOOST

// RKSSYSTEM

// ELECTRICSURGE

// PSYCHICSURGE

// MISTYSURGE

// GRASSYSURGE

// FULLMETAL

// SHADOWSHIELD

// PRISMARMOR

// NEUROFORCE
