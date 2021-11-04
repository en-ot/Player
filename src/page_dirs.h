#ifndef _PAGE_DIRS_H_
#define _PAGE_DIRS_H_

#include "player.h"
#include "player_ctrl.h"


class PageDirs : public Page
{
public:
    void init();
    void box(int dircnt);
    void goto_cur();

    class DirsPrivate * g;
    class CtrlPageDirs * c;
};

extern PageDirs page_dirs;


#endif // _PAGE_DIRS_H_
