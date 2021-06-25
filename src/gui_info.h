#define INFO_BACK_COL       COL_BLACK

#define INFO_ICON_W LINE_H
#define INFO_ICON_H LINE_H
#define INFO_GAP 2

#define INFO_MODE_COL       COL_GREEN_DARK
#define INFO_MODE_ICON_RECT (gslc_tsRect){x, 0, INFO_ICON_W, INFO_ICON_H}

#define INFO_VOLUME_W 27
#define INFO_VOLUME_RECT    (gslc_tsRect){x, 0, INFO_VOLUME_W, LINE_H}

#define INFO_FAV_W 27
#define INFO_FAV_RECT       (gslc_tsRect){x, 0, INFO_FAV_W, LINE_H}

#define INFO_INDEX_RECT     (gslc_tsRect){x, 0, (uint16_t)(LCD_W-x), LINE_H}
#define INFO_INDEX_COL      COL_GREEN_DARK

#define INFO_ICON_W         LINE_H
#define INFO_ICON_H         LINE_H

#if (LINE_H==20)
#define INFO_LINE_Y         (LINE_H * 2)
#define INFO_LINE_STEP      (LINE_H + 3)
#elif (LINE_H==28)
#define INFO_LINE_Y         (LINE_H + 3)
#define INFO_LINE_STEP      (LINE_H + 2)
#endif
#define INFO_LINE_RECT      (gslc_tsRect){INFO_ICON_W, y, LCD_W-INFO_ICON_W, LINE_H}

#define INFO_ICON_RECT      (gslc_tsRect){0, y, INFO_ICON_W, INFO_ICON_H}

#define INFO_PGS1_W         60
#define INFO_PGS3_W         60
#define INFO_PGS2_W         (LCD_W-INFO_PGS1_W-INFO_PGS3_W)
#define INFO_PGS2_MAX       (INFO_PGS2_W-1)
#define INFO_PGS1_RECT      (gslc_tsRect){0, LCD_H-LINE_H, INFO_PGS1_W, LINE_H}
#define INFO_PGS2_RECT      (gslc_tsRect){INFO_PGS1_W, LCD_H-LINE_H, INFO_PGS2_W, LINE_H}
#define INFO_PGS3_RECT      (gslc_tsRect){LCD_W-INFO_PGS3_W, LCD_H-LINE_H, INFO_PGS3_W, LINE_H}
#define INFO_PGS_FILL_COL   COL_GREEN_DARK
#define INFO_PGS2_FILL_COL  COL_GRAY
#define INFO_PGS2_LINE_COL  COL_GREEN
#define INFO_PGS2_FRAME_COL COL_WHITE

enum {
    INFO_PATH, INFO_FILE, INFO_BAND, INFO_ARTIST, INFO_ALBUM, INFO_TITLE, 
    INFO_LINES
};

#define SCROLL_DELAY 2000
#define SCROLL_PERIOD 200
#define SCROLL_STEP 15

void page_info_init();
