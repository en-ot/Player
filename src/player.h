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


class Player
{
public:
    Player();
    void set_gui(PlayerGui * gui);
    
    void key_pressed(PlayerKey key, int param=0);

private:
    PlayerGui * _gui = nullptr;

};

