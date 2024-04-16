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


void updatePlayerVelocity(Player *player, float vx, float vy) {
    player->playerVelocityX = vx;
    player->playerVelocityY = vy;
}

SDL_Texture *getPlayerTexture(Player *player) {
    return player->playerTexture;
}

SDL_Rect getPlayerRect(Player *player) {
    return player->playerRect;
}

void updatePlayerVUp(Player *player) {
    player->playerVelocityY = -MOVEMENT_SPEED;
}

void updatePlayerVDown(Player *player) {
    player->playerVelocityY = MOVEMENT_SPEED;
}

void updatePlayerVLeft(Player *player) {
    player->playerVelocityX = -MOVEMENT_SPEED;
}

void updatePlayerVRight(Player *player) {
    player->playerVelocityX = MOVEMENT_SPEED;
}

void resetPlayerSpeed(Player *player, int x, int y) {
    if (x == 1) player->playerVelocityX = 0;
    if (y == 1) player->playerVelocityY = 0;
}

int getPlayerSpeedY(Player *player) {
    return player->playerVelocityY != 0;
}

int getPlayerSpeedX(Player *player) {
    return player->playerVelocityX != 0;
}

void updatePlayerPosition(Player *pPlayer, float deltaTime) {
    int newX = pPlayer->playerRect.x + pPlayer->playerVelocityX * deltaTime;
    int newY = pPlayer->playerRect.y + pPlayer->playerVelocityY * deltaTime;
    setPlayerPosition(pPlayer, newX, newY);
}

void setPlayerPosition(Player *player, int x, int y) {
    player->playerRect.x = x;
    player->playerRect.y = y;
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