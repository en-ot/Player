//#define LINE_H 20 //font 18
#define LINE_H 28 //font 24


//###############################################################
#define COL_BLACK       ((gslc_tsColor) { 0, 0, 0})
#define COL_GRAY_DARK   ((gslc_tsColor) { 32, 32, 32})
#define COL_GRAY        ((gslc_tsColor) { 64, 64, 64})
#define COL_WHITE       ((gslc_tsColor) { 255, 255, 255})

#define COL_GREEN_DARK  ((gslc_tsColor) { 0, 96, 0})
#define COL_GREEN       ((gslc_tsColor) { 0, 192, 0})
#define COL_GREEN_LIGHT ((gslc_tsColor) { 0, 255, 0})

#define COL_RED_DARK    ((gslc_tsColor) { 96, 0, 0})
#define COL_RED         ((gslc_tsColor) { 192, 0, 0})
#define COL_RED_LIGHT   ((gslc_tsColor) { 255, 0, 0})

#define COL_BLUE_DARK   ((gslc_tsColor) { 0, 0, 96})
#define COL_BLUE        ((gslc_tsColor) { 0, 0, 192})
#define COL_BLUE_LIGHT  ((gslc_tsColor) { 0, 0, 255})

#define COL_FRAME       COL_WHITE
#define COL_TEXT_NORMAL COL_WHITE
#define COL_ERROR       ((gslc_tsColor) { 255, 0, 255})

enum {
    FONT_BUILTIN5X8, FONT_BUILTIN20x32, FONT_SMOOTH,
    FONT_MAX
};

#define LCD_H ((int16_t)240)
#define LCD_W ((int16_t)320)

enum {
    //info
    INFO_PLAY_ICON, INFO_SHUFFLE_ICON, INFO_REPEAT_ICON, INFO_VOLUME_ICON, INFO_FAV_ICON,
    INFO_VOLUME_ELEM, INFO_FAV_ELEM, INFO_INDEX_ELEM, INFO_PGS1_ELEM, INFO_PGS2_ELEM, INFO_PGS3_ELEM, 
    INFO_PATH_ELEM, INFO_FILE_ELEM, INFO_BAND_ELEM, INFO_ARTIST_ELEM, INFO_ALBUM_ELEM, INFO_TITLE_ELEM,
    INFO_PATH_ICON, INFO_FILE_ICON, INFO_BAND_ICON, INFO_ARTIST_ICON, INFO_ALBUM_ICON, INFO_TITLE_ICON,

    //files
    FILES_BOX_ELEM, FILES_SLIDER_ELEM, 
/*
    //pic
    PIC_PIC_ELEM, PIC_NAME_ELEM, 
*/
    //fav
    FAV_BOX_ELEM, FAV_SLIDER_ELEM,

    //dirs
    DIRS_BOX_ELEM, DIRS_SLIDER_ELEM,

    //system
    SYS_MEM_ELEM,

    GUI_ELEM_MAX
};

#define INFO_ELEM_MAX     (FILES_BOX_ELEM  -INFO_PLAY_ICON)
#define FILES_ELEM_MAX     (FAV_BOX_ELEM   -FILES_BOX_ELEM)
//#define FILES_ELEM_MAX     (PIC_PIC_ELEM   -FILES_BOX_ELEM)
//#define PIC_ELEM_MAX      (FAV_BOX_ELEM   -PIC_PIC_ELEM)
#define FAV_ELEM_MAX      (DIRS_BOX_ELEM  -FAV_BOX_ELEM)
#define DIRS_ELEM_MAX     (SYS_MEM_ELEM   -DIRS_BOX_ELEM)
#define SYS_ELEM_MAX      (GUI_ELEM_MAX   -SYS_MEM_ELEM)

#define INFO_MODE_ICONS     INFO_VOLUME_ELEM-INFO_PLAY_ICON

#define SLIDER_POS_MAX 240

#define COL_SEL        COL_BLUE
