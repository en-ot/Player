//###############################################################
// page:info
//###############################################################

#include <Arduino.h>
#include "debug.h"

#include "elem/XProgress.h"

#include "sound.h"

#include "gui.h"
#include "gui_common.h"
#include "gui_info.h"

#include "page_info.h"

#include "gui_icons.h"


class PageInfoPrivate
{
public:
    void init();

    gslc_tsColor                colors[INFO_LINES];

    gslc_tsElem                 elem[INFO_ELEM_MAX];
    gslc_tsElemRef              ref[INFO_ELEM_MAX];

    char                        mode_str[20] = "";
    gslc_tsElemRef*             mode_ref = NULL;
    gslc_tsElemRef*             mode_icons_ref[INFO_MODE_ICONS] = {0};

    char                        fav_str[10] = "";
    gslc_tsElemRef*             fav_ref = NULL;

    char                        index_str[INFO_INDEX_MAX_STR] = {0};
    gslc_tsElemRef*             index_ref = NULL;

    char                        pgs1_str[10] = {0};
    gslc_tsElemRef*             pgs1_ref = NULL;

    gslc_tsXProgress            pgs2_elem;
    gslc_tsElemRef*             pgs2_ref  = NULL;

    char                        pgs3_str[10] = {0};
    gslc_tsElemRef*             pgs3_ref = NULL;

    char                        lines_str[INFO_LINES][INFO_LINE_MAX_STR];
    gslc_tsElemRef*             lines_ref[INFO_LINES] = {0};
    gslc_tsElemRef*             icons_ref[INFO_LINES] = {0};

    Gui * gui;

    QueueHandle_t queue;
};


class PageInfoCtrl : public CtrlPage
{
public:
    void b1_short()         {       player->page_next();}
    bool vol(int change)    {return player->change_volume(change);}
    void vol_short()        {       player->play_file_next();}
    void vol_long()         {       player->play_file_prev();}
    bool seek(int by)       {return player->file_seek(by);}
    void seek_short()       {       player->toggle_pause();}
    void seek_long()        {       player->fav_next();}
    void b1_long()          {       player->play_file_down();}
    void b2_long()          {       player->toggle_shuffle();}
    void b2_short()         {       player->play_dir_prev();}
    void b3_long()          {       player->toggle_repeat();}
    void b3_short()         {       player->play_dir_next();}

    PageInfoPrivate * g;
    PageInfo * p;
};


const unsigned short * icons[ICONS_TOTAL] GSLC_PMEM = {
    icon_pause, icon_disk0, icon_disk1, icon_disk2, icon_disk3, icon_fav, icon_index, 
    icon_wifi_off, icon_wifi_on, icon_tether, icon_ftp, icon_upgrade, icon_write, icon_read, icon_error, 
    icon_bluetooth_off, icon_bluetooth_on, 
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
    gslc_ElemSetTxtAlign            (&gslc, pElemRef, GSLC_ALIGN_MID_LEFT);
    gslc_ElemSetRoundEn             (&gslc, pElemRef, true);
    x += rect.w + gap;
    return pElemRef;
}


void PageInfo::activate()
{
    gslc_SetBkgndColor(&gslc, INFO_BACK_COL);
    // DEBUG("INFO ACTIVATED\n");
}


void PageInfo::set_queue(QueueHandle_t tag_queue)
{
    g->queue = tag_queue;
}


PageInfo::PageInfo(Gui * gui)
{
    g = new PageInfoPrivate;
    g->gui = gui;

    auto c = new PageInfoCtrl;
    c->g = g;
    c->p = this;
    ctrl = c;
//    player->set_page(PAGE_INFO, this);
    // DEBUG("%X = %X\n", ctrl, this->ctrl);

    g->init();
}


void PageInfoPrivate::init()
{
    gslc_PageAdd(&gslc, PAGE_INFO, elem, INFO_ELEM_MAX, ref, INFO_ELEM_MAX);
    gslc_tsElemRef* pElemRef = NULL;

    gslc_tsColor col[INFO_LINES] = {COL_BLUE_DARK, COL_BLUE_DARK, COL_RED_DARK, COL_RED_DARK, COL_RED_DARK, COL_RED_DARK};
    memcpy(colors, col, sizeof(colors));

    int r = 0;
    int16_t x = 0;

    mode_icons_ref[r++] = create_icon(INFO_PLAY_ICON, ICON_PAUSE, ICON_PAUSE, x, INFO_GAP);

    mode_icons_ref[r++] = create_icon(INFO_SHUFFLE_ICON, ICON_SHUFFLE_OFF, ICON_SHUFFLE_ON, x, INFO_GAP);
 
    mode_icons_ref[r++] = create_icon(INFO_REPEAT_ICON, ICON_REPEAT_OFF, ICON_REPEAT_ON, x, INFO_GAP);

    mode_icons_ref[r++] = create_icon(INFO_VOLUME_ICON, ICON_VOLUME_NOGAIN, ICON_VOLUME_GAIN, x, 0);
    mode_ref = create_text(INFO_VOLUME_ELEM, INFO_VOLUME_RECT, mode_str, sizeof(mode_str), x, INFO_GAP);

    // fav
    mode_icons_ref[r++] = create_icon(INFO_FAV_ICON, ICON_FAV, ICON_FAV, x, 0);
    fav_ref = create_text(INFO_FAV_ELEM, INFO_FAV_RECT, fav_str, sizeof(fav_str), x, INFO_GAP);

    // index
    mode_icons_ref[r++] = create_icon(INFO_INDEX_ICON, ICON_INDEX, ICON_INDEX, x, 0);
    index_ref = create_text(INFO_INDEX_ELEM, INFO_INDEX_RECT, index_str, sizeof(index_str), x, INFO_GAP);

    // wifi
    mode_icons_ref[r++] = create_icon(INFO_WIFI_ICON, ICON_WIFI_OFF, ICON_WIFI_OFF, x, 0);

    // progress
    pElemRef = gslc_ElemCreateTxt   (&gslc, INFO_PGS1_ELEM, PAGE_INFO, INFO_PGS1_RECT, pgs1_str, sizeof(pgs1_str), FONT_BUILTIN5X8);
    gslc_ElemSetTxtAlign            (&gslc, pElemRef, GSLC_ALIGN_MID_MID);
    gslc_ElemSetTxtCol              (&gslc, pElemRef, COL_TEXT_NORMAL);
    gslc_ElemSetCol                 (&gslc, pElemRef, COL_ERROR, INFO_PGS_FILL_COL, INFO_PGS_FILL_COL);
    gslc_ElemSetRoundEn             (&gslc, pElemRef, true);
    pgs1_ref = pElemRef;

    pElemRef = gslc_ElemXProgressCreate(&gslc, INFO_PGS2_ELEM, PAGE_INFO, &pgs2_elem, INFO_PGS2_RECT, 0, INFO_PGS2_MAX, 0, INFO_PGS2_LINE_COL, false);
    gslc_ElemSetCol                 (&gslc, pElemRef, INFO_PGS2_FRAME_COL, INFO_PGS2_FILL_COL, INFO_PGS2_FILL_COL);
    pgs2_ref = pElemRef;

    pElemRef = gslc_ElemCreateTxt   (&gslc, INFO_PGS3_ELEM, PAGE_INFO, INFO_PGS3_RECT, pgs3_str, sizeof(pgs3_str), FONT_BUILTIN5X8);
    gslc_ElemSetTxtAlign            (&gslc, pElemRef, GSLC_ALIGN_MID_MID);
    gslc_ElemSetTxtCol              (&gslc, pElemRef, COL_TEXT_NORMAL);
    gslc_ElemSetCol                 (&gslc, pElemRef, COL_ERROR, INFO_PGS_FILL_COL, INFO_PGS_FILL_COL);
    gslc_ElemSetRoundEn             (&gslc, pElemRef, true);
    pgs3_ref = pElemRef;

    // lines
    int i;
    int16_t y = INFO_LINE_Y;
    for (i = 0; i < INFO_LINES; i++)
    {
        lines_str[i][0] = 0;

        pElemRef = gslc_ElemCreateTxt(&gslc, INFO_PATH_ELEM+i, PAGE_INFO, INFO_LINE_RECT, lines_str[i], sizeof(lines_str[i]), FONT_BUILTIN5X8);
        gslc_ElemSetTxtCol          (&gslc, pElemRef, COL_TEXT_NORMAL);
        gslc_ElemSetCol             (&gslc, pElemRef, COL_ERROR, colors[i], colors[i]);
        //gslc_ElemSetTxtMarginXY     (&gslc, pElemRef, 2, 2);
        gslc_ElemSetRoundEn             (&gslc, pElemRef, true);
        lines_ref[i] = pElemRef;

        pElemRef = gslc_ElemCreateImg(&gslc, INFO_PATH_ICON+i, PAGE_INFO, INFO_ICON_RECT,
            gslc_GetImageFromProg((const unsigned char*)icons[ICON_PATH+i], GSLC_IMGREF_FMT_BMP24)); 
        gslc_ElemSetCol             (&gslc, pElemRef, COL_ERROR, colors[i], colors[i]);
        icons_ref[i] = pElemRef;

        y += INFO_LINE_STEP;
    }
}


//###############################################################
int scroll_rounds[INFO_LINES] = {0};
uint32_t scroll_t0[INFO_LINES];

void PageInfo::scroll_reset()
{
    int i;
    for (i = 0; i < INFO_LINES; i++)
    {
        gslc_tsElemRef *pElemRef = g->lines_ref[i];
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


void PageInfo::scroll()
{
    uint32_t t = millis();

    int i;
    for (i = 0; i < INFO_LINES; i++)
    {
        if ((int32_t)(t - scroll_t0[i]) < SCROLL_PERIOD)
            continue;
        scroll_t0[i] = t;        
        
        gslc_tsElemRef *pElemRef = g->lines_ref[i];
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
void PageInfo::update()
{
    fav(player->cur_fav_num);
    shuffle(player->shuffle);
    repeat(player->repeat);
    volume(player->volume);
    gui->redraw();
}


void PageInfo::fav(int fav_num)
{
    char t[10] = "";
    sprintf(t, "%i", fav_num);
    gslc_ElemSetTxtStr(&gslc, g->fav_ref, t);
}


void PageInfo::index(const char * text)
{
    gslc_ElemSetTxtStr(&gslc, g->index_ref, text);
}


void PageInfo::step_progress(uint32_t pos, uint32_t total)
{
    char t[10] = "";

    sprintf(t, "%i", pos);
    gslc_ElemSetTxtStr(&gslc, g->pgs1_ref, t);

    sprintf(t, "%i", total);
    gslc_ElemSetTxtStr(&gslc, g->pgs3_ref, t);

    int wdt = total ? INFO_PGS2_MAX * pos / total : 0;
    gslc_ElemXProgressSetVal(&gslc, g->pgs2_ref, wdt);
}


void PageInfo::step_begin(const char * text)
{
    player->page_change(PAGE_INFO);
    auto ref = g->lines_ref;
    gslc_ElemSetTxtStr(&gslc, ref[INFO_PATH], text);
    gslc_ElemSetTxtStr(&gslc, ref[INFO_FILE], "");
    gslc_ElemSetTxtStr(&gslc, ref[INFO_BAND], "");
    gslc_ElemSetTxtStr(&gslc, ref[INFO_ARTIST], "");
    gslc_ElemSetTxtStr(&gslc, ref[INFO_ALBUM], "");
    gslc_ElemSetTxtStr(&gslc, ref[INFO_TITLE], "");
    //redraw();
    gui->loop();
}


void PageInfo::message(const char * message)
{
    player->page_change(PAGE_INFO);
    gslc_ElemSetTxtStr(&gslc, g->lines_ref[INFO_FILE], message);
    gui->redraw();
    gui->loop();
}


void PageInfo::error(const char * errtxt)
{
    DEBUG("Error: %s\n", errtxt);
    message(errtxt);
}


void PageInfo::time_progress(uint32_t pos, uint32_t total)
{
    char t[15] = "";

    sprintf(t, "%i:%02i", pos/60, pos%60);
    gslc_ElemSetTxtStr(&gslc, g->pgs1_ref, t);

    sprintf(t, "%i:%02i", total/60, total%60);
    gslc_ElemSetTxtStr(&gslc, g->pgs3_ref, t);

    int wdt = total ? INFO_PGS2_MAX * pos / total : 0;
    gslc_ElemXProgressSetVal(&gslc, g->pgs2_ref, wdt);
}


void PageInfo::band(const char * text)
{
    gslc_ElemSetTxtStr(&gslc, g->lines_ref[INFO_BAND], text);
}


void PageInfo::artist(const char * text)
{
    gslc_ElemSetTxtStr(&gslc, g->lines_ref[INFO_ARTIST], text);
}


void PageInfo::album(const char * text)
{
    gslc_ElemSetTxtStr(&gslc, g->lines_ref[INFO_ALBUM], text);
}


void PageInfo::title(const char * text)
{
    gslc_ElemSetTxtStr(&gslc, g->lines_ref[INFO_TITLE], text);
}


void PageInfo::file(const char * text)
{
    gslc_ElemSetTxtStr(&gslc, g->lines_ref[INFO_FILE], text);
}


void PageInfo::path(const char * text, const char * root)
{
    scroll_reset();
    UNUSED(root);
    gslc_ElemSetTxtStr(&gslc, g->lines_ref[INFO_PATH], text);
}


void PageInfo::gain(bool gain)
{
    gslc_ElemSetGlow(&gslc, g->mode_icons_ref[INFO_VOLUME_ICON], gain);
}


void PageInfo::volume(int volume)
{
    char t[20];
    sprintf(t, "%2i", volume);
    gslc_ElemSetTxtStr(&gslc, g->mode_ref, t);
}


void PageInfo::repeat(bool val)
{
    gslc_ElemSetGlow(&gslc, g->mode_icons_ref[INFO_REPEAT_ICON], val);
}


void PageInfo::shuffle(bool val)
{
    gslc_ElemSetGlow(&gslc, g->mode_icons_ref[INFO_SHUFFLE_ICON], val);
}


void PageInfo::alive(bool running)
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
    gslc_ElemSetImage(&gslc, g->mode_icons_ref[INFO_PLAY_ICON], imgref, imgref);
}


void PageInfo::net(int mode)
{
    int index = ICON_WIFI_OFF + mode;
    gslc_tsImgRef imgref = gslc_GetImageFromProg((const unsigned char*)icons[index], GSLC_IMGREF_FMT_BMP24);
    gslc_ElemSetImage(&gslc, g->mode_icons_ref[INFO_WIFI_ICON], imgref, imgref);
}


void PageInfo::bluetooth(bool enabled)
{

}


//###############################################################
// Display : Main Task
//###############################################################
static bool cmp(char * info, const char * tst, char ** p)
{
    int len = strlen(tst);
    if (strncmp(info, tst, len))
        return false;

    *p = &info[len];
//    DEBUG("%s: %s", tst, p); 
    return true;
}


void PageInfo::loop2()
{
    char msg[QUEUE_MSG_SIZE];
    BaseType_t res = xQueueReceive(g->queue, &msg, 0);
    msg[QUEUE_MSG_SIZE-1] = 0;
    if (res == pdTRUE)
    {
        DEBUG("id3data[%i]: %s\n", strlen(msg), msg);
        //DEBUG_DUMP8(msg, len, len);
//        DEBUG("\n");

        char * p;
             if (cmp(msg, "Artist: ", &p))    artist(p);
        else if (cmp(msg, "Band: ", &p))      band(p);
        else if (cmp(msg, "Album: ", &p))     album(p);
        else if (cmp(msg, "Title: ", &p))     title(p);
        else if (cmp(msg, "File: ", &p))      file(p);
        else if (cmp(msg, "Path: ", &p))      path(p, fc->root_path.c_str());
        else if (cmp(msg, "Index: ", &p))     {index(p); player->update();}
        else return;
        return;
    }
}


void PageInfo::gui_loop()
{
    scroll();

    uint32_t t = millis();

    static uint32_t t1 = 0;
    static uint32_t old_pos = 0;

//    uint32_t pos = sound_is_playing() ? sound_current_time() : old_pos;
    uint32_t pos = sound_current_time();

    if (((int32_t)(t - t1) > 2000) || (pos != old_pos))
    {
        t1 = t;
        old_pos = pos;
        time_progress(pos, sound_duration());
        return;
    }

    static uint32_t t0 = 0;
    if ((int32_t)(t - t0) > 100)
    {
        t0 = t;
        alive(sound_is_playing());
        gain(sound_is_gain());
        return;
    }
}
