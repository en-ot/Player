// Player must contain all business logic, but no depenencies on components

typedef enum 
{
    KEY_VOLUME,           KEY_SEEK,
    KEY_VOLLONG,          KEY_VOLSHORT,
    KEY_SEEKLONG,         KEY_SEEKSHORT, 
    KEY_B1LONG,           KEY_B1SHORT, 
    KEY_B2LONG,           KEY_B2SHORT, 
    KEY_B3LONG,           KEY_B3SHORT,
} PlayerKey;


class PlayerGui
{
public:
    PlayerGui();

private:

};


typedef enum
{
    I_BUTTON,
    I_KEY,
    I_SEEK1, 
    I_SEEK2,
} InputType;


class Player
{
public:
    Player();
    void set_gui(PlayerGui * gui);
    
    bool input(InputType type, int key);

private:
    PlayerGui * _gui = nullptr;

};


//temp
bool input(InputType type, int key);
bool files_goto_curfile();
bool dirs_goto_curdir();
bool fav_goto_curfav();

bool play_file_next();
int file_random();

bool play_dir_next();
bool play_dir_prev();
bool play_root_prev();


