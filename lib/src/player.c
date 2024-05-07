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
    float playerVelocityX, playerVelocityY;
    int xPos, yPos;
    Ball *pBall;
    SDL_Texture *playerTexture;
    SDL_Rect playerRect;
    //SDL_Renderer *pGameRenderer; maybe add
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
    pPlayer->playerVelocityY = -MOVEMENT_SPEED;
}

void updatePlayerVDown(Player *pPlayer) {
    pPlayer->playerVelocityY = MOVEMENT_SPEED;
}

void updatePlayerVLeft(Player *pPlayer) {
    pPlayer->playerVelocityX = -MOVEMENT_SPEED;
}

void updatePlayerVRight(Player *pPlayer) {
    pPlayer->playerVelocityX = MOVEMENT_SPEED;
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
    /*spelare 0, 2 och 4 är i samma lag
    spelare 1, 3, och 5 är i samma lag
    lag beror på vilken ordning man joinar*/
    if(playerIndex == 0 || playerIndex == 1) {
        pPlayer->playerRect.x = w / 4 - pPlayer->playerRect.w / 2;
        if(playerIndex == 1) {
            pPlayer->playerRect.x += w/2; 
        }
        pPlayer->playerRect.y = (h - pPlayer->playerRect.h) / 2;
    }
    /*else if(playerIndex == 2 || playerIndex == 3) {
        
        if(playerIndex == 3) {
            
        }
    }
    else if(playerIndex == 4 || playerIndex == 5) {

        if(playerIndex == 5) {

        }
    }*/
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

void resetPlayerPos(Player *pPlayer, int playerIndex, int w, int h)
{
    int halfWidth = w / 2;
    if (playerIndex == 0) {
        pPlayer->playerRect.x = halfWidth / 2 - pPlayer->playerRect.w / 2 + BALL_WINDOW_X1; //mitten av första planhalva
    } else {
        pPlayer->playerRect.x = halfWidth + (halfWidth / 2 - pPlayer->playerRect.w / 2) - BALL_WINDOW_X1; //mitten av andra planhalva
    }
    pPlayer->playerRect.y = MIDDLE_OF_FIELD_Y - (pPlayer->playerRect.h/2);
    pPlayer->playerVelocityX = 0;
    pPlayer->playerVelocityY = 0;
}

void getPlayerSendData(Player *pPlayer, PlayerData *pPlayerData){
    pPlayerData->playerVelocityX = pPlayer->playerVelocityX;
    pPlayerData->playerVelocityY = pPlayer->playerVelocityY;
    pPlayerData->yPos = pPlayer->playerRect.y;
    pPlayerData->xPos = pPlayer->playerRect.x;
}

void updatePlayerWithRecievedData(Player *pPlayer, PlayerData *pPlayerData){
    pPlayer->playerVelocityX = pPlayerData->playerVelocityX;
    pPlayer->playerVelocityY = pPlayerData->playerVelocityY;
    pPlayer->playerRect.y = pPlayerData->yPos;
    pPlayer->playerRect.x = pPlayerData->xPos;
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
        int overlapX = (rect1.x < rect2.x) ? (rect1.x + rect1.w - rect2.x) : (rect2.x + rect2.w - rect1.x);
        int overlapY = (rect1.y < rect2.y) ? (rect1.y + rect1.h - rect2.y) : (rect2.y + rect2.h - rect1.y);

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