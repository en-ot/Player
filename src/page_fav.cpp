//###############################################################
// page:fav
//###############################################################

#include <Arduino.h>
#include "debug.h"

#include "strcache.h"
#include "prefs.h"
#include "player.h"

#include "gui.h"
#include "gui_common.h"
#include "gui_fav.h"

#include "page_fav.h"


class FavPrivate
{
public:
    int sel = 1;
    void select(int fav_num, bool center);
    void highlight(void *m_gui, void *pElemRef, int type);

    bool seek(int by);
    bool pgupdn(int by);
    void reset();
    void set_num();

    gslc_tsElem                 elem[FAV_ELEM_MAX];
    gslc_tsElemRef              ref[FAV_ELEM_MAX];

    gslc_tsXListbox             box_elem;
    gslc_tsElemRef*             box_ref     = NULL;

    gslc_tsXSlider              slider_elem;
    gslc_tsElemRef*             slider_ref  = NULL;

    StrCache * cache = nullptr;
};

PageFav page_fav;


class CtrlPageFav : public CtrlPage
{
    void b1_short()         {       player->change_page();}
    void b2_short()         {       player->sys_page();}
    bool vol(int change)    {return g->pgupdn(change);}
    void vol_short()        {       page_fav.goto_cur();}
    bool seek(int by)       {return g->seek(by);}
    void seek_long()        {       g->reset();}
    void seek_short()       {       g->set_num();}

    friend class PageFav;
    FavPrivate * g;
} ctrl_page_fav;


//###############################################################
char fav_str[FAV_MAX][XLISTBOX_MAX_STR] = {0};


void fav_set_str(int fav_num, const char * path)
{
    int nItem = fav_num - 1;
    sprintf(fav_str[nItem], "%d ", fav_num);
    strlcat(fav_str[nItem], path, sizeof(fav_str[nItem]));
}


void fav_set_path(int fav_num, const char * path)
{
    fav_set_str(fav_num, path);
    prefs_set_path(fav_num, path);
    gui->redraw();
}


bool fav_get_item(void* pvGui, void* pvElem, int16_t nItem, char* pStrItem, uint8_t nStrItemLen)
{
    int fav_num = nItem + 1;
    
    int type = (fav_num == player->cur_fav_num) ? 1 : 0;
    page_fav.g->highlight(pvGui, pvElem, type);

    strlcpy(pStrItem, fav_str[nItem], nStrItemLen);
    //DEBUG("%s\n", pStrItem);
    return true;
}


int PageFav::sel()
{
    return g->sel;
}


void PageFav::init()
{
    g = new FavPrivate;
    c = new CtrlPageFav;
    c->g = g;
    player->set_ctrl(PAGE_FAV, c);

    gslc_PageAdd(&gslc, PAGE_FAV, g->elem, FAV_ELEM_MAX, g->ref, FAV_ELEM_MAX);
    g->box_ref   = create_listbox(PAGE_FAV, FAV_BOX_ELEM,    &g->box_elem,    FAV_BACK_COL);
    g->slider_ref = create_slider(PAGE_FAV, FAV_SLIDER_ELEM, &g->slider_elem, FAV_BACK_COL);
}


void FavPrivate::select(int fav_num, bool center)
{
    sel = box_goto(box_ref, slider_ref, fav_num-1, center) + 1;
}


bool FavPrivate::seek(int by)
{
    if (!by)
        return false;
    select(sel - by, false);
    return true;
}


bool FavPrivate::pgupdn(int by)
{
    if (!by)
        return false;
    select(sel - by * LISTBOX_LINES, false);
    return true;
}


void FavPrivate::reset()
{
    fav_set_path(sel, "/");
}


void FavPrivate::set_num()
{
    fav_switch(sel, false);
    player->ui_page = PAGE_INFO;
    gui->page(player->ui_page);
}


void PageFav::goto_cur()
{
    g->select(player->cur_fav_num, true);
}


void PageFav::box()
{
    char tmp[XLISTBOX_MAX_STR];
    int fav_num;
    for (fav_num = 1; fav_num <= FAV_MAX; fav_num++)
    {
        prefs_get_path(fav_num, tmp, sizeof(tmp));
        if (!tmp[0]) strcpy(tmp, "/");
        fav_set_str(fav_num, tmp);
    }

    g->box_elem.pfuncXGet = fav_get_item;
    g->box_elem.nItemCnt = FAV_MAX;
    g->box_elem.bNeedRecalc = true;
}


void FavPrivate::highlight(void *gslc, void *pElemRef, int type)
{
    static gslc_tsColor colors_b[] = {FAV_COL_NORMAL_B, FAV_COL_PLAY_B};
    static gslc_tsColor colors_f[] = {FAV_COL_NORMAL_F, FAV_COL_PLAY_F};
    gslc_tsElem * elem = &this->elem[FAV_BOX_ELEM-FAV_BOX_ELEM];
    elem->colElemText       = colors_f[type];
    elem->colElemTextGlow   = colors_f[type];
    elem->colElemFill       = colors_b[type];
}


