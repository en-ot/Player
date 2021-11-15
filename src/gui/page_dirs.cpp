//###############################################################
// page:dirs
//###############################################################

#include <Arduino.h>

#include "globals.h"
#include "debug.h"

#include "strcache.h"

#include "gui.h"
#include "gui_common.h"
#include "gui_dirs.h"

#include "page_fav.h"
#include "page_dirs.h"


#define DIRS_CACHE_LINES 20


class PageDirsPrivate
{
public:
    int sel = 1;
    void box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb);
    void highlight(void *gslc, void *pElemRef, int type);
    void select(int dirnum, bool center);

    bool seek(int by);
    bool pgupdn(int by);
    void set_fav();
    void play_sel();

    int dir_file_num(int dirs_sel);

    gslc_tsElem                 elem[DIRS_ELEM_MAX];
    gslc_tsElemRef              ref[DIRS_ELEM_MAX];

    gslc_tsXListbox             box_elem;
    gslc_tsElemRef*             box_ref   = nullptr;

    gslc_tsXSlider              slider_elem;
    gslc_tsElemRef*             slider_ref    = nullptr;

    StrCache * cache = nullptr;

    Gui * gui;
    GuiPrivate * p;
    gslc_tsGui * s;
};


class PageDirsCtrl : public CtrlPage
{
public:
    void b1_short()         {       player->page_next();}
    bool vol(int change)    {return g->pgupdn(change);}
    void vol_short()        {       p->goto_cur();}
    bool seek(int by)       {return g->seek(by);}
    void seek_long()        {       g->set_fav();}
    void seek_short()       {       g->play_sel();}

    PageDirsPrivate * g;
    PageDirs * p;
};


//###############################################################
bool dirs_get_item(void* pvGui, void* pvElem, int16_t nItem, char* pStrItem, uint8_t nStrItemLen)
{
    int dirnum = nItem+1;

    PageDirsPrivate * g = ((PageDirs *)*player->page_ptr(PAGE_DIRS))->g;

    int index = g->cache->get(dirnum);
    int dir_level = 0;
    if (index == CACHE_MISS)
    {
        if (!player->set_dir(LIST, dirnum))
            return false;

        int filenum = player->cur_file(LIST);

        char buf[XLISTBOX_MAX_STR] = "# # # # # # # # # # # # # # # ";  // >= DIR_DEPTH*2

        int disp = 0;
        dir_level = player->cur_level(LIST) + 1;
        disp = dir_level*2-2;

        player->file_name(LIST, player->cur_file(LIST), &buf[disp], sizeof(buf)-disp);
        snprintf(pStrItem, nStrItemLen, "%d-%s", filenum, buf);

        g->cache->put(dirnum, buf, dir_level | (filenum << 16));
    }
    else
    {
        uint32_t flags = g->cache->lines[index].flags;
        int filenum = flags >> 16;
        dir_level = flags & 0xFFFF;
        snprintf(pStrItem, nStrItemLen, "%d-%s", filenum, g->cache->lines[index].txt);
    }

    int type = (dirnum == player->cur_dir(PLAYING)) ? 1 : 0;
    g->highlight(pvGui, pvElem, type);

    return true;
}


int PageDirsPrivate::dir_file_num(int dirs_sel)
{
    int dirnum = dirs_sel;
    int index = cache->get(dirnum);
    if (index == CACHE_MISS)
        return 0;
    int file_num = cache->lines[index].flags >> 16;
    return file_num;
}


PageDirs::PageDirs(Gui * gui)
{
    g = new PageDirsPrivate;
    g->gui = gui;
    g->p = gui->p;
    g->s = &gui->p->gslc;

    auto c = new PageDirsCtrl;
    c->g = g;
    c->p = this;
    ctrl = c;

    gslc_PageAdd(g->s, PAGE_DIRS, g->elem, DIRS_ELEM_MAX, g->ref, DIRS_ELEM_MAX);
    g->box_ref   = g->gui->p->create_listbox(PAGE_DIRS, DIRS_BOX_ELEM,    &g->box_elem,    DIRS_BACK_COL);
    g->slider_ref = g->gui->p->create_slider(PAGE_DIRS, DIRS_SLIDER_ELEM, &g->slider_elem, DIRS_BACK_COL);
}


void PageDirs::activate()
{
    gslc_SetBkgndColor(g->s, DIRS_BACK_COL);
}


void PageDirs::update()
{
    g->gui->redraw();
}


void PageDirs::box(int cnt)
{
    g->box_elem.pfuncXGet = dirs_get_item;
    g->box_elem.nItemCnt = cnt;
    g->box_elem.nItemCurSel = 0;
    g->box_elem.nItemTop = 0;
    g->box_elem.bNeedRecalc = true;

    if (!g->cache)
        g->cache = new StrCache(DIRS_CACHE_LINES);
    else
        g->cache->clear();
}


void PageDirsPrivate::highlight(void *gslc, void *pElemRef, int type)
{
    static gslc_tsColor colors_b[] = {DIRS_COL_NORMAL_B, DIRS_COL_PLAY_B};
    static gslc_tsColor colors_f[] = {DIRS_COL_NORMAL_F, DIRS_COL_PLAY_F};
    gslc_tsElem * elem = &this->elem[DIRS_BOX_ELEM-DIRS_BOX_ELEM];
    elem->colElemText       = colors_f[type];
    elem->colElemTextGlow   = colors_f[type];
    elem->colElemFill       = colors_b[type];
}


void PageDirsPrivate::select(int dir_num, bool center)
{
    sel = gui->p->box_goto(box_ref, slider_ref, dir_num-1, center) + 1;
}


bool PageDirsPrivate::pgupdn(int by)
{
    if (!by)
        return false;
    select(sel - by * LISTBOX_LINES, false);
    return true;
}


bool PageDirsPrivate::seek(int by)
{
    if (!by)
        return false;
    select(sel - by, false);
    return true;
}


void PageDirs::goto_cur()
{
    g->select(player->cur_dir(PLAYING), true);
}


void PageDirsPrivate::set_fav()
{
    int file_num = dir_file_num(sel);
    if (!file_num)
        return;

    char path[PATHNAME_MAX_LEN];
    player->dir_name(LIST, file_num, path, sizeof(path));

    player->fav_set(path);
    player->page_change(PAGE_FAV);
}


void PageDirsPrivate::play_sel()
{
    int file_num = dir_file_num(sel);
    if (!file_num)
        return;

    player->page_change(PAGE_INFO);
    player->play_file_num(file_num, FAIL_NEXT);
}


