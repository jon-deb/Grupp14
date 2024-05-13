#ifndef player_data_h
#define player_data_h

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800
#define MAX_PLAYERS 2

enum gameState{START, ONGOING, GAME_OVER};
typedef enum gameState GameState;

enum clientCommand{READY, UP, DOWN, LEFT, RIGHT, RESET_X_VEL, RESET_Y_VEL, RESTRICT_PLAYER};
typedef enum clientCommand ClientCommand;

enum powerUp{NO_POWERUP, SPEED_BOOST, FREEZE, SLOW_DOWN_OPPONENTS};
typedef enum powerUp PowerUp;

typedef struct clientData{
    ClientCommand command;
    int clientNumber;
} ClientData;

typedef struct ballData{
    float velocityX, velocityY;
    int x, y;
} BallData;

typedef struct playerData{
    float playerVelocityX, playerVelocityY;
    int xPos, yPos;
} PlayerData;   

typedef struct serverData{
    PlayerData players[MAX_PLAYERS];
    BallData ball;
    int clientNr;
    GameState gState;
} ServerData;

#endif