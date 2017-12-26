#include <pokeagb/pokeagb.h>
#include "moves/moves.h"
#include "battle_abilities.h"


struct ability_data abilities[] = {
	{
		// None - Example
		.before_turn = NULL,
		.before_switch = NULL,
		.before_move = NULL,
		.on_fail = NULL,
		.on_modify_move = NULL,
		.on_tryhit = NULL,
		.on_base_power = NULL,
		.on_damage = NULL,
		.on_effect = ability_none_on_effect,
		.on_after_move = NULL,
		.on_status = NULL,
	},
	{
		// Stench
		.on_damage = stench_on_damage,
	},
	{
		// DRIZZLE
		.on_start = drizzle_on_start,
	},
	{
		// SPEEDBOOST
	},
	{
		// BATTLEARMOR
	},
	{
		// STURDY
	},
	{
		// DAMP
	},
	{
		// Limber
		.on_status = limber_on_status,
	},
	{
		// SANDVEIL
	},
	{
		// STATIC
	},
	{
		// VOLTABSORB
	},
	{
		// WATERABSORB
	},
	{
		// OBLIVIOUS
	},
	{
		// CLOUDNINE
	},
	{
		// COMPOUNDEYES
	},
	{
		// Insomnia
		.on_status = insomnia_on_status,
	},
	{
		// Color Change
		.on_after_move = colorchange_on_after_move,
	},
	{
		// Immunity
		.on_status = immunity_on_status,
	},
	{
		// FLASHFIRE
	},
	{
		// SHIELDDUST
	},
	{
		// OWNTEMPO
	},
	{
		// SUCTIONCUPS
	},
	{
		// INTIMIDATE
	},
	{
		// SHADOWTAG
	},
	{
		// ROUGHSKIN
	},
	{
		// WONDERGUARD
	},
	{
		// LEVITATE
	},
	{
		// EFFECTSPORE
	},
	{
		// SYNCHRONIZE
	},
	{
		// CLEARBODY
	},
	{
		// NATURALCURE
	},
	{
		// LIGHTNINGROD
	},
	{
		// SERENEGRACE
	},
	{
		// SWIFTSWIM
	},
	{
		// CHLOROPHYLL
	},
	{
		// ILLUMINATE
	},
	{
		// TRACE
	},
	{
		// HUGEPOWER
	},
	{
		// POISONPOINT
	},
	{
		// INNERFOCUS
	},
	{
		// MAGMAARMOR
		.on_status = magmaarmor_on_status,
	},
	{
		// WATERVEIL
		.on_status = waterveil_on_status,
	},
	{
		// MAGNETPULL
	},
	{
		// SOUNDPROOF
	},
	{
		// RAINDISH
	},
	{
		// SANDSTREAM
	},
	{
		// PRESSURE
	},
	{
		// THICKFAT
	},
	{
		// EARLYBIRD
	},
	{
		// FLAMEBODY
	},
	{
		// RUNAWAY
	},
	{
		// KEENEYE
	},
	{
		// HYPERCUTTER
	},
	{
		// PICKUP
	},
	{
		// TRUANT
	},
	{
		// HUSTLE
	},
	{
		// CUTECHARM
	},
	{
		// PLUS
	},
	{
		// MINUS
	},
	{
		// FORECAST
	},
	{
		// STICKYHOLD
	},
	{
		// SHEDSKIN
	},
	{
		// GUTS
	},
	{
		// MARVELSCALE
	},
	{
		// LIQUIDOOZE
	},
	{
		// OVERGROW
	},
	{
		// BLAZE
	},
	{
		// TORRENT
	},
	{
		// SWARM
	},
	{
		// ROCKHEAD
	},
	{
		// DROUGHT
	},
	{
		// ARENATRAP
	},
	{
		// Vital Spirit
		.on_status = vitalspirit_on_status,
	},
	{
		// WHITESMOKE
	},
	{
		// PUREPOWER
	},
	{
		// SHELLARMOR
	},
	{
		// AIRLOCK
	},
	{
		// TANGLEDFEET
	},
	{
		// MOTORDRIVE
	},
	{
		// RIVALRY
	},
	{
		// STEADFAST
	},
	{
		// SNOWCLOAK
	},
	{
		// GLUTTONY
	},
	{
		// ANGERPOINT
	},
	{
		// UNBURDEN
	},
	{
		// HEATPROOF
	},
	{
		// Simple
		.on_stat_boost_mod = simple_on_stat_boost_mod,
	},
	{
		// DRYSKIN
	},
	{
		// DOWNLOAD
	},
	{
		// IRONFIST
	},
	{
		// POISONHEAL
	},
	{
		// ADAPTABILITY
	},
	{
		// SKILLLINK
	},
	{
		// HYDRATION
	},
	{
		// SOLARPOWER
	},
	{
		// QUICKFEET
	},
	{
		// NORMALIZE
	},
	{
		// SNIPER
	},
	{
		// MAGICGUARD
	},
	{
		// NOGUARD
	},
	{
		// STALL
	},
	{
		// Technician
		.on_base_power = technician_on_base_power,
	},
	{
		// LEAFGUARD
	},
	{
		// KLUTZ
	},
	{
		// MOLDBREAKER
	},
	{
		// SUPERLUCK
	},
	{
		// AFTERMATH
	},
	{
		// ANTICIPATION
	},
	{
		// FOREWARN
	},
	{
		// UNAWARE
	},
	{
		// TINTEDLENS
	},
	{
		// FILTER
	},
	{
		// SLOWSTART
	},
	{
		// SCRAPPY
	},
	{
		// STORMDRAIN
	},
	{
		// ICEBODY
	},
	{
		// SOLIDROCK
	},
	{
		// SNOWWARNING
	},
	{
		// HONEYGATHER
	},
	{
		// FRISK
	},
	{
		// RECKLESS
	},
	{
		// MULTITYPE
	},
	{
		// FLOWERGIFT
	},
	{
		// BADDREAMS
	},
	{
		// PICKPOCKET
	},
	{
		// SHEERFORCE
	},
	{
		// Contary
		.on_stat_boost_mod = contrary_on_stat_boost_mod,
	},
	{
		// UNNERVE
	},
	{
		// DEFIANT
	},
	{
		// DEFEATIST
	},
	{
		// CURSEDBODY
	},
	{
		// HEALER
	},
	{
		// FRIENDGUARD
	},
	{
		// WEAKARMOR
	},
	{
		// HEAVYMETAL
	},
	{
		// LIGHTMETAL
	},
	{
		// MULTISCALE
	},
	{
		// TOXICBOOST
	},
	{
		// FLAREBOOST
	},
	{
		// HARVEST
	},
	{
		// TELEPATHY
	},
	{
		// MOODY
	},
	{
		// OVERCOAT
	},
	{
		// POISONTOUCH
	},
	{
		// REGENERATOR
	},
	{
		// BIGPECKS
	},
	{
		// SANDRUSH
	},
	{
		// WONDERSKIN
	},
	{
		// ANALYTIC
	},
	{
		// ILLUSION
	},
	{
		// IMPOSTER
	},
	{
		// INFILTRATOR
	},
	{
		// MUMMY
	},
	{
		// MOXIE
	},
	{
		// JUSTIFIED
	},
	{
		// RATTLED
	},
	{
		// MAGICBOUNCE
	},
	{
		// SAPSIPPER
	},
	{
		// PRANKSTER
	},
	{
		// SANDFORCE
	},
	{
		// IRONBARBS
	},
	{
		// ZENMODE
	},
	{
		// VICTORYSTAR
	},
	{
		// TURBOBLAZE
	},
	{
		// TERAVOLT
	},
	{
		// AROMAVEIL
	},
	{
		// FLOWERVEIL
	},
	{
		// CHEEKPOUCH
	},
	{
		// PROTEAN
	},
	{
		// FURCOAT
	},
	{
		// MAGICIAN
	},
	{
		// BULLETPROOF
	},
	{
		// COMPETITIVE
	},
	{
		// STRONGJAW
	},
	{
		// REFRIGERATE
	},
	{
		// SWEETVEIL
	},
	{
		// STANCECHANGE
	},
	{
		// GALEWINGS
	},
	{
		// MEGALAUNCHER
	},
	{
		// GRASSPELT
	},
	{
		// SYMBIOSIS
	},
	{
		// TOUGHCLAWS
	},
	{
		// PIXILATE
	},
	{
		// GOOEY
	},
	{
		// AERILATE
	},
	{
		// PARENTALBOND
	},
	{
		// DARKAURA
	},
	{
		// FAIRYAURA
	},
	{
		// AURABREAK
	},
	{
		// PRIMORDIALSEA
	},
	{
		// DESOLATELAND
	},
	{
		// DELTASTREAM
	},
	{
		// STAMINA
	},
	{
		// WIMPOUT
	},
	{
		// EMERGENCYEXIT
	},
	{
		// WATERCOMPACTION
	},
	{
		// MERCILESS
	},
	{
		// SHIELDSDOWN
	},
	{
		// STAKEOUT
	},
	{
		// WATERBUBBLE
	},
	{
		// Steel Worker
		.on_base_power = steelworker_on_base_power,
	},
	{
		// BERSERK
	},
	{
		// SLUSHRUSH
	},
	{
		// LONGREACH
	},
	{
		// LIQUIDVOICE
	},
	{
		// TRIAGE
	},
	{
		// GALVANIZE
	},
	{
		// SURGESURFER
	},
	{
		// SCHOOLING
	},
	{
		// DISGUISE
	},
	{
		// BATTLEBOND
	},
	{
		// POWERCONSTRUCT
	},
	{
		// CORROSION
	},
	{
		// COMATOSE
	},
	{
		// QUEENLYMAJESTY
	},
	{
		// INNARDSOUT
	},
	{
		// DANCER
	},
	{
		// BATTERY
	},
	{
		// FLUFFY
	},
	{
		// DAZZLING
	},
	{
		// SOUL
	},
	{
		// TANGLINGHAIR
	},
	{
		// RECEIVER
	},
	{
		// POWEROFALCHEMY
	},
	{
		// BEASTBOOST
	},
	{
		// RKSSYSTEM
	},
	{
		// ELECTRICSURGE
	},
	{
		// PSYCHICSURGE
	},
	{
		// MISTYSURGE
	},
	{
		// GRASSYSURGE
	},
	{
		// FULLMETAL
	},
	{
		// SHADOWSHIELD
	},
	{
		// PRISMARMOR
	},
	{
		// NEUROFORCE
	},
};
