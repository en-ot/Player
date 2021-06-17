#ifndef _FILE_CONTROL_H_
#define _FILE_CONTROL_H_


#include "SD_Libs.h"

#define MAX_FILE_NUMBER 0x7FFFFFFF

#define DIR_DEPTH 10
#define PATHNAME_MAX_LEN 256
#define FILENAME_MAX_LEN 256

extern int rootcnt;
extern int curroot;

int clamp1(int num, int cnt);


class playlist
{
    public:
        playlist();

        void set_root(String path);

        bool find_file(int file_num);
        bool find_dir(int dir_num);

        // size_t curdirname(char * dst, int len);
        // size_t curfilename(char * buf, int len);
        
        size_t file_name(int file_num, char * buf, int len);
        size_t file_dirname(int file_num, char * dst, int len);
        size_t file_pathname(int file_num, char * dst, int len);
        bool file_is_dir(int file_num);

        int dircnt;
        int curdir;

        int filecnt;
        int curfile;

        int level = 0;
        
    private:
        size_t entry_name(File & file, char * buf, int len);
        bool find_file0(int file_num);
        void rewind();
        void scan_files(int *fcnt, int *dcnt);

        String root_path;
        File dirs[DIR_DEPTH];
        File entry;
};


#endif // _FILE_CONTROL_H_
