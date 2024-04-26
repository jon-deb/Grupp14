#ifndef player
#define player

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 800
#define PLAYER_MAX 6

enum gameState{MENU,LOCALPLAY, CREATETEAMS, PLAYING, GAME_OVER};
typedef enum gameState GameState;

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
    PlayerData players[PLAYER_MAX];
    int playerNr;
    GameState gState;
};
typedef struct serverData ServerData;

#endif