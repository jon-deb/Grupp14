#include <SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "player_data.h"
#include "power.h"
#include "ball.h"
#include <SDL_image.h>

#define POWER_WINDOW_X1 64 //distance from left of window to left of field
#define POWER_WINDOW_X2 1236 //distance from left of window to right of field
#define POWER_WINDOW_Y1 114 //distance from top of window to top of field
#define POWER_WINDOW_Y2 765 //distance from top of window to bottom of field


struct powerUpBox {
    SDL_Texture *texture;
    SDL_Rect rect;
    SDL_Surface *surface;
    bool visible;
    SDL_TimerID restartTimerID;
};

PowerUpBox *createPower(SDL_Renderer *renderer) {
    PowerUpBox *pPowerUpBox = malloc(sizeof(PowerUpBox));
    if (!pPowerUpBox) {
        fprintf(stderr, "Failed to allocate memory for pPower.\n");
        return NULL;
    }

    pPowerUpBox->surface = IMG_Load("../lib/resources/power.png");
    if (!pPowerUpBox->surface) {
        fprintf(stderr, "Error loading pPower texture: %s\n", SDL_GetError());
        free(pPowerUpBox);
        return NULL;
    }

    pPowerUpBox->texture = SDL_CreateTextureFromSurface(renderer, pPowerUpBox->surface);
    SDL_FreeSurface(pPowerUpBox->surface);
    if (!pPowerUpBox->texture) {
        fprintf(stderr, "Error creating pPower texture: %s\n", SDL_GetError());
        free(pPowerUpBox);
        return NULL;
    }

    srand(500);
    pPowerUpBox->rect.w = 48;
    pPowerUpBox->rect.h = 48;
    pPowerUpBox->visible = false;

    return pPowerUpBox;
}

void spawnPowerCube(PowerUpBox *pPowerUpBox) {
    if (!pPowerUpBox) return;

    pPowerUpBox->rect.x = POWER_WINDOW_X1 + rand() % (POWER_WINDOW_X2 - POWER_WINDOW_X1 - pPowerUpBox->rect.w);
    pPowerUpBox->rect.y = POWER_WINDOW_Y1 + rand() % (POWER_WINDOW_Y2 - POWER_WINDOW_Y1 - pPowerUpBox->rect.h);
    pPowerUpBox->visible = true;

    if (pPowerUpBox->restartTimerID != 0) {
        SDL_RemoveTimer(pPowerUpBox->restartTimerID);
        pPowerUpBox->restartTimerID = 0;
    }
}

Uint32 respawnPowerCubeCallback(Uint32 interval, void *param) {
    PowerUpBox *pPowerUpBox = (PowerUpBox *)param;
    if(pPowerUpBox) spawnPowerCube(pPowerUpBox);
    return 0;
}

void renderPowerCube(PowerUpBox *pPowerUpBox, SDL_Renderer *renderer) {
    if (pPowerUpBox->visible) SDL_RenderCopy(renderer, pPowerUpBox->texture, NULL, &pPowerUpBox->rect);
}

void updatePowerCube(PowerUpBox *pPowerUpBox, SDL_Renderer *renderer, SDL_Rect playerRect) {
    if(pPowerUpBox->visible/*&& checkCollision(playerRect, power->rect)*/) {
        pPowerUpBox->visible = false;
        if(pPowerUpBox->restartTimerID) SDL_RemoveTimer(pPowerUpBox->restartTimerID);
        pPowerUpBox->restartTimerID = SDL_AddTimer(2000, respawnPowerCubeCallback, pPowerUpBox); // Respawn after 10 seconds (set to 10000)
    }
    renderPowerCube(pPowerUpBox, renderer);
}

SDL_Texture *getPowerTexture(PowerUpBox *pPowerUpBox) {
    return pPowerUpBox->texture;
}

SDL_Rect getPowerRect(PowerUpBox *pPowerUpBox) {
    return pPowerUpBox->rect;
}

void destroyPowerCube(PowerUpBox *pPowerUpBox) {
    if (pPowerUpBox->texture) SDL_DestroyTexture(pPowerUpBox->texture);
    free(pPowerUpBox);
}