#ifndef _PAGE_FAV_H_
#define _PAGE_FAV_H_


#include "player.h"
#include "player_ctrl.h"


class PageFav : public Page
{
public:
    PageFav(Gui * gui);
    void box();
    void goto_cur();
    void activate();

    int sel();
    void set_path(int fav_num, const char * path);
    void reset();

    class PageFavPrivate * g;
};


#endif // _PAGE_FAV_H_
