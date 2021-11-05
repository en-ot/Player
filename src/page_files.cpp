//###############################################################
// page:files
//###############################################################

#include <Arduino.h>
#include "debug.h"

#include "strcache.h"

#include "gui.h"
#include "gui_common.h"
#include "gui_files.h"

#include "page_fav.h"

#include "page_files.h"


#define FILES_CACHE_LINES 20


class FilesPrivate
{
public:
    int sel = 1;
    bool seek(int by);
    void box(int cnt);
    void select(int curfile, bool center);
    void highlight(void *m_gui, void *pElemRef, int type);

    bool pgupdn(int by);
    void dir_prev();
    void dir_next();
    void play_sel();
    void set_fav();

    gslc_tsElem                 elem[FILES_ELEM_MAX];
    gslc_tsElemRef              files_ref[FILES_ELEM_MAX];

    gslc_tsXListbox             box_elem;
    gslc_tsElemRef*             box_ref   = nullptr;

    gslc_tsXSlider              slider_elem;
    gslc_tsElemRef*             slider_ref    = nullptr;

    StrCache * cache = nullptr;
};


PageFiles page_files;


class CtrlPageFiles : public CtrlPage
{
public:
    void b1_short()         {       player->next_page();}
    bool vol(int change)    {return g->pgupdn(change);}
    void vol_short()        {       p->goto_cur();}
    bool seek(int by)       {return g->seek(by);}
    void seek_long()        {       g->set_fav();}
    void seek_short()       {       g->play_sel();}
    void b2_short()         {       g->dir_prev();}
    void b3_short()         {       g->dir_next();}

    FilesPrivate * g;
    PageFiles * p;
};



//###############################################################
void FilesPrivate::select(int curfile, bool center)
{
    sel = box_goto(box_ref, slider_ref, curfile-1, center) + 1;
}


bool FilesPrivate::seek(int by)
{
    if (!by) 
        return false;
    select(sel - by, false);
    return true;
}


bool FilesPrivate::pgupdn(int by)
{
    if (!by)
        return false;
    select(sel - by * LISTBOX_LINES, false);
    return true;
}


void FilesPrivate::dir_prev()
{
    pl->find_file(sel);
    pl->find_dir(pl->curdir-1);
    //DEBUG("goto %d\n", pl->curfile);
    select(pl->curfile, true);
}


void FilesPrivate::dir_next()
{
    pl->find_file(sel);
    pl->find_dir(pl->curdir+1);
    //DEBUG("goto %d\n", pl->curfile);
    select(pl->curfile, true);
}


void PageFiles::goto_cur()
{
    g->select(fc->curfile, true);
}


void FilesPrivate::play_sel()
{
    player->set_page(PAGE_INFO);
    player->play_file_num(sel, FAIL_NEXT);
}


void FilesPrivate::set_fav()
{
    if (!pl->file_is_dir(sel))
        return;

    char path[PATHNAME_MAX_LEN];
    pl->file_dirname(sel, path, sizeof(path));

    int fav_num = page_fav.sel();
    page_fav.set_path(fav_num, path);

    player->set_page(PAGE_FAV);
}


bool files_get_item(void* pvGui, void* pvElem, int16_t nItem, char* pStrItem, uint8_t nStrItemLen)
{
    int filenum = nItem+1;

    int index = page_files.g->cache->get(filenum);
    int dir_level = 0;
    if (index == CACHE_MISS)
    {
        if (!pl->find_file(filenum))
            return false;

        char buf[XLISTBOX_MAX_STR] = "# # # # # # # # # # # # # # # ";  // >= DIR_DEPTH*2

        int disp = 0;
        if (pl->file_is_dir(filenum))
        {
            dir_level = pl->level + 1;
            disp = dir_level*2-2;
        }

        pl->file_name(filenum, &buf[disp], sizeof(buf)-disp);
        snprintf(pStrItem, nStrItemLen, "%d-%s", filenum, buf);

        page_files.g->cache->put(filenum, buf, dir_level);
    }
    else
    {
        snprintf(pStrItem, nStrItemLen, "%d-%s", filenum, page_files.g->cache->lines[index].txt);
        dir_level = page_files.g->cache->lines[index].flags;
    }
    if (nItem == 0)
    {
        int p = strlen(pStrItem);
        snprintf(&pStrItem[p], nStrItemLen-p, " [%d]", pl->filecnt);
    }

    int type = 0;
    if (filenum == fc->curfile) type = 2;
    else if (dir_level)         type = 1;
    page_files.g->highlight(pvGui, pvElem, type);

    return true;
}


void PageFiles::box(int cnt)
{
    g->box(cnt);
}


void FilesPrivate::box(int cnt)
{
    box_elem.pfuncXGet = files_get_item;
    box_elem.nItemCnt = cnt;
    box_elem.nItemCurSel = 0;
    box_elem.nItemTop = 0;
    box_elem.bNeedRecalc = true;

    if (!cache)
        cache = new StrCache(FILES_CACHE_LINES);
    else
        cache->clear();
}


void PageFiles::init()
{
    g = new FilesPrivate;
    c = new CtrlPageFiles;
    c->g = g;
    c->p = this;
    player->set_ctrl(PAGE_FILES, c);

    gslc_PageAdd(&gslc, PAGE_FILES, g->elem, FILES_ELEM_MAX, g->files_ref, FILES_ELEM_MAX);
    g->box_ref   = create_listbox(PAGE_FILES, FILES_BOX_ELEM,    &g->box_elem,    FILES_BACK_COL);
    g->slider_ref = create_slider(PAGE_FILES, FILES_SLIDER_ELEM, &g->slider_elem, FILES_BACK_COL);
}


void FilesPrivate::highlight(void *gslc, void *pElemRef, int type)
{
    static gslc_tsColor colors_b[] = {FILES_COL_NORMAL_B, FILES_COL_DIR_B, FILES_COL_PLAY_B};
    static gslc_tsColor colors_f[] = {FILES_COL_NORMAL_F, FILES_COL_DIR_F, FILES_COL_PLAY_F};
    gslc_tsElem * elem = &this->elem[FILES_BOX_ELEM-FILES_BOX_ELEM];
    elem->colElemText       = colors_f[type];
    elem->colElemTextGlow   = colors_f[type];
    elem->colElemFill       = colors_b[type];
}

