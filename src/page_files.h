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

    class FilesPrivate * g;
    class CtrlPageFiles * c;
};

extern PageFiles page_files;


#endif // _PAGE_FILES_H_
