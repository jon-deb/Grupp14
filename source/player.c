#include <SDL2/SDL.h>
#include <SDL_image.h>
#include "../include/player.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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
    setStartingPosition(pPlayer, playerIndex, w, h);

    char imagePath[22];
    snprintf(imagePath, sizeof(imagePath), "resources/player%d.png", playerIndex+1);
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

void handlePlayerCollision(Player *pPlayer1, Player *pPlayer2) {
    SDL_Rect p1rect = getPlayerRect(pPlayer1);
    SDL_Rect p2rect = getPlayerRect(pPlayer2);
    if(checkCollision(p1rect, p2rect)) {
        //resetPlayerSpeed(pPlayer1, 1, 1);
        //resetPlayerSpeed(pPlayer2, 1, 1);
    }
}

int checkCollision(SDL_Rect rect1, SDL_Rect rect2) {
    if (rect1.y + rect1.h <= rect2.y) return 0; // Bottom is above top
    if (rect1.y >= rect2.y + rect2.h) return 0; // Top is below bottom
    if (rect1.x + rect1.w <= rect2.x) return 0; // Right is left of left
    if (rect1.x >= rect2.x + rect2.w) return 0; // Left is right of right
    return 1;
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

void resetPlayerPos(Player *pPlayer, int playerIndex, int w, int h)
{
    int halfWidth = w / 2;
    if (playerIndex == 0) {
        pPlayer->playerRect.x = halfWidth / 2 - pPlayer->playerRect.w / 2; //mitten av första planhalva
    } else {
        pPlayer->playerRect.x = halfWidth + (halfWidth / 2 - pPlayer->playerRect.w / 2); //mitten av andra planhalva
    }
    pPlayer->playerRect.y = (h - pPlayer->playerRect.h) / 2;
    pPlayer->playerVelocityX = 0;
    pPlayer->playerVelocityY = 0;
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