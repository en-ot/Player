#ifndef _PAGE_DIRS_H_
#define _PAGE_DIRS_H_

#include "player.h"
#include "player_ctrl.h"


extern CtrlPage * ctrl_page_dirs;


class PageDirs
{
public:
    void box(int dircnt);
    void goto_curdir();
};

extern PageDirs page_dirs;


#endif // _PAGE_DIRS_H_
