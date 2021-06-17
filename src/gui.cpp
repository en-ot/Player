#include "pinout.h"

#include <TFT_eSPI.h>      // Hardware-specific library

#include "GUIslice.h"
#include "GUIslice_drv.h"
#include "elem/XProgress.h"
#include "elem/XSlider.h"
//#include "elem/XTextbox.h"
#include "elem/XListbox.h"

#include "gui.h"


//#define SPIFFS_FONT

//#define FONTSIZE 18
#define FONTSIZE 24

//#define LINE_H 20 //font 18
#define LINE_H 28 //font 24

#if (FONTSIZE==18)
#ifdef SPIFFS_FONT
#define FONT_NAME1 "x318"
#else
#define FONT_NAME1 x318
#include "Fonts/x318.h"
#endif

#elif (FONTSIZE==24)
#ifdef SPIFFS_FONT
#define FONT_NAME1 "x324"
#else
#define FONT_NAME1 x324
#include "Fonts/x324.h"
#endif

#endif


//###############################################################
#define COL_BLACK       ((gslc_tsColor) { 0, 0, 0})
#define COL_GRAY_DARK   ((gslc_tsColor) { 32, 32, 32})
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

#define COL_FRAME           COL_WHITE
#define COL_TEXT_NORMAL     COL_WHITE
#define COL_ERROR       ((gslc_tsColor) { 255, 0, 255})


//###############################################################
extern TFT_eSPI m_disp;
TFT_eSPI &tft = m_disp;
static int16_t DebugOut(char ch) { Serial.write(ch); return 0; }

enum {
    FONT_BUILTIN5X8, FONT_BUILTIN20x32, FONT_SMOOTH,
    FONT_MAX
};

#define LCD_H 240
#define LCD_W 320

enum {
    //info
    INFO_MODE_ELEM, INFO_INDEX_ELEM, INFO_PGS1_ELEM, INFO_PGS2_ELEM, INFO_PGS3_ELEM, 
    INFO_PATH_ELEM, INFO_FILE_ELEM, INFO_BAND_ELEM, INFO_ARTIST_ELEM, INFO_ALBUM_ELEM, INFO_TITLE_ELEM,

    //list
    LIST_BOX_ELEM, LIST_SLIDER_ELEM, 
/*
    //pic
    PIC_PIC_ELEM, PIC_NAME_ELEM, 
*/
    //fav
    FAV_BOX_ELEM, FAV_SLIDER_ELEM,

    //dirs
    DIRS_BOX_ELEM, DIRS_SLIDER_ELEM,

    GUI_ELEM_MAX
};

#define INFO_ELEM_MAX     (LIST_BOX_ELEM  -INFO_MODE_ELEM)
#define LIST_ELEM_MAX     (FAV_BOX_ELEM   -LIST_BOX_ELEM)
//#define LIST_ELEM_MAX     (PIC_PIC_ELEM   -LIST_BOX_ELEM)
//#define PIC_ELEM_MAX      (FAV_BOX_ELEM   -PIC_PIC_ELEM)
#define FAV_ELEM_MAX      (DIRS_BOX_ELEM  -FAV_BOX_ELEM)
#define DIRS_ELEM_MAX     (GUI_ELEM_MAX   -DIRS_BOX_ELEM)

gslc_tsGui                  gslc;
gslc_tsDriver               m_drv;
gslc_tsFont                 fonts[FONT_MAX];
gslc_tsPage                 pages[PAGE_MAX];


void gslc_init()
{
    gslc_InitDebug(&DebugOut);

    if (!gslc_Init(&gslc,&m_drv,pages, PAGE_MAX, fonts, FONT_MAX))
    {
        Serial.println("gslc init error");
        return;
    }

    if (!gslc_FontSet(&gslc,FONT_BUILTIN20x32,GSLC_FONTREF_PTR,NULL,4))
    {
        Serial.println("gslc fontset20 error");
        return;
    }

    if (!gslc_FontSet(&gslc,FONT_BUILTIN5X8,GSLC_FONTREF_PTR,NULL,1))
    {
        Serial.println("gslc fontset5 error");
        return;
    }

#ifdef SPIFFS_FONT
    if (!gslc_FontSet(&gslc,FONT_SMOOTH,GSLC_FONTREF_FNAME,FONT_NAME1,1))
#else
    if (!gslc_FontSet(&gslc,FONT_SMOOTH,GSLC_FONTREF_PTR,FONT_NAME1,1))
#endif
    {
        Serial.println("gslc fontset2 error");
        return;
    }
}


//###############################################################
#define SLIDER_W       15
#define BOX_Y          0
#define BOX_X          0
#define BOX_LINE_H     LINE_H
#define BOX_H          (LCD_H-BOX_Y)
#define BOX_W          (LCD_W-SLIDER_W-2)
#define BOX_LINES      (BOX_H / BOX_LINE_H)
#define BOX_RECT       (gslc_tsRect){BOX_X, BOX_Y, BOX_W, BOX_H}
#define SLIDER_RECT    (gslc_tsRect){LCD_W - SLIDER_W, BOX_Y, SLIDER_W, BOX_H}

#define COL_SEL        COL_BLUE

#define SLIDER_COL     COL_WHITE

gslc_tsElemRef* create_listbox(int16_t page, int16_t elem, gslc_tsXListbox* pelem, gslc_tsColor col)
{
    gslc_tsElemRef* pElemRef = gslc_ElemXListboxCreate(&gslc, elem, page, pelem, BOX_RECT, FONT_BUILTIN5X8, NULL, 0, 0);
    gslc_ElemXListboxSetSize            (&gslc, pElemRef, BOX_LINES, 1); // rows, columns
    gslc_ElemSetTxtMarginXY             (&gslc, pElemRef, 0, 0);
    gslc_ElemXListboxSetMargin          (&gslc, pElemRef, 0, 0);
    gslc_ElemSetTxtCol                  (&gslc, pElemRef, COL_TEXT_NORMAL);
    gslc_ElemSetCol                     (&gslc, pElemRef, COL_ERROR, col, COL_BLACK);
    gslc_ElemSetGlowCol                 (&gslc, pElemRef, COL_ERROR, COL_SEL, COL_TEXT_NORMAL);
    gslc_ElemXListboxItemsSetGap        (&gslc, pElemRef, 0, COL_ERROR);
    gslc_ElemXListboxItemsSetSize       (&gslc, pElemRef, BOX_W, XLISTBOX_SIZE_AUTO);
    return pElemRef;
}

#define SLIDER_POS_MAX 240
gslc_tsElemRef* create_slider(int16_t page, int16_t elem, gslc_tsXSlider* pelem, gslc_tsColor col)
{
    gslc_tsElemRef* pElemRef = gslc_ElemXSliderCreate(&gslc, elem, page, pelem, SLIDER_RECT, 0, SLIDER_POS_MAX, 0, 5, true);
    gslc_ElemSetCol                     (&gslc, pElemRef, SLIDER_COL, col, col);
    gslc_ElemXSliderSetPosFunc          (&gslc, pElemRef, NULL);
    return pElemRef;
}


//###############################################################
// page:info
//###############################################################
#define INFO_BACK_COL       COL_GRAY_DARK

#define INFO_GAP 2
#define INFO_MODE_W 100
#define INFO_MODE_RECT      (gslc_tsRect){0, 0, INFO_MODE_W, LINE_H}
#define INFO_MODE_COL       COL_GREEN_DARK

#define INFO_INDEX_RECT     (gslc_tsRect){INFO_MODE_W+INFO_GAP, 0, LCD_W-INFO_MODE_W-INFO_GAP, LINE_H}
#define INFO_INDEX_COL      COL_GREEN_DARK

#if (LINE_H==20)
#define INFO_LINE_Y         (LINE_H * 2)
#define INFO_LINE_STEP      (LINE_H + 3)
#elif (LINE_H==28)
#define INFO_LINE_Y         (LINE_H + 3)
#define INFO_LINE_STEP      (LINE_H + 2)
#endif
#define INFO_LINE_RECT      (gslc_tsRect){0, y, LCD_W, LINE_H}
gslc_tsColor                info_colors[] = {COL_BLUE_DARK, COL_BLUE_DARK, COL_RED_DARK, COL_RED_DARK, COL_RED_DARK, COL_RED_DARK};

#define INFO_PGS1_W         60
#define INFO_PGS3_W         60
#define INFO_PGS2_W         (LCD_W-INFO_PGS1_W-INFO_PGS3_W)
#define INFO_PGS2_MAX       (INFO_PGS2_W-1)
#define INFO_PGS1_RECT      (gslc_tsRect){0, LCD_H-LINE_H, INFO_PGS1_W, LINE_H}
#define INFO_PGS2_RECT      (gslc_tsRect){INFO_PGS1_W, LCD_H-LINE_H, INFO_PGS2_W, LINE_H}
#define INFO_PGS3_RECT      (gslc_tsRect){LCD_W-INFO_PGS3_W, LCD_H-LINE_H, INFO_PGS3_W, LINE_H}
#define INFO_PGS_FILL_COL   COL_GREEN_DARK
#define INFO_PGS2_FILL_COL  COL_BLACK
#define INFO_PGS_LINE_COL   COL_GREEN
#define INFO_PGS_FRAME_COL  COL_WHITE

gslc_tsElem                 info_elem[INFO_ELEM_MAX];
gslc_tsElemRef              info_ref[INFO_ELEM_MAX];

enum {
    INFO_PATH, INFO_FILE, INFO_BAND, INFO_ARTIST, INFO_ALBUM, INFO_TITLE, 
    INFO_LINES
};

char                        info_mode_str[20] = "                   ";
gslc_tsElemRef*             info_mode_ref = NULL;

char                        info_index_str[MAX_STR] = {0};
gslc_tsElemRef*             info_index_ref = NULL;

char                        info_pgs1_str[10] = {0};
gslc_tsElemRef*             info_pgs1_ref = NULL;

gslc_tsXProgress            info_pgs2_elem;
gslc_tsElemRef*             info_pgs2_ref  = NULL;

char                        info_pgs3_str[10] = {0};
gslc_tsElemRef*             info_pgs3_ref = NULL;

char                        info_lines_str[INFO_LINES][MAX_STR] = {0};
gslc_tsElemRef*             info_lines_ref[INFO_LINES] = {0};

void page_info_init()
{
    gslc_PageAdd                    (&gslc, PAGE_INFO, info_elem, INFO_ELEM_MAX, info_ref, INFO_ELEM_MAX);
    gslc_tsElemRef* pElemRef = NULL;

    pElemRef = gslc_ElemCreateTxt   (&gslc, INFO_MODE_ELEM, PAGE_INFO, INFO_MODE_RECT, info_mode_str, sizeof(info_mode_str), FONT_BUILTIN5X8);
    gslc_ElemSetTxtCol              (&gslc, pElemRef, COL_TEXT_NORMAL);
    gslc_ElemSetCol                 (&gslc, pElemRef, COL_ERROR, INFO_MODE_COL, INFO_MODE_COL);
    info_mode_ref = pElemRef;

    pElemRef = gslc_ElemCreateTxt   (&gslc, INFO_INDEX_ELEM, PAGE_INFO, INFO_INDEX_RECT, info_index_str, sizeof(info_index_str), FONT_BUILTIN5X8);
    gslc_ElemSetTxtAlign            (&gslc, pElemRef, GSLC_ALIGN_MID_MID);
    gslc_ElemSetTxtCol              (&gslc, pElemRef, COL_WHITE);
    gslc_ElemSetCol                 (&gslc, pElemRef, COL_ERROR, INFO_INDEX_COL, INFO_INDEX_COL);
    info_index_ref = pElemRef;

    pElemRef = gslc_ElemCreateTxt   (&gslc, INFO_PGS1_ELEM, PAGE_INFO, INFO_PGS1_RECT, info_pgs1_str, sizeof(info_pgs1_str), FONT_BUILTIN5X8);
    gslc_ElemSetTxtAlign            (&gslc, pElemRef, GSLC_ALIGN_MID_MID);
    gslc_ElemSetTxtCol              (&gslc, pElemRef, COL_TEXT_NORMAL);
    gslc_ElemSetCol                 (&gslc, pElemRef, COL_ERROR, INFO_PGS_FILL_COL, INFO_PGS_FILL_COL);
    info_pgs1_ref = pElemRef;

    pElemRef = gslc_ElemXProgressCreate(&gslc, INFO_PGS2_ELEM, PAGE_INFO, &info_pgs2_elem, INFO_PGS2_RECT, 0, INFO_PGS2_MAX, 0, INFO_PGS_LINE_COL, false);
    info_pgs2_ref = pElemRef;

    pElemRef = gslc_ElemCreateTxt   (&gslc, INFO_PGS3_ELEM, PAGE_INFO, INFO_PGS3_RECT, info_pgs3_str, sizeof(info_pgs3_str), FONT_BUILTIN5X8);
    gslc_ElemSetTxtAlign            (&gslc, pElemRef, GSLC_ALIGN_MID_MID);
    gslc_ElemSetTxtCol              (&gslc, pElemRef, COL_TEXT_NORMAL);
    gslc_ElemSetCol                 (&gslc, pElemRef, COL_ERROR, INFO_PGS_FILL_COL, INFO_PGS_FILL_COL);
    info_pgs3_ref = pElemRef;

    int i;
    int16_t y = INFO_LINE_Y;
    for (i = 0; i < INFO_LINES; i++)
    {
        pElemRef = gslc_ElemCreateTxt(&gslc, INFO_PATH_ELEM+i, PAGE_INFO, INFO_LINE_RECT, info_lines_str[i], sizeof(info_lines_str[i]), FONT_BUILTIN5X8);
        gslc_ElemSetTxtCol          (&gslc, pElemRef, COL_TEXT_NORMAL);
        gslc_ElemSetCol             (&gslc, pElemRef, COL_ERROR, info_colors[i], info_colors[i]);
        info_lines_ref[i] = pElemRef;
        y += INFO_LINE_STEP;
    }
}



//###############################################################
// page:list
//###############################################################
#define LIST_BACK_COL       COL_BLACK

#define LIST_COL_NORMAL_B   COL_BLACK
#define LIST_COL_NORMAL_F   COL_WHITE
#define LIST_COL_PLAY_B     COL_RED_DARK
#define LIST_COL_PLAY_F     COL_RED_LIGHT
#define LIST_COL_DIR_B      COL_GREEN_DARK
#define LIST_COL_DIR_F      COL_GREEN_LIGHT

gslc_tsElem                 list_elem[LIST_ELEM_MAX];
gslc_tsElemRef              list_ref[LIST_ELEM_MAX];

gslc_tsXListbox             list_box_elem;
gslc_tsElemRef*             list_box_ref   = NULL;

gslc_tsXSlider              list_slider_elem;
gslc_tsElemRef*             list_slider_ref    = NULL;

void page_list_init()
{
    gslc_PageAdd(&gslc, PAGE_LIST, list_elem, LIST_ELEM_MAX, list_ref, LIST_ELEM_MAX);
    list_box_ref   = create_listbox(PAGE_LIST, LIST_BOX_ELEM,    &list_box_elem,    LIST_BACK_COL);
    list_slider_ref = create_slider(PAGE_LIST, LIST_SLIDER_ELEM, &list_slider_elem, LIST_BACK_COL);
}


//###############################################################
// page:pic
//###############################################################
/*
#define PIC_BACK_COL COL_BLACK
#define PIC_X 0
#define PIC_Y 0
#define PIC_H 200
#define PIC_W 200
#define PIC_LINE_Y (PIC_Y + PIC_H + 20)
#define PIC_LINE_W LCD_W
#define PIC_FILL_COL COL_BLUE_DARK
#define PIC_TEXT_COL COL_WHITE

#define PIC_NAME_RECT       (gslc_tsRect){0, 0, LCD_W, LINE_H}

//#define PIC_PIC_NAME        "/Test Pics/Cover.jpg"
#define PIC_PIC_NAME        "/back1_24.bmp"

#define PIC_PIC_RECT        (gslc_tsRect){0, LINE_H, LCD_W, LCD_H-LINE_H}

gslc_tsElem                 pic_elem[PIC_ELEM_MAX];
gslc_tsElemRef              pic_ref[PIC_ELEM_MAX];

char                        pic_name_str[MAX_STR] = "Song Name ...";
gslc_tsElemRef*             pic_name_ref = NULL;

gslc_tsElemRef*             pic_pic_ref = NULL;


//#if (GSLC_SD_EN)

//   pElemRef = gslc_ElemCreateBox(&m_gui,E_ELEM_BOX,E_PG_MAIN,(gslc_tsRect){10,50,300,150});
//   gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_WHITE,GSLC_COL_BLACK,GSLC_COL_BLACK);
//   pElemRef = gslc_ElemCreateBtnImg(&m_gui,E_ELEM_BTN_QUIT,E_PG_MAIN,(gslc_tsRect){258,70,32,32},
//           gslc_GetImageFromSD(IMG_BTN_QUIT,GSLC_IMGREF_FMT_BMP24),
//           gslc_GetImageFromSD(IMG_BTN_QUIT_SEL,GSLC_IMGREF_FMT_BMP24),
//           &CbBtnQuit);
//   gslc_ElemSetFillEn(&m_gui,pElemRef,true); // On slow displays disable transparency to prevent full redraw

// bool gslc_DrvSetElemImageNorm(gslc_tsGui* pGui,gslc_tsElem* pElem,gslc_tsImgRef sImgRef)


void page_pic_init()
{
    gslc_PageAdd                    (&gslc, PAGE_PIC, pic_elem, PIC_ELEM_MAX, pic_ref, PIC_ELEM_MAX);
    gslc_tsElemRef* pElemRef = NULL;

    pElemRef = gslc_ElemCreateTxt   (&gslc, PIC_NAME_ELEM, PAGE_PIC, PIC_NAME_RECT, pic_name_str, sizeof(pic_name_str), FONT_BUILTIN5X8);
    gslc_ElemSetTxtCol              (&gslc, pElemRef, COL_TEXT_NORMAL);
    gslc_ElemSetCol                 (&gslc, pElemRef, COL_FRAME, PIC_FILL_COL, PIC_FILL_COL);
    pic_name_ref = pElemRef;

//    gslc_ElemCreateBox              (&gslc, PIC_PIC_ELEM, PAGE_PIC, PIC_PIC_RECT);
//    gslc_ElemSetImage               (&gslc, pGui,gslc_tsElemRef* pElemRef,gslc_tsImgRef sImgRef,
//     gslc_tsImgRef sImgRefSel)

    pElemRef = gslc_ElemCreateBtnImg(&gslc, PIC_PIC_ELEM, PAGE_PIC, PIC_PIC_RECT,
          gslc_GetImageFromSD(PIC_PIC_NAME, GSLC_IMGREF_FMT_BMP24), 
          gslc_GetImageFromSD(PIC_PIC_NAME, GSLC_IMGREF_FMT_BMP24), NULL);
    pic_pic_ref = pElemRef;

    // pElemRef = gslc_ElemCreateBtnImg(&gslc, PIC_PIC_ELEM, PAGE_PIC, PIC_PIC_RECT,
    //       gslc_GetImageFromSD(PIC_PIC_NAME, GSLC_IMGREF_FMT_JPG), 
    //       gslc_GetImageFromSD(PIC_PIC_NAME, GSLC_IMGREF_FMT_JPG), NULL);
    // pic_pic_ref = pElemRef;
}
*/

//###############################################################
// page:fav
//###############################################################
#define FAV_BACK_COL        COL_RED_DARK

#define FAV_COL_NORMAL_B    COL_RED_DARK
#define FAV_COL_NORMAL_F    COL_WHITE
#define FAV_COL_PLAY_B      COL_RED_DARK
#define FAV_COL_PLAY_F      COL_RED_LIGHT

gslc_tsElem                 fav_elem[FAV_ELEM_MAX];
gslc_tsElemRef              fav_ref[FAV_ELEM_MAX];

gslc_tsXListbox             fav_box_elem;
gslc_tsElemRef*             fav_box_ref   = NULL;

gslc_tsXSlider              fav_slider_elem;
gslc_tsElemRef*             fav_slider_ref    = NULL;

void page_fav_init()
{
    gslc_PageAdd(&gslc, PAGE_FAV, fav_elem, FAV_ELEM_MAX, fav_ref, FAV_ELEM_MAX);
    fav_box_ref   = create_listbox(PAGE_FAV, FAV_BOX_ELEM,    &fav_box_elem,    FAV_BACK_COL);
    fav_slider_ref = create_slider(PAGE_FAV, FAV_SLIDER_ELEM, &fav_slider_elem, FAV_BACK_COL);
}


//###############################################################
// page:dirs
//###############################################################
#define DIRS_BACK_COL       COL_GREEN_DARK

#define DIRS_COL_NORMAL_B   COL_GREEN_DARK
#define DIRS_COL_NORMAL_F   COL_WHITE
#define DIRS_COL_PLAY_B     COL_GREEN_DARK
#define DIRS_COL_PLAY_F     COL_RED_LIGHT

gslc_tsElem                 dirs_elem[DIRS_ELEM_MAX];
gslc_tsElemRef              dirs_ref[DIRS_ELEM_MAX];

gslc_tsXListbox             dirs_box_elem;
gslc_tsElemRef*             dirs_box_ref   = NULL;

gslc_tsXSlider              dirs_slider_elem;
gslc_tsElemRef*             dirs_slider_ref    = NULL;

void page_dirs_init()
{
    gslc_PageAdd(&gslc, PAGE_DIRS, dirs_elem, DIRS_ELEM_MAX, dirs_ref, DIRS_ELEM_MAX);
    dirs_box_ref   = create_listbox(PAGE_DIRS, DIRS_BOX_ELEM,    &dirs_box_elem,    DIRS_BACK_COL);
    dirs_slider_ref = create_slider(PAGE_DIRS, DIRS_SLIDER_ELEM, &dirs_slider_elem, DIRS_BACK_COL);
}


//###############################################################
Gui::Gui()
{
    Serial.println("LCD init ...");
    gslc_init();
    page_info_init();
    page_list_init();
    //page_pic_init();
    page_fav_init();
    page_dirs_init();

    page(PAGE_INFO);

    Serial.println("gslc initialized");

    tft.fillScreen(TFT_BLACK);
    tft.setTextWrap(false, false);

#ifdef SPIFFS_FONT
    if (!SPIFFS.begin()) 
    {
        Serial.println("SPIFFS initialisation failed!");
        while (1) 
            yield(); // Stay here twiddling thumbs waiting
    }
#endif

    tft.loadFont(FONT_NAME1);

    initialized = true;
    Serial.println("LCD init OK");
}


void Gui::loop()
{
    gslc_Update(&gslc);
}


void Gui::redraw()
{
    gslc_PageRedrawSet(&gslc, true);
}


//###############################################################
void Gui::page(int page_n)
{
    static gslc_tsColor page_back_col[] = {INFO_BACK_COL, LIST_BACK_COL, /*PIC_BACK_COL,*/ FAV_BACK_COL, DIRS_BACK_COL};
    gslc_SetPageCur(&gslc, page_n);
    gslc_SetBkgndColor(&gslc, page_back_col[page_n]);
}


//###############################################################
void Gui::list_select(int curfile)
{
    int index = curfile-1;
    gslc_ElemXListboxSetSel(&gslc, list_box_ref, index);
    gslc_ElemXListboxSetScrollPos(&gslc, list_box_ref, index - BOX_LINES/2 + 1);
    list_selfile = gslc_ElemXListboxGetSel(&gslc, list_box_ref) + 1;
}


void Gui::list_highlight(void *gslc, void *pElemRef, int type)
{
    static gslc_tsColor colors_b[] = {LIST_COL_NORMAL_B, LIST_COL_DIR_B, LIST_COL_PLAY_B};
    static gslc_tsColor colors_f[] = {LIST_COL_NORMAL_F, LIST_COL_DIR_F, LIST_COL_PLAY_F};
    gslc_tsElem * elem = &list_elem[LIST_BOX_ELEM-LIST_BOX_ELEM];
    elem->colElemText       = colors_f[type];
    elem->colElemTextGlow   = colors_f[type];
    elem->colElemFill       = colors_b[type];
}


void Gui::list_box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb)
{
    list_box_elem.pfuncXGet = cb;
    list_box_elem.nItemCnt = cnt;
    list_box_elem.nItemCurSel = 0;
    list_box_elem.nItemTop = 0;
    list_box_elem.bNeedRecalc = true;
}


static int listbox_seek(gslc_tsXListbox * box, gslc_tsElemRef * box_ref, gslc_tsElemRef * slider_ref, int by)
{
    //todo: вычислять box, slider
    int sel = gslc_ElemXListboxGetSel(&gslc, box_ref);
    sel -= by;
    gslc_ElemXListboxSetSel(&gslc, box_ref, sel);

    sel = gslc_ElemXListboxGetSel(&gslc, box_ref);

    int max = box->nItemCnt - 1;
    if (max < 1) max = 1;
    
    int pos = SLIDER_POS_MAX * sel / max;
    gslc_ElemXSliderSetPos(&gslc, slider_ref, pos);

    return sel;
}


void Gui::list_seek(int by)
{
    list_selfile = listbox_seek(&list_box_elem, list_box_ref, list_slider_ref, by) + 1;
}


//###############################################################
void Gui::fav_box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb)
{
    fav_box_elem.pfuncXGet = cb;
    fav_box_elem.nItemCnt = cnt;
    fav_box_elem.bNeedRecalc = true;
}


void Gui::fav_set(int num)
{
    gslc_ElemXListboxSetSel(&gslc, fav_box_ref, num-1);
}


void Gui::fav_seek(int by)
{
    fav_selfile = listbox_seek(&fav_box_elem, fav_box_ref, fav_slider_ref, by) + 1;
}


void Gui::fav_highlight(void *gslc, void *pElemRef, int type)
{
    static gslc_tsColor colors_b[] = {FAV_COL_NORMAL_B, FAV_COL_PLAY_B};
    static gslc_tsColor colors_f[] = {FAV_COL_NORMAL_F, FAV_COL_PLAY_F};
    gslc_tsElem * elem = &fav_elem[FAV_BOX_ELEM-FAV_BOX_ELEM];
    elem->colElemText       = colors_f[type];
    elem->colElemTextGlow   = colors_f[type];
    elem->colElemFill       = colors_b[type];
}


//###############################################################
void Gui::dirs_box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb)
{
    dirs_box_elem.pfuncXGet = cb;
    dirs_box_elem.nItemCnt = cnt;
    dirs_box_elem.bNeedRecalc = true;
}


void Gui::dirs_highlight(void *gslc, void *pElemRef, int type)
{
    static gslc_tsColor colors_b[] = {DIRS_COL_NORMAL_B, DIRS_COL_PLAY_B};
    static gslc_tsColor colors_f[] = {DIRS_COL_NORMAL_F, DIRS_COL_PLAY_F};
    gslc_tsElem * elem = &dirs_elem[DIRS_BOX_ELEM-DIRS_BOX_ELEM];
    elem->colElemText       = colors_f[type];
    elem->colElemTextGlow   = colors_f[type];
    elem->colElemFill       = colors_b[type];
}


void Gui::dirs_seek(int by)
{
    dirs_seldir = listbox_seek(&dirs_box_elem, dirs_box_ref, dirs_slider_ref, by) + 1;
}


//###############################################################
void Gui::index(const char * text)
{
    gslc_ElemSetTxtStr(&gslc, info_index_ref, text);
}


void Gui::step_progress(uint32_t pos, uint32_t total)
{
    char t[10] = "";

    sprintf(t, "%i", pos);
    gslc_ElemSetTxtStr(&gslc, info_pgs1_ref, t);

    sprintf(t, "%i", total);
    gslc_ElemSetTxtStr(&gslc, info_pgs3_ref, t);

    int wdt = total ? INFO_PGS2_MAX * pos / total : 0;
    gslc_ElemXProgressSetVal(&gslc, info_pgs2_ref, wdt);
}


void Gui::time_progress(uint32_t pos, uint32_t total)
{
    char t[15] = "";

    sprintf(t, "%i:%02i", pos/60, pos%60);
    gslc_ElemSetTxtStr(&gslc, info_pgs1_ref, t);

    sprintf(t, "%i:%02i", total/60, total%60);
    gslc_ElemSetTxtStr(&gslc, info_pgs3_ref, t);

    int wdt = total ? INFO_PGS2_MAX * pos / total : 0;
    gslc_ElemXProgressSetVal(&gslc, info_pgs2_ref, wdt);
}


void Gui::band(const char * text)
{
    gslc_ElemSetTxtStr(&gslc, info_lines_ref[INFO_BAND], text);
}


void Gui::artist(const char * text)
{
    gslc_ElemSetTxtStr(&gslc, info_lines_ref[INFO_ARTIST], text);
}


void Gui::album(const char * text)
{
    gslc_ElemSetTxtStr(&gslc, info_lines_ref[INFO_ALBUM], text);
}


void Gui::title(const char * text)
{
    gslc_ElemSetTxtStr(&gslc, info_lines_ref[INFO_TITLE], text);
}


void Gui::file(const char * text)
{
    gslc_ElemSetTxtStr(&gslc, info_lines_ref[INFO_FILE], text);
}


void Gui::path(const char * text)
{
    gslc_ElemSetTxtStr(&gslc, info_lines_ref[INFO_PATH], text);
}


//###############################################################
static int vol = 0;
static char rep = '?';
static char shf = '?';
static char alv = '?';

void Gui::mode()
{
    char t[20];
    sprintf(t, "%c %c %2i %c", rep, shf, vol, alv);
    gslc_ElemSetTxtStr(&gslc, info_mode_ref, t);
}


void Gui::volume(int volume)
{
    vol = volume;
    mode();
}


void Gui::repeat(bool val)
{
    rep = val ? 'O' : '.';
    mode();
}


void Gui::shuffle(bool val)
{
    shf = val ? 'X' : '=';
    mode();
}


void Gui::alive(bool running)
{
    static int index = 0;
    static char sym[] = "-\\|/";
    alv = running ? sym[++index %= 4] : 'P';
    mode();
}


