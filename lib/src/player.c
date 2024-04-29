#include <SDL.h>
#include <SDL_image.h>
#include "player.h"
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
    float playerVelocityX;
    float playerVelocityY;
    int xPos, yPos;
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
    resetPlayerPos(pPlayer, playerIndex, w, h);

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

void destroyPlayer(Player *pPlayer) {
    if (pPlayer != NULL) {
        if (pPlayer->playerTexture != NULL) {
            SDL_DestroyTexture(pPlayer->playerTexture);
            pPlayer->playerTexture = NULL;
        }
        free(pPlayer);
    }
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