#ifndef player
#define player

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 800
#define PLAYER_MAX 6

enum gameState {
    MENU = 0,           
    PLAYING = 1,        //tills tiden är ute
    GAMEOVER = 2,       //return to menu
    LOCALPLAY = 3,      //spela solo för testning
}; typedef enum gameState GameState;

enum clientCommand{READY, ACC, LEFT, RIGHT, FIRE};
typedef enum clientCommand ClientCommand;

struct clientData{
    ClientCommand command;
    int playerNumber;
};
typedef struct clientData ClientData;

struct ballData{
    float x, y, vx, vy;
};
typedef struct ballData BallData;

struct PlayerData{
    float x, y, vx, vy;
   PlayerData pData;
};
typedef struct rocketData RocketData;   

struct serverData{
    PlayerData players[MAX_PLAYERS];
    int playerNr;
    GameState gState;
};
typedef struct serverData ServerData;

#endif