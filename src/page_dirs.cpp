#include <Arduino.h>
#include "debug.h"

#include "strcache.h"

#include "gui.h"
#include "gui_common.h"
#include "gui_dirs.h"

#include "page_dirs.h"


#define DIRS_CACHE_LINES 20


class DirsPrivate
{
public:
    int sel = 1;
    void box(int cnt, GSLC_CB_XLISTBOX_GETITEM cb);
    void highlight(void *gslc, void *pElemRef, int type);
    void select(int dirnum, bool center);

    bool seek(int by);
    bool pgupdn(int by);
    void goto_cur();
    void set_fav();
    void play_sel();

    gslc_tsElem                 elem[DIRS_ELEM_MAX];
    gslc_tsElemRef              ref[DIRS_ELEM_MAX];

    gslc_tsXListbox             box_elem;
    gslc_tsElemRef*             box_ref   = NULL;

    gslc_tsXSlider              slider_elem;
    gslc_tsElemRef*             slider_ref    = NULL;

    StrCache * cache;
};

PageDirs page_dirs;


class CtrlPageDirs : public CtrlPage
{
    void b1_short()         {       player->change_page();}
    bool vol(int change)    {return g->pgupdn(change);}
    void vol_short()        {       g->goto_cur();}
    bool seek(int by)       {return g->seek(by);}
    void seek_long()        {       g->set_fav();}
    void seek_short()       {       g->play_sel();}

    friend class PageDirs;
    DirsPrivate * g;
};



//###############################################################
bool dirs_get_item(void* pvGui, void* pvElem, int16_t nItem, char* pStrItem, uint8_t nStrItemLen)
{
    int dirnum = nItem+1;

    int index = page_dirs.g->cache->get(dirnum);
    int dir_level = 0;
    if (index == CACHE_MISS)
    {
        if (!pl->find_dir(dirnum))
            return false;

        int filenum = pl->curfile;

        char buf[XLISTBOX_MAX_STR] = "# # # # # # # # # # # # # # # ";  // >= DIR_DEPTH*2

        int disp = 0;
        dir_level = pl->level + 1;
        disp = dir_level*2-2;

        pl->file_name(pl->curfile, &buf[disp], sizeof(buf)-disp);
        snprintf(pStrItem, nStrItemLen, "%d-%s", filenum, buf);

        page_dirs.g->cache->put(dirnum, buf, dir_level | (filenum << 16));
    }
    else
    {
        uint32_t flags = page_dirs.g->cache->lines[index].flags;
        int filenum = flags >> 16;
        dir_level = flags & 0xFFFF;
        snprintf(pStrItem, nStrItemLen, "%d-%s", filenum, page_dirs.g->cache->lines[index].txt);
    }

    int type = 0;
    if (dirnum == fc->curdir)  type = 1;
    page_dirs.g->highlight(pvGui, pvElem, type);

    return true;
}


int dirs_file_num(int dirs_sel)
{
    int dirnum = dirs_sel;
    int index = page_dirs.g->cache->get(dirnum);
    if (index == CACHE_MISS)
        return 0;
    int file_num = page_dirs.g->cache->lines[index].flags >> 16;
    return file_num;
}



//###############################################################
// page:dirs
//###############################################################
void PageDirs::init()
{
    g = new DirsPrivate;
    c = new CtrlPageDirs;
    c->g = g;
    player->set_ctrl(PAGE_DIRS, c);

    gslc_PageAdd(&gslc, PAGE_DIRS, g->elem, DIRS_ELEM_MAX, g->ref, DIRS_ELEM_MAX);
    g->box_ref   = create_listbox(PAGE_DIRS, DIRS_BOX_ELEM,    &g->box_elem,    DIRS_BACK_COL);
    g->slider_ref = create_slider(PAGE_DIRS, DIRS_SLIDER_ELEM, &g->slider_elem, DIRS_BACK_COL);
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


void PageDirs::goto_cur()
{
    g->goto_cur();
}


void DirsPrivate::highlight(void *gslc, void *pElemRef, int type)
{
    static gslc_tsColor colors_b[] = {DIRS_COL_NORMAL_B, DIRS_COL_PLAY_B};
    static gslc_tsColor colors_f[] = {DIRS_COL_NORMAL_F, DIRS_COL_PLAY_F};
    gslc_tsElem * elem = &this->elem[DIRS_BOX_ELEM-DIRS_BOX_ELEM];
    elem->colElemText       = colors_f[type];
    elem->colElemTextGlow   = colors_f[type];
    elem->colElemFill       = colors_b[type];
}


void DirsPrivate::select(int dir_num, bool center)
{
    sel = box_goto(box_ref, slider_ref, dir_num-1, center) + 1;
}


bool DirsPrivate::pgupdn(int by)
{
    if (!by)
        return false;
    select(sel - by * LISTBOX_LINES, false);
    return true;
}


bool DirsPrivate::seek(int by)
{
    if (!by)
        return false;
    select(sel - by, false);
    return true;
}


void DirsPrivate::goto_cur()
{
    select(fc->curdir, true);
}


void DirsPrivate::set_fav()
{
    int file_num = dirs_file_num(sel);
    if (!file_num)
        return;

    char path[PATHNAME_MAX_LEN];
    pl->file_dirname(file_num, path, sizeof(path));

    int fav_num = gui->fav_sel;
    fav_set_path(fav_num, path);

    player->ui_page = PAGE_FAV;
    gui->page(player->ui_page);
}


void DirsPrivate::play_sel()
{
    int file_num = dirs_file_num(sel);
    if (!file_num)
        return;

    player->ui_page = PAGE_INFO;
    gui->page(player->ui_page);
    player->play_file_num(file_num, FAIL_NEXT);
}


