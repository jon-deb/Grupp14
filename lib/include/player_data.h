#ifndef player_data_h
#define player_data_h

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800
#define MAX_PLAYERS 2

enum gameState{START, ONGOING, GAME_OVER};
typedef enum gameState GameState;

enum clientCommand{READY, UP, DOWN, LEFT, RIGHT, RESET_X_VEL, RESET_Y_VEL};
typedef enum clientCommand ClientCommand;

struct clientData{
    ClientCommand command;
    int clientNumber;
};
typedef struct clientData ClientData;

struct ballData{
    //float x, y, vx, vy;
    //int time;
};

typedef struct ballData BallData;

struct playerData{
    float playerVelocityX, playerVelocityY;
    int xPos, yPos;
    BallData bData;
};
typedef struct playerData PlayerData;   

struct serverData{
    PlayerData players[MAX_PLAYERS];
    int clientNr;
    GameState gState;
};
typedef struct serverData ServerData;

#endif