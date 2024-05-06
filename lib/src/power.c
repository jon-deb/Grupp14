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


struct power {
    SDL_Texture *texture;
    SDL_Rect rect;
    SDL_Surface *surface;
    bool visible;
    SDL_TimerID restartTimerID;
};

Power *createPower(SDL_Renderer *renderer) {
    Power *pPower = malloc(sizeof(Power));
    if (!pPower) {
        fprintf(stderr, "Failed to allocate memory for pPower.\n");
        return NULL;
    }

    pPower->surface = IMG_Load("../lib/resources/power.png");
    if (!pPower->surface) {
        fprintf(stderr, "Error loading pPower texture: %s\n", SDL_GetError());
        free(pPower);
        return NULL;
    }

    pPower->texture = SDL_CreateTextureFromSurface(renderer, pPower->surface);
    SDL_FreeSurface(pPower->surface);
    if (!pPower->texture) {
        fprintf(stderr, "Error creating pPower texture: %s\n", SDL_GetError());
        free(pPower);
        return NULL;
    }

    srand(time(NULL));
    pPower->rect.w = 48;
    pPower->rect.h = 48;
    pPower->visible = false;

    return pPower;
}

void spawnPowerCube(Power *power) {
    if (!power) return;

    power->rect.x = POWER_WINDOW_X1 + rand() % (POWER_WINDOW_X2 - POWER_WINDOW_X1 - power->rect.w);
    power->rect.y = POWER_WINDOW_Y1 + rand() % (POWER_WINDOW_Y2 - POWER_WINDOW_Y1 - power->rect.h);
    power->visible = true;

    if (power->restartTimerID != 0) {
        SDL_RemoveTimer(power->restartTimerID);
        power->restartTimerID = 0;
    }
}

Uint32 respawnPowerCubeCallback(Uint32 interval, void *param) {
    Power *power = (Power *)param;
    if (power) spawnPowerCube(power);
    return 0;
}

void renderPowerCube(Power *power, SDL_Renderer *renderer) {
    if (power->visible) SDL_RenderCopy(renderer, power->texture, NULL, &power->rect);
}

int handlePowerCubeCollision(Power *power, SDL_Rect playerRect) {
    int collided = 0;
    if (power->visible && checkCollision(playerRect, power->rect)) {

        power->visible = false;

        if(power->restartTimerID) SDL_RemoveTimer(power->restartTimerID);
        power->restartTimerID = SDL_AddTimer(10000, respawnPowerCubeCallback, power); // Respawn after 10 seconds
    }

}

void updatePowerCube(Power *power, SDL_Renderer *renderer, SDL_Rect playerRect) {
    handlePowerCubeCollision(power, playerRect);
    renderPowerCube(power, renderer);
}

SDL_Texture *getPowerTexture(Power *pPower) {
    return pPower->texture;
}

SDL_Rect getPowerRect(Power *pPower) {
    return pPower->rect;
}

void destroyPowerCube(Power *power) {
    if (power->texture) SDL_DestroyTexture(power->texture);
    free(power);
}