#ifndef _PAGE_DIRS_H_
#define _PAGE_DIRS_H_

#include "player.h"
#include "player_ctrl.h"


class PageDirs : public Page
{
public:
    PageDirs(Gui * gui);
    void box(int dircnt);
    void goto_cur();
    void go_to(int line);
    void activate();
    void update();

    class PageDirsPrivate * g;
};


#endif // _PAGE_DIRS_H_
