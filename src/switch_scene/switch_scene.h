#ifndef SWITCH_SCENE_H_
#define SWITCH_SCENE_H_

// tags
#define SLIDER_PAL_TAG 0xD740
#define SLIDER_GFX_TAG 0xD740
#define ICON_PAL_TAG 0xDAC0
#define ICON_GFX_TAG 0xD75A

// positions
#define Y_POS_MID_ONE 38
#define Y_POS_MID_TWO 62
#define Y_POS_MID_THREE 87
#define Y_POS_MID_FOUR 110

// text palette related
#define rgb5(r, g, b) (u16)((r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10))
extern bool nature_stat_nerved(u8 stat, struct Pokemon *pokemon);
extern bool nature_stat_boosted(u8 stat, struct Pokemon *pokemon);
#define STAT_COLOR(stat, pkmn) \
    (nature_stat_boosted(stat, pkmn) ? &switch_color_green : (nature_stat_nerved(stat, pkmn) ? &switch_color_red : &switch_color))

// Text table indexes for strings
#define SWB_ABILITY 0
#define SWB_ITEM 1
#define SWB_ATK 2
#define SWB_DEF 3
#define SWB_SPA 4
#define SWB_SPD 5
#define SWB_SPE 6
#define SWB_MOVES 7
#define SWB_POW 8
#define SWB_ACC 9
#define SWB_PP 10
#define SWB_NAME 11

#define SWM_LOG (battle_master->switch_main.sd)


// text related declarations, see switch_text.c
extern const pchar str_no_item[];
extern const pchar str_invalid_num[];
extern u16 switch_text_pal[];
extern struct TextColor switch_color;
extern struct TextColor switch_color_bg;
extern struct TextColor switch_color_red;
extern struct TextColor switch_color_green;
extern struct TextboxTemplate switch_boxes[];


extern u8 load_dmg_type_icon(u8 type, s16 x, s16 y, u8 tag);
extern u8 load_dmg_category_icon(u8 category, s16 x, s16 y, u8 tag);
extern void vblank_cb_no_merge(void);
extern void c2_switch_menu(void);
extern void oac_nullsub(struct Object* obj);

#endif /* SWITCH_SCENE_H_ */
