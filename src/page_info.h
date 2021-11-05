#ifndef _PAGE_INFO_H_
#define _PAGE_INFO_H_

#include "player.h"
#include "player_ctrl.h"


class PageInfo : public Page
{
public:
    void init();

    class FilesPrivate * g;
    class CtrlPageFiles * c;
};

extern PageInfo page_info;


#endif // _PAGE_INFO_H_
