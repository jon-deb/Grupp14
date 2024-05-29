#include <SDL.h>
#include <SDL_image.h>
#include "player.h"
#include "player_data.h"
#include "ball.h"
#include <stdlib.h>
#include <stdio.h>
#define BALL_WINDOW_X1 64 //distance from left of window to left of field
#define BALL_WINDOW_X2 1236 //distance from left of window to right of field
#define BALL_WINDOW_Y1 114 //distance from top of window to top of field
#define BALL_WINDOW_Y2 765 //distance from top of window to bottom of field
#define WINDOW_HEIGHT_MARGIN_OFFSET 730 //used to calculate middle of field for player
#define MIDDLE_OF_FIELD_Y 436 //distance from top of window to mid point of field
#define EAST_PLAYER_BORDER 1236 //distance from left of window to eastern player border
#define WEST_PLAYER_BORDER 64 //distance from left of window to western player border
#define NORTH_PLAYER_BORDER 114 //distance from top of window to northern player border
#define SOUTH_PLAYER_BORDER 765 //distance from top of window to souther player border

#define MOVEMENT_SPEED 400

struct player {
    float playerVelocityX, playerVelocityY, speedMultiplier;
    int xPos, yPos, team;
    Ball *pBall;
    PowerUp activePower;
    SDL_TimerID powerUpTimer;
    SDL_Texture *playerTexture;
    SDL_Rect playerRect;
};

Player *createPlayer(SDL_Renderer *pGameRenderer, int w, int h, int playerIndex) {
    Player *pPlayer = malloc(sizeof(Player));
    if (!pPlayer) {
        fprintf(stderr, "Memory allocation failed for Player\n");
        return NULL;
    }
    pPlayer->playerRect.w = 64;
    pPlayer->playerRect.h = 64;
    setStartingPosition(pPlayer, playerIndex, w, h);

    char imagePath[29];
    snprintf(imagePath, sizeof(imagePath), "../lib/resources/player%d.png", playerIndex+1);
    pPlayer->team = playerIndex % 2; //for use with assigning powerups
    pPlayer->activePower = NO_POWERUP;
    pPlayer->speedMultiplier = 1;
    pPlayer->powerUpTimer = SDL_AddTimer(0, removePowerUp, pPlayer);
    
    SDL_Surface *playerSurface = IMG_Load(imagePath);
    if (!playerSurface) {
        fprintf(stderr, "Failed to load player image: %s\n", IMG_GetError());
        free(pPlayer);
        return NULL;
    }

    pPlayer->playerTexture = SDL_CreateTextureFromSurface(pGameRenderer, playerSurface);
    SDL_FreeSurface(playerSurface);
    if (!pPlayer->playerTexture) {
        fprintf(stderr, "Error creating texture: %s\n", SDL_GetError());
        free(pPlayer);
        return NULL;
    }
    return pPlayer;
}

void assignPowerUp(int powerUpValue, Player *pPlayer) {
    if(pPlayer->activePower != NO_POWERUP) return;
    pPlayer->activePower = powerUpValue;
    if(pPlayer->activePower == SPEED_BOOST) pPlayer->speedMultiplier=2;
    if(pPlayer->activePower == FROZEN) resetPlayerSpeed(pPlayer, 1, 1);
    if(pPlayer->powerUpTimer != 0) SDL_RemoveTimer(pPlayer->powerUpTimer);
    pPlayer->powerUpTimer = SDL_AddTimer(3000, removePowerUp, pPlayer);
}

void freezeEnemyPlayer(Player *pPlayer1, Player *pPlayer2) {
    if(pPlayer1->activePower == FREEZE) assignPowerUp(FROZEN, pPlayer2); //player2 state is now frozen
    else if(pPlayer2->activePower == FREEZE) assignPowerUp(FROZEN, pPlayer1); //player1 state is now frozen
}

Uint32 removePowerUp(Uint32 interval, void *param) {
    Player *pPlayer = (Player *)param;
    pPlayer->speedMultiplier = 1;
    pPlayer->activePower = NO_POWERUP;
    return 0;
}

int getCurrentPowerUp(Player *pPlayer) {
    return pPlayer->activePower;
}

void updatePlayerVelocity(Player *pPlayer, float vx, float vy) {
    pPlayer->playerVelocityX = vx;
    pPlayer->playerVelocityY = vy;
}

SDL_Texture *getPlayerTexture(Player *pPlayer) {
    return pPlayer->playerTexture;
}

SDL_Rect getPlayerRect(Player *pPlayer) {
    return pPlayer->playerRect;
}

void updatePlayerVUp(Player *pPlayer) {
    pPlayer->playerVelocityY = -MOVEMENT_SPEED * pPlayer->speedMultiplier;
}

void updatePlayerVDown(Player *pPlayer) {
    pPlayer->playerVelocityY = MOVEMENT_SPEED * pPlayer->speedMultiplier;
}

void updatePlayerVLeft(Player *pPlayer) {
    pPlayer->playerVelocityX = -MOVEMENT_SPEED * pPlayer->speedMultiplier;
}

void updatePlayerVRight(Player *pPlayer) {
    pPlayer->playerVelocityX = MOVEMENT_SPEED * pPlayer->speedMultiplier;
}

void resetPlayerSpeed(Player *pPlayer, int x, int y) {
    if (x == 1) pPlayer->playerVelocityX = 0;
    if (y == 1) pPlayer->playerVelocityY = 0;
}

int getPlayerSpeedY(Player *pPlayer) {
    return pPlayer->playerVelocityY != 0;
}

int getPlayerSpeedX(Player *pPlayer) {
    return pPlayer->playerVelocityX != 0;
}

void updatePlayerPosition(Player *pPlayer, float deltaTime) {
    int newX = pPlayer->playerRect.x + pPlayer->playerVelocityX * deltaTime;
    int newY = pPlayer->playerRect.y + pPlayer->playerVelocityY * deltaTime;
    setPlayerPosition(pPlayer, newX, newY);
}

void setPlayerPosition(Player *pPlayer, int x, int y) {
    pPlayer->playerRect.x = x;
    pPlayer->playerRect.y = y;
}

void setStartingPosition(Player *pPlayer, int playerIndex, int w, int h) {
    pPlayer->playerVelocityX = 0;
    pPlayer->playerVelocityY = 0;
    switch(playerIndex) {
        case 0: pPlayer->playerRect.x = w / 4 - pPlayer->playerRect.w / 2;
                pPlayer->playerRect.y = h/ 2;
        break;
        case 1: pPlayer->playerRect.x = w / 4 - pPlayer->playerRect.w / 2 + (w/2);
                pPlayer->playerRect.y = h/ 2;
        break;
        case 2: pPlayer->playerRect.x = w / 5 - pPlayer->playerRect.w / 2;
                pPlayer->playerRect.y = h / 3;
        break;
        case 3: pPlayer->playerRect.x = w - (w/5);
                pPlayer->playerRect.y = h / 3;
        break;
        
    }
}

void restrictPlayerWithinWindow(Player *pPlayer, int width, int height) {
    if (pPlayer->playerRect.x < 0) { 
        setPlayerPosition(pPlayer, 0, pPlayer->playerRect.y); 
    }
    else if (pPlayer->playerRect.x + pPlayer->playerRect.w < WEST_PLAYER_BORDER) {
        setPlayerPosition(pPlayer, WEST_PLAYER_BORDER - pPlayer->playerRect.w, pPlayer->playerRect.y);
    }
    else if (pPlayer->playerRect.x > EAST_PLAYER_BORDER) {
        setPlayerPosition(pPlayer, EAST_PLAYER_BORDER, pPlayer->playerRect.y);
    }
    if (pPlayer->playerRect.y < 0) {
        setPlayerPosition(pPlayer, pPlayer->playerRect.x, 0);
    }
    else if (pPlayer->playerRect.y + pPlayer->playerRect.h < NORTH_PLAYER_BORDER) {
        setPlayerPosition(pPlayer, pPlayer->playerRect.x, NORTH_PLAYER_BORDER - pPlayer->playerRect.h);
    }
    else if (pPlayer->playerRect.y > SOUTH_PLAYER_BORDER) {
        setPlayerPosition(pPlayer, pPlayer->playerRect.x, SOUTH_PLAYER_BORDER);
    }
}

void getPlayerSendData(Player *pPlayer, PlayerData *pPlayerData){
    pPlayerData->playerVelocityX = pPlayer->playerVelocityX;
    pPlayerData->playerVelocityY = pPlayer->playerVelocityY;
    pPlayerData->yPos = pPlayer->playerRect.y;
    pPlayerData->xPos = pPlayer->playerRect.x;
    pPlayerData->activePower = pPlayer->activePower;
}

void updatePlayerWithRecievedData(Player *pPlayer, PlayerData *pPlayerData){
    pPlayer->playerVelocityX = pPlayerData->playerVelocityX;
    pPlayer->playerVelocityY = pPlayerData->playerVelocityY;
    pPlayer->playerRect.y = pPlayerData->yPos;
    pPlayer->playerRect.x = pPlayerData->xPos;
    pPlayer->activePower = pPlayerData->activePower;
}

void destroyPlayer(Player *pPlayer) {
    if (pPlayer != NULL) {
        if (pPlayer->playerTexture != NULL) {
            SDL_DestroyTexture(pPlayer->playerTexture);
            pPlayer->playerTexture = NULL;
        }
        free(pPlayer);
    }
}

void handlePlayerCollision(Player *pPlayer1, Player *pPlayer2) {
    SDL_Rect rect1 = getPlayerRect(pPlayer1);
    SDL_Rect rect2 = getPlayerRect(pPlayer2);
    
    if (checkCollision(rect1, rect2)) {
        // Calculate overlap in both dimensions
        int overlapX;
        if(rect1.x<rect2.x) overlapX = (rect1.x + rect1.w - rect2.x);
        else overlapX = (rect2.x + rect2.w - rect1.x);

        int overlapY;
        if(rect1.y<rect2.y) 
            overlapY = (rect1.y + rect1.h - rect2.y);
        else 
            overlapY = (rect2.y + rect2.h - rect1.y);

        // Resolve collision based on the lesser overlap
        if (overlapX < overlapY) {
            int shift = overlapX / 2 + 1;  // Added +1 for anti-sticking
            if (rect1.x < rect2.x) {
                setPlayerPosition(pPlayer1, rect1.x - shift, rect1.y);
                setPlayerPosition(pPlayer2, rect2.x + shift, rect2.y);
            } else {
                setPlayerPosition(pPlayer1, rect1.x + shift, rect1.y);
                setPlayerPosition(pPlayer2, rect2.x - shift, rect2.y);
            }
        } else {
            int shift = overlapY / 2 + 1;  // Added +1 for anti-sticking
            if (rect1.y < rect2.y) {
                setPlayerPosition(pPlayer1, rect1.x, rect1.y - shift);
                setPlayerPosition(pPlayer2, rect2.x, rect2.y + shift);
            } else {
                setPlayerPosition(pPlayer1, rect1.x, rect1.y + shift);
                setPlayerPosition(pPlayer2, rect2.x, rect2.y - shift);
            }
        }
    }
}