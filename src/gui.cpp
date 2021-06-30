#include <TFT_eSPI.h>      // Hardware-specific library

#include "GUIslice.h"
#include "GUIslice_drv.h"
#include "elem/XProgress.h"
#include "elem/XSlider.h"
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
#include "gui_sys.h"

#include "gui_icons.h"

//###############################################################
// GUI common
//###############################################################
extern TFT_eSPI m_disp;
TFT_eSPI &tft = m_disp;
static int16_t DebugOut(char ch) { Serial.write(ch); return 0; }

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

    if (!gslc_FontSet(&gslc, FONT_SMOOTH, GSLC_FONTREF_PTR, FONT_NAME1, 1))
    {
        DEBUG("gslc fontset2 error\n");
        return;
    }
}


//###############################################################
Gui::Gui()
{
    DEBUG("LCD init ...\n");
    gslc_init();
    page_info_init();
    page_files_init();
    //page_pic_init();
    page_fav_init();
    page_dirs_init();
    page_sys_init();

    page(PAGE_INFO);

    DEBUG("gslc initialized\n");

    tft.fillScreen(TFT_BLACK);
    tft.setTextWrap(false, false);

    tft.loadFont(FONT_NAME1);

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
void Gui::page(int page_n)
{
    static gslc_tsColor page_back_col[] = {INFO_BACK_COL, FILES_BACK_COL, /*PIC_BACK_COL,*/ FAV_BACK_COL, DIRS_BACK_COL};
    gslc_SetPageCur(&gslc, page_n);
    gslc_SetBkgndColor(&gslc, page_back_col[page_n]);
    scroll_reset();
}


//###############################################################
// page:info
//###############################################################
gslc_tsColor                info_colors[] = {COL_BLUE_DARK, COL_BLUE_DARK, COL_RED_DARK, COL_RED_DARK, COL_RED_DARK, COL_RED_DARK};

gslc_tsElem                 info_elem[INFO_ELEM_MAX];
gslc_tsElemRef              info_ref[INFO_ELEM_MAX];

char                        info_mode_str[20] = "";
gslc_tsElemRef*             info_mode_ref = NULL;
gslc_tsElemRef*             info_mode_icons_ref[INFO_MODE_ICONS] = {0};

char                        info_fav_str[10] = "";
gslc_tsElemRef*             info_fav_ref = NULL;

char                        info_index_str[INFO_INDEX_MAX_STR] = {0};
gslc_tsElemRef*             info_index_ref = NULL;

char                        info_pgs1_str[10] = {0};
gslc_tsElemRef*             info_pgs1_ref = NULL;

gslc_tsXProgress            info_pgs2_elem;
gslc_tsElemRef*             info_pgs2_ref  = NULL;

char                        info_pgs3_str[10] = {0};
gslc_tsElemRef*             info_pgs3_ref = NULL;

char                        info_lines_str[INFO_LINES][INFO_LINE_MAX_STR] = {0};
gslc_tsElemRef*             info_lines_ref[INFO_LINES] = {0};
gslc_tsElemRef*             info_icons_ref[INFO_LINES] = {0};


const unsigned short * icons[ICONS_TOTAL] GSLC_PMEM = {
    icon_pause, icon_disk0, icon_disk1, icon_disk2, icon_disk3, icon_fav, icon_index, 
    icon_wifi_off, icon_wifi_on, icon_tether, icon_ftp, icon_upgrade, icon_bluetooth_off, icon_bluetooth_on, 
    icon_shuffle_off, icon_shuffle_on, icon_repeat_off, icon_repeat_on, icon_volume_nogain, icon_volume_gain,
    icon_path, icon_file, icon_band, icon_artist, icon_album, icon_title,
};


gslc_tsElemRef* create_icon(int elem_id, int icon1, int icon2, int16_t &x, int gap)
{
    gslc_tsImgRef imgref1 = gslc_GetImageFromProg((const unsigned char*)icons[icon1], GSLC_IMGREF_FMT_BMP24);
    gslc_tsImgRef imgref2 = gslc_GetImageFromProg((const unsigned char*)icons[icon2], GSLC_IMGREF_FMT_BMP24);
    gslc_tsElemRef* pElemRef = gslc_ElemCreateImg(&gslc, elem_id, PAGE_INFO, INFO_MODE_ICON_RECT, imgref1);
    gslc_ElemSetGlowEn(&gslc, pElemRef, true);
    gslc_ElemSetImage (&gslc, pElemRef, imgref1, imgref2);
    gslc_ElemSetCol   (&gslc, pElemRef, COL_ERROR, INFO_COL, INFO_COL);
    x += INFO_ICON_W + gap;
    return pElemRef;
}


gslc_tsElemRef* create_text(int elem_id, gslc_tsRect rect, char * str, int strsize, int16_t &x, int gap)
{
    gslc_tsElemRef* pElemRef = gslc_ElemCreateTxt(&gslc, elem_id, PAGE_INFO, rect, str, strsize, FONT_BUILTIN5X8);
    gslc_ElemSetTxtCol              (&gslc, pElemRef, COL_TEXT_NORMAL);
    gslc_ElemSetCol                 (&gslc, pElemRef, COL_ERROR, INFO_COL, INFO_COL);
    gslc_ElemSetTxtAlign            (&gslc, pElemRef, GSLC_ALIGN_MID_MID);
    x += rect.w + gap;
    return pElemRef;
}


void page_info_init()
{
    gslc_PageAdd(&gslc, PAGE_INFO, info_elem, INFO_ELEM_MAX, info_ref, INFO_ELEM_MAX);
    gslc_tsElemRef* pElemRef = NULL;

    int r = 0;
    int16_t x = 0;

    info_mode_icons_ref[r++] = create_icon(INFO_PLAY_ICON, ICON_PAUSE, ICON_PAUSE, x, INFO_GAP);

    info_mode_icons_ref[r++] = create_icon(INFO_SHUFFLE_ICON, ICON_SHUFFLE_OFF, ICON_SHUFFLE_ON, x, INFO_GAP);
 
    info_mode_icons_ref[r++] = create_icon(INFO_REPEAT_ICON, ICON_REPEAT_OFF, ICON_REPEAT_ON, x, INFO_GAP);

    info_mode_icons_ref[r++] = create_icon(INFO_VOLUME_ICON, ICON_VOLUME_NOGAIN, ICON_VOLUME_GAIN, x, 0);
    info_mode_ref = create_text(INFO_VOLUME_ELEM, INFO_VOLUME_RECT, info_mode_str, sizeof(info_mode_str), x, INFO_GAP);

    // fav
    info_mode_icons_ref[r++] = create_icon(INFO_FAV_ICON, ICON_FAV, ICON_FAV, x, 0);
    info_fav_ref = create_text(INFO_FAV_ELEM, INFO_FAV_RECT, info_fav_str, sizeof(info_fav_str), x, INFO_GAP);

    // index
    info_mode_icons_ref[r++] = create_icon(INFO_INDEX_ICON, ICON_INDEX, ICON_INDEX, x, 0);
    info_index_ref = create_text(INFO_INDEX_ELEM, INFO_INDEX_RECT, info_index_str, sizeof(info_index_str), x, INFO_GAP);

    // wifi
    info_mode_icons_ref[r++] = create_icon(INFO_WIFI_ICON, ICON_WIFI_OFF, ICON_WIFI_OFF, x, 0);

    // progress
    pElemRef = gslc_ElemCreateTxt   (&gslc, INFO_PGS1_ELEM, PAGE_INFO, INFO_PGS1_RECT, info_pgs1_str, sizeof(info_pgs1_str), FONT_BUILTIN5X8);
    gslc_ElemSetTxtAlign            (&gslc, pElemRef, GSLC_ALIGN_MID_MID);
    gslc_ElemSetTxtCol              (&gslc, pElemRef, COL_TEXT_NORMAL);
    gslc_ElemSetCol                 (&gslc, pElemRef, COL_ERROR, INFO_PGS_FILL_COL, INFO_PGS_FILL_COL);
    info_pgs1_ref = pElemRef;

    pElemRef = gslc_ElemXProgressCreate(&gslc, INFO_PGS2_ELEM, PAGE_INFO, &info_pgs2_elem, INFO_PGS2_RECT, 0, INFO_PGS2_MAX, 0, INFO_PGS2_LINE_COL, false);
    gslc_ElemSetCol                 (&gslc, pElemRef, INFO_PGS2_FRAME_COL, INFO_PGS2_FILL_COL, INFO_PGS2_FILL_COL);
    info_pgs2_ref = pElemRef;

    pElemRef = gslc_ElemCreateTxt   (&gslc, INFO_PGS3_ELEM, PAGE_INFO, INFO_PGS3_RECT, info_pgs3_str, sizeof(info_pgs3_str), FONT_BUILTIN5X8);
    gslc_ElemSetTxtAlign            (&gslc, pElemRef, GSLC_ALIGN_MID_MID);
    gslc_ElemSetTxtCol              (&gslc, pElemRef, COL_TEXT_NORMAL);
    gslc_ElemSetCol                 (&gslc, pElemRef, COL_ERROR, INFO_PGS_FILL_COL, INFO_PGS_FILL_COL);
    info_pgs3_ref = pElemRef;

    // lines
    int i;
    int16_t y = INFO_LINE_Y;
    for (i = 0; i < INFO_LINES; i++)
    {
        pElemRef = gslc_ElemCreateTxt(&gslc, INFO_PATH_ELEM+i, PAGE_INFO, INFO_LINE_RECT, info_lines_str[i], sizeof(info_lines_str[i]), FONT_BUILTIN5X8);
        gslc_ElemSetTxtCol          (&gslc, pElemRef, COL_TEXT_NORMAL);
        gslc_ElemSetCol             (&gslc, pElemRef, COL_ERROR, info_colors[i], info_colors[i]);
        //gslc_ElemSetTxtMarginXY     (&gslc, pElemRef, 2, 2);
        info_lines_ref[i] = pElemRef;

        pElemRef = gslc_ElemCreateImg(&gslc, INFO_PATH_ICON+i, PAGE_INFO, INFO_ICON_RECT,
            gslc_GetImageFromProg((const unsigned char*)icons[ICON_PATH+i], GSLC_IMGREF_FMT_BMP24)); 
        gslc_ElemSetCol             (&gslc, pElemRef, COL_ERROR, info_colors[i], info_colors[i]);
        info_icons_ref[i] = pElemRef;

        y += INFO_LINE_STEP;
    }
}


//###############################################################
int scroll_rounds[INFO_LINES] = {0};
uint32_t scroll_t0[INFO_LINES];

void Gui::scroll_reset()
{
    int i;
    for (i = 0; i < INFO_LINES; i++)
    {
        gslc_tsElemRef *pElemRef = info_lines_ref[i];
        gslc_tsElem *pElem = pElemRef->pElem;
        if (pElem->scr_pos)
        {
            pElem->scr_pos = 0;
            gslc_ElemSetRedraw(&gslc, pElemRef, GSLC_REDRAW_INC);
        }
        scroll_rounds[i] = 0;
        scroll_t0[i] = millis() + SCROLL_DELAY;
    }
}


void Gui::scroll()
{
    uint32_t t = millis();

    int i;
    for (i = 0; i < INFO_LINES; i++)
    {
        if ((int32_t)(t - scroll_t0[i]) < SCROLL_PERIOD)
            continue;
        scroll_t0[i] = t;        
        
        gslc_tsElemRef *pElemRef = info_lines_ref[i];
        gslc_tsElem *pElem = pElemRef->pElem;

        if (!pElem->txt_fit)
        {
            if (scroll_rounds[i] < 1)
            {
                pElem->scr_pos += SCROLL_STEP;
                gslc_ElemSetRedraw(&gslc, pElemRef, GSLC_REDRAW_INC);
            }
        }
        else
        {
            if (pElem->scr_pos)
            {
                if (scroll_rounds[i] < 1)
                {
                    scroll_rounds[i]++;
                    scroll_t0[i] = t + SCROLL_DELAY;
                }
                else
                {
                    pElem->scr_pos = 0;
                    gslc_ElemSetRedraw(&gslc, pElemRef, GSLC_REDRAW_INC);
                }
            }
        } 
    }
}


//###############################################################
void Gui::fav(int fav_num)
{
    char t[10] = "";
    sprintf(t, "%i", fav_num);
    gslc_ElemSetTxtStr(&gslc, info_fav_ref, t);
}


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


void Gui::step_begin(const char * text)
{
    page(PAGE_INFO);
    gslc_ElemSetTxtStr(&gslc, info_lines_ref[INFO_PATH], text);
    gslc_ElemSetTxtStr(&gslc, info_lines_ref[INFO_FILE], "");
    gslc_ElemSetTxtStr(&gslc, info_lines_ref[INFO_BAND], "");
    gslc_ElemSetTxtStr(&gslc, info_lines_ref[INFO_ARTIST], "");
    gslc_ElemSetTxtStr(&gslc, info_lines_ref[INFO_ALBUM], "");
    gslc_ElemSetTxtStr(&gslc, info_lines_ref[INFO_TITLE], "");
    //redraw();
    loop();
}


void Gui::message(const char * message)
{
    page(PAGE_INFO);
    gslc_ElemSetTxtStr(&gslc, info_lines_ref[INFO_FILE], message);
    redraw();
    loop();
}


void Gui::error(const char * errtxt)
{
    DEBUG("Error: %s\n", errtxt);
    message(errtxt);
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


void Gui::path(const char * text, const char * root)
{
    scroll_reset();
    UNUSED(root);
    gslc_ElemSetTxtStr(&gslc, info_lines_ref[INFO_PATH], text);
}


void Gui::gain(bool gain)
{
    gslc_ElemSetGlow(&gslc, info_mode_icons_ref[INFO_VOLUME_ICON], gain);
}


void Gui::volume(int volume)
{
    char t[20];
    sprintf(t, "%2i", volume);
    gslc_ElemSetTxtStr(&gslc, info_mode_ref, t);
}


void Gui::repeat(bool val)
{
    gslc_ElemSetGlow(&gslc, info_mode_icons_ref[INFO_REPEAT_ICON], val);
}


void Gui::shuffle(bool val)
{
    gslc_ElemSetGlow(&gslc, info_mode_icons_ref[INFO_SHUFFLE_ICON], val);
}


void Gui::alive(bool running)
{
    static int index = ICON_PAUSE;
    if (running)
    {
        index++;
        if (index > ICON_PLAY3)
            index = ICON_PLAY0;
    }
    else
    {
        if (index == ICON_PAUSE)
            return;
        index = ICON_PAUSE;
    }

    gslc_tsImgRef imgref = gslc_GetImageFromProg((const unsigned char*)icons[index], GSLC_IMGREF_FMT_BMP24);
    gslc_ElemSetImage(&gslc, info_mode_icons_ref[INFO_PLAY_ICON], imgref, imgref);
}


void Gui::net(int mode)
{
    int index = ICON_WIFI_OFF + mode;
    gslc_tsImgRef imgref = gslc_GetImageFromProg((const unsigned char*)icons[index], GSLC_IMGREF_FMT_BMP24);
    gslc_ElemSetImage(&gslc, info_mode_icons_ref[INFO_WIFI_ICON], imgref, imgref);
}


void Gui::bluetooth(bool enabled)
{

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


static int box_goto(gslc_tsElemRef * box_ref, gslc_tsElemRef * slider_ref, int16_t index, bool center)
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
// page:files
//###############################################################
gslc_tsElem                 files_elem[FILES_ELEM_MAX];
gslc_tsElemRef              files_ref[FILES_ELEM_MAX];

gslc_tsXListbox             files_box_elem;
gslc_tsElemRef*             files_box_ref   = NULL;

gslc_tsXSlider              files_slider_elem;
gslc_tsElemRef*             files_slider_ref    = NULL;

void page_files_init()
{
    gslc_PageAdd(&gslc, PAGE_FILES, files_elem, FILES_ELEM_MAX, files_ref, FILES_ELEM_MAX);
    files_box_ref   = create_listbox(PAGE_FILES, FILES_BOX_ELEM,    &files_box_elem,    FILES_BACK_COL);
    files_slider_ref = create_slider(PAGE_FILES, FILES_SLIDER_ELEM, &files_slider_elem, FILES_BACK_COL);
}


void Gui::files_highlight(void *gslc, void *pElemRef, int type)
{
    static gslc_tsColor colors_b[] = {FILES_COL_NORMAL_B, FILES_COL_DIR_B, FILES_COL_PLAY_B};
    static gslc_tsColor colors_f[] = {FILES_COL_NORMAL_F, FILES_COL_DIR_F, FILES_COL_PLAY_F};
    gslc_tsElem * elem = &files_elem[FILES_BOX_ELEM-FILES_BOX_ELEM];
    elem->colElemText       = colors_f[type];
    elem->colElemTextGlow   = colors_f[type];
    elem->colElemFill       = colors_b[type];
}


void Gui::files_box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb)
{
    files_box_elem.pfuncXGet = cb;
    files_box_elem.nItemCnt = cnt;
    files_box_elem.nItemCurSel = 0;
    files_box_elem.nItemTop = 0;
    files_box_elem.bNeedRecalc = true;
}


void Gui::files_select(int curfile)
{
    files_sel = box_goto(files_box_ref, files_slider_ref, curfile-1, true) + 1;
}


void Gui::files_seek(int by)
{
    files_sel = box_goto(files_box_ref, files_slider_ref, files_sel-1-by, false) + 1;
}


//###############################################################
// page:dirs
//###############################################################
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


void Gui::dirs_box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb)
{
    dirs_box_elem.pfuncXGet = cb;
    dirs_box_elem.nItemCnt = cnt;
    dirs_box_elem.nItemCurSel = 0;
    dirs_box_elem.nItemTop = 0;
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


void Gui::dirs_select(int dir_num)
{
    dirs_sel = box_goto(dirs_box_ref, dirs_slider_ref, dir_num-1, true) + 1;
}


void Gui::dirs_seek(int by)
{
    dirs_sel = box_goto(dirs_box_ref, dirs_slider_ref, dirs_sel-1-by, false) + 1;
}


//###############################################################
// page:fav
//###############################################################
gslc_tsElem                 fav_elem[FAV_ELEM_MAX];
gslc_tsElemRef              fav_ref[FAV_ELEM_MAX];

gslc_tsXListbox             fav_box_elem;
gslc_tsElemRef*             fav_box_ref     = NULL;

gslc_tsXSlider              fav_slider_elem;
gslc_tsElemRef*             fav_slider_ref  = NULL;

void page_fav_init()
{
    gslc_PageAdd(&gslc, PAGE_FAV, fav_elem, FAV_ELEM_MAX, fav_ref, FAV_ELEM_MAX);
    fav_box_ref   = create_listbox(PAGE_FAV, FAV_BOX_ELEM,    &fav_box_elem,    FAV_BACK_COL);
    fav_slider_ref = create_slider(PAGE_FAV, FAV_SLIDER_ELEM, &fav_slider_elem, FAV_BACK_COL);
}


void Gui::fav_box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb)
{
    fav_box_elem.pfuncXGet = cb;
    fav_box_elem.nItemCnt = cnt;
    fav_box_elem.bNeedRecalc = true;
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


void Gui::fav_select(int fav_num)
{
    fav_sel = box_goto(fav_box_ref, fav_slider_ref, fav_num-1, true) + 1;
}


void Gui::fav_seek(int by)
{
    fav_sel = box_goto(fav_box_ref, fav_slider_ref, fav_sel-1-by, false) + 1;
}


//###############################################################
// page:sys
//###############################################################
gslc_tsElem                 sys_elem[SYS_ELEM_MAX];
gslc_tsElemRef              sys_ref[SYS_ELEM_MAX];

gslc_tsXListbox             sys_box_elem;
gslc_tsElemRef*             sys_box_ref     = NULL;

gslc_tsXSlider              sys_slider_elem;
gslc_tsElemRef*             sys_slider_ref  = NULL;

void page_sys_init()
{
    gslc_PageAdd(&gslc, PAGE_SYS, sys_elem, SYS_ELEM_MAX, sys_ref, SYS_ELEM_MAX);
    sys_box_ref   = create_listbox(PAGE_SYS, SYS_BOX_ELEM,    &sys_box_elem,    SYS_BACK_COL);
    sys_slider_ref = create_slider(PAGE_SYS, SYS_SLIDER_ELEM, &sys_slider_elem, SYS_BACK_COL);
}


void Gui::sys_box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb, GSLC_CB_TICK tick_cb)
{
    sys_box_elem.pfuncXGet = cb;
    sys_box_elem.nItemCnt = cnt;
    sys_box_elem.bNeedRecalc = true;
    gslc_ElemSetTickFunc(&gslc, sys_box_ref, tick_cb);
}


void Gui::sys_set_update()
{
    // DEBUG("Sys set update\n");
    gslc_ElemSetRedraw(&gslc, sys_box_ref, GSLC_REDRAW_FULL);
}


void Gui::sys_seek(int by)
{
    sys_sel = box_goto(sys_box_ref, sys_slider_ref, sys_sel-1-by, false) + 1;
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


