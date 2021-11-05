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


class PageFavPrivate
{
public:
    int sel = 1;
    void box();
    void select(int fav_num, bool center);
    void highlight(void *m_gui, void *pElemRef, int type);

    bool seek(int by);
    bool pgupdn(int by);
    void set_num();

    gslc_tsElem                 elem[FAV_ELEM_MAX];
    gslc_tsElemRef              ref[FAV_ELEM_MAX];

    gslc_tsXListbox             box_elem;
    gslc_tsElemRef*             box_ref     = NULL;

    gslc_tsXSlider              slider_elem;
    gslc_tsElemRef*             slider_ref  = NULL;

    char cache[FAV_MAX][XLISTBOX_MAX_STR];
    void set_str(int fav_num, const char * path);
};

PageFav page_fav;


class PageFavCtrl : public CtrlPage
{
public:
    void b1_short()         {       player->next_page();}
    void b2_short()         {       player->change_page(PAGE_SYS);}
    bool vol(int change)    {return g->pgupdn(change);}
    void vol_short()        {       p->goto_cur();}
    bool seek(int by)       {return g->seek(by);}
    void seek_long()        {       p->reset();}
    void seek_short()       {       g->set_num();}

    PageFavPrivate * g;
    PageFav * p;
};


//###############################################################
void PageFavPrivate::set_str(int fav_num, const char * path)
{
    int nItem = fav_num - 1;
    sprintf(cache[nItem], "%d ", fav_num);
    strlcat(cache[nItem], path, sizeof(cache[nItem]));
}


void PageFav::set_path(int fav_num, const char * path)
{
    g->set_str(fav_num, path);
    prefs_set_path(fav_num, path);
    gui->redraw();
}


void PageFav::reset()
{
    set_path(g->sel, "/");
}


bool fav_get_item(void* pvGui, void* pvElem, int16_t nItem, char* pStrItem, uint8_t nStrItemLen)
{
    int fav_num = nItem + 1;
    
    int type = (fav_num == player->cur_fav_num) ? 1 : 0;
    page_fav.g->highlight(pvGui, pvElem, type);

    strlcpy(pStrItem, page_fav.g->cache[nItem], nStrItemLen);
    //DEBUG("%s\n", pStrItem);
    return true;
}


int PageFav::sel()
{
    return g->sel;
}


void PageFav::init()
{
    g = new PageFavPrivate;
    auto c = new PageFavCtrl;
    c->g = g;
    c->p = this;
    ctrl = c;
    player->set_page(PAGE_FAV, this);

    gslc_PageAdd(&gslc, PAGE_FAV, g->elem, FAV_ELEM_MAX, g->ref, FAV_ELEM_MAX);
    g->box_ref   = create_listbox(PAGE_FAV, FAV_BOX_ELEM,    &g->box_elem,    FAV_BACK_COL);
    g->slider_ref = create_slider(PAGE_FAV, FAV_SLIDER_ELEM, &g->slider_elem, FAV_BACK_COL);
}


void PageFavPrivate::select(int fav_num, bool center)
{
    sel = box_goto(box_ref, slider_ref, fav_num-1, center) + 1;
}


bool PageFavPrivate::seek(int by)
{
    if (!by)
        return false;
    select(sel - by, false);
    return true;
}


bool PageFavPrivate::pgupdn(int by)
{
    if (!by)
        return false;
    select(sel - by * LISTBOX_LINES, false);
    return true;
}


void PageFavPrivate::set_num()
{
    player->fav_switch(sel, false);
    player->change_page(PAGE_INFO);
}


void PageFav::goto_cur()
{
    g->select(player->cur_fav_num, true);
}


void PageFav::box()
{
    g->box();
}


void PageFavPrivate::box()
{
    char tmp[XLISTBOX_MAX_STR];
    int fav_num;
    for (fav_num = 1; fav_num <= FAV_MAX; fav_num++)
    {
        prefs_get_path(fav_num, tmp, sizeof(tmp));
        if (!tmp[0]) strcpy(tmp, "/");
        set_str(fav_num, tmp);
    }

    box_elem.pfuncXGet = fav_get_item;
    box_elem.nItemCnt = FAV_MAX;
    box_elem.bNeedRecalc = true;
}


void PageFavPrivate::highlight(void *gslc, void *pElemRef, int type)
{
    static gslc_tsColor colors_b[] = {FAV_COL_NORMAL_B, FAV_COL_PLAY_B};
    static gslc_tsColor colors_f[] = {FAV_COL_NORMAL_F, FAV_COL_PLAY_F};
    gslc_tsElem * elem = &this->elem[FAV_BOX_ELEM-FAV_BOX_ELEM];
    elem->colElemText       = colors_f[type];
    elem->colElemTextGlow   = colors_f[type];
    elem->colElemFill       = colors_b[type];
}


