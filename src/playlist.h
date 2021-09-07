#ifndef _FILE_CONTROL_H_
#define _FILE_CONTROL_H_


#define MAX_FILE_NUMBER 0x7FFFFFFF

#define DIR_DEPTH 10
#define PATHNAME_MAX_LEN 256
#define FILENAME_MAX_LEN 256


int clamp1(int num, int cnt);


class Playlist
{
    friend class PlaylistPrivate;

    public:
        Playlist(int thread_id);

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
        
        String root_path;

    private:
        int thread;
        bool find_file0(int file_num);
        void rewind();
        void rewind_to_file(int file_num);
        void rewind_to_dir(int dir_num);
        void scan_files(int *fcnt, int *dcnt);

        class PlaylistPrivate * p;

        //File dirs[DIR_DEPTH];
        //File entry;
};


#endif // _FILE_CONTROL_H_
