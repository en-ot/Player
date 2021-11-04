#ifndef _PAGE_FAV_H_
#define _PAGE_FAV_H_


#include "player.h"
#include "player_ctrl.h"


class PageFav : public Page
{
public:
    void init();
    void box();
    void goto_cur();

    int sel();

    class FavPrivate * g;
    class CtrlPageFav * c;
};

extern PageFav page_fav;


#endif // _PAGE_FAV_H_