#include <TFT_eSPI.h>      // Hardware-specific library

#include "GUIslice.h"
#include "GUIslice_drv.h"
#include "elem/XProgress.h"
#include "elem/XTextbox.h"
#include "elem/XListbox.h"

#include "debug.h"
#include "globals.h"

#include "font.h"
#include "gui.h"

#include "gui_common.h"
#include "gui_info.h"
#include "gui_files.h"
#include "gui_dirs.h"
#include "gui_fav.h"

#include "page_info.h"
#include "page_files.h"
#include "page_dirs.h"
#include "page_sys.h"
#include "page_fav.h"

//###############################################################
// GUI common
//###############################################################
extern TFT_eSPI m_disp;
TFT_eSPI &tft = m_disp;
static int16_t DebugOut(char ch) { Serial.write(ch); return 0; }

TFT_eSprite gui_spr = TFT_eSprite(&tft);
uint8_t * spr_buf;

gslc_tsGui                  gslc;
gslc_tsDriver               m_drv;
gslc_tsFont                 fonts[FONT_MAX];
gslc_tsPage                 pages[PAGE_MAX];

void gslc_init()
{
    gslc_InitDebug(&DebugOut);

    if (!gslc_Init(&gslc,&m_drv, pages, PAGE_MAX, fonts, FONT_MAX))
    {
        DEBUG("gslc init error\n");
        return;
    }

    if (!gslc_FontSet(&gslc, FONT_BUILTIN20x32, GSLC_FONTREF_PTR, NULL, 4))
    {
        DEBUG("gslc fontset20 error\n");
        return;
    }

    if (!gslc_FontSet(&gslc, FONT_BUILTIN5X8, GSLC_FONTREF_PTR, NULL, 1))
    {
        DEBUG("gslc fontset5 error\n");
        return;
    }

    // if (!gslc_FontSet(&gslc, FONT_SMOOTH, GSLC_FONTREF_PTR, FONT_NAME1, 1))
    // {
    //     DEBUG("gslc fontset2 error\n");
    //     return;
    // }
}


void check_font()
{
    return;
//     uint8_t *font_addr = (uint8_t *)gui_spr.gFont.gArray;
//     Serial.printf("font addr: %08X, chars:%d\n", (uint32_t)font_addr, gui_spr.gFont.gCount);
//     int i;
//     for (i = 0; i < gui_spr.gFont.gCount; i++)
//     {
//         TFT_eSPI::CharMetrics * cm = gui_spr.getCharMetrics(i);
//         const uint8_t* gPtr = (const uint8_t*) gui_spr.gFont.gArray + cm->gBitmap;
//         uint32_t data = *(uint32_t *)((uint32_t)gPtr & ~3);
//         if (data == 0xBAD00BAD)
//         {
//             Serial.printf("error char: %d, code: %d, addr: %08X\n", i, cm->gUnicode, (uint32_t)gPtr);
//         }
//         if (cm->gUnicode == 0x9CE5 || data == 0xBAD00BAD)
//         {
// //            DEBUG_DUMP32((void *)(((uint32_t)gPtr & ~0x7FF) - 16), 64);
//         }
//         if (data == 0xBAD00BAD)
//             return;
//     }
//     Serial.printf("all chars are ok\n");
}


//###############################################################
Gui::Gui()
{
    DEBUG("LCD init ...\n");
    spr_buf = (uint8_t *)malloc(LCD_W*LINE_H_MAX*(16/8)+1);

    gslc_init();
    
    page_info.init();
    page_files.init();
    //page_pic_init();
    page_fav.init();
    page_dirs.init();
    page_sys.init();

    set_page(PAGE_INFO);

    DEBUG("gslc initialized\n");

    tft.fillScreen(TFT_BLACK);
    gui_spr.setTextWrap(false, false);

    //tft.loadFont(FONT_NAME1);
    //tft.loadFont("font1", ESP_PARTITION_SUBTYPE_DATA_FAT);  //ESP_PARTITION_SUBTYPE_ANY

    // spi_flash_mmap_dump();
    // uint32_t free1 = spi_flash_mmap_get_free_pages(SPI_FLASH_MMAP_INST);
    // uint32_t free2 = spi_flash_mmap_get_free_pages(SPI_FLASH_MMAP_DATA);
    // Serial.printf("free inst:%d data:%d\n", free1, free2);

    //gui_spr.loadFont(FONT_NAME1);
    gui_spr.loadFont("font1", ESP_PARTITION_SUBTYPE_DATA_FAT);  //ESP_PARTITION_SUBTYPE_ANY

    // spi_flash_mmap_dump();
    // free1 = spi_flash_mmap_get_free_pages(SPI_FLASH_MMAP_INST);
    // free2 = spi_flash_mmap_get_free_pages(SPI_FLASH_MMAP_DATA);
    // Serial.printf("free inst:%d data:%d\n", free1, free2);

    //check_font();

    initialized = true;
    DEBUG("LCD init OK\n");
}


//###############################################################
void Gui::loop()
{
    gslc_Update(&gslc);
    scroll();
}


//###############################################################
void Gui::redraw()
{
    gslc_PageRedrawSet(&gslc, true);
}


//###############################################################
void Gui::set_page(int page_n)
{
    static gslc_tsColor page_back_col[] = {INFO_BACK_COL, FILES_BACK_COL, /*PIC_BACK_COL,*/ FAV_BACK_COL, DIRS_BACK_COL};
    gslc_SetPageCur(&gslc, page_n);
    gslc_SetBkgndColor(&gslc, page_back_col[page_n]);
    scroll_reset();
}


//###############################################################
// listbox common
//###############################################################
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
    gslc_ElemSetRoundEn                 (&gslc, pElemRef, true);
    return pElemRef;
}

gslc_tsElemRef* create_slider(int16_t page, int16_t elem, gslc_tsXSlider* pelem, gslc_tsColor col)
{
    gslc_tsElemRef* pElemRef = gslc_ElemXSliderCreate(&gslc, elem, page, pelem, SLIDER_RECT, 0, SLIDER_POS_MAX, 0, 5, true);
    gslc_ElemSetCol                     (&gslc, pElemRef, SLIDER_COL, col, col);
    gslc_ElemXSliderSetPosFunc          (&gslc, pElemRef, NULL);
    return pElemRef;
}


// static void slider_update(gslc_tsElemRef * slider_ref, int sel, int nItemCnt)
// {
//     int max = nItemCnt - 1;
//     if (max < 1) 
//         max = 1;
    
//     int pos = SLIDER_POS_MAX * sel / max;
//     gslc_ElemXSliderSetPos(&gslc, slider_ref, pos);
// }


int box_goto(gslc_tsElemRef * box_ref, gslc_tsElemRef * slider_ref, int16_t index, bool center)
{
    gslc_tsElem * elem = gslc_GetElemFromRef(&gslc, box_ref);
    gslc_tsXListbox * box = (gslc_tsXListbox *)elem->pXData;
    int16_t cnt = box->nItemCnt;

    if (index >= cnt)
        index = cnt-1;
    if (index < 0)
        index = 0;
    gslc_ElemXListboxSetSel(&gslc, box_ref, index);
    
    if (center)
    {
        int top = index - BOX_LINES/2 + 1;
        if (top + BOX_LINES > cnt)
            top = cnt - BOX_LINES;
        if (top < 0)
            top = 0;
        gslc_ElemXListboxSetScrollPos(&gslc, box_ref, top);
    }
    
    int max = cnt - 1;
    if (max < 1) 
        max = 1;
    int pos = SLIDER_POS_MAX * index / max;
    gslc_ElemXSliderSetPos(&gslc, slider_ref, pos);

    return index;
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


