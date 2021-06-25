#define SLIDER_W       15
#define BOX_Y          0
#define BOX_X          0
#define BOX_LINE_H     LINE_H
#define BOX_H          (LCD_H-BOX_Y)
#define BOX_W          (LCD_W-SLIDER_W-2)
#define BOX_LINES      (BOX_H / BOX_LINE_H)
#define BOX_RECT       (gslc_tsRect){BOX_X, BOX_Y, BOX_W, BOX_H}
#define SLIDER_RECT    (gslc_tsRect){LCD_W - SLIDER_W, BOX_Y, SLIDER_W, BOX_H}


#define SLIDER_COL     COL_WHITE

#define FILES_BACK_COL       COL_BLACK

#define FILES_COL_NORMAL_B   COL_BLACK
#define FILES_COL_NORMAL_F   COL_WHITE
#define FILES_COL_PLAY_B     COL_RED_DARK
#define FILES_COL_PLAY_F     COL_RED_LIGHT
#define FILES_COL_DIR_B      COL_GREEN_DARK
#define FILES_COL_DIR_F      COL_GREEN_LIGHT

void page_files_init();
