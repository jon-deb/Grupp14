#include <SDL2/SDL.h>
#include <SDL_image.h>
#include "../include/player.h"
#include <stdlib.h>
#include <stdio.h>

#define MOVEMENT_SPEED 400

struct player {
    float playerVelocityX;
    float playerVelocityY;
    int xPos, yPos;
    SDL_Texture *playerTexture;
    SDL_Rect playerRect;
};

Player *createPlayer(SDL_Renderer *pGameRenderer, int w, int h) {
    Player *pPlayer = malloc(sizeof(Player));
    if (!pPlayer) {
        fprintf(stderr, "Memory allocation failed for Player\n");
        return NULL;
    }
    pPlayer->playerRect.w = 64;
    pPlayer->playerRect.h = 64;
    pPlayer->playerRect.x = (w - pPlayer->playerRect.w) / 2;
    pPlayer->playerRect.y = (h - pPlayer->playerRect.h) / 2;
    pPlayer->playerVelocityX = 0;
    pPlayer->playerVelocityY = 0;

    SDL_Surface *playerSurface = IMG_Load("resources/player.png");
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
    else if (pPlayer->playerRect.x + pPlayer->playerRect.w > width) {
        setPlayerPosition(pPlayer, width - pPlayer->playerRect.w, pPlayer->playerRect.y);
    }
    if (pPlayer->playerRect.y < 0) {
        setPlayerPosition(pPlayer, pPlayer->playerRect.x, 0);
    }
    else if (pPlayer->playerRect.y + pPlayer->playerRect.h > height) {
        setPlayerPosition(pPlayer, pPlayer->playerRect.x, height - pPlayer->playerRect.h);
    }
}