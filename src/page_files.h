#ifndef _PAGE_FILES_H_
#define _PAGE_FILES_H_

#include "player.h"
#include "player_ctrl.h"


class PageFiles : public Page
{
public:
    void init();
    void box(int dircnt);
    void goto_cur();

    class PageFilesPrivate * g;
    class PageFilesCtrl * c;
};

extern PageFiles page_files;


#endif // _PAGE_FILES_H_
