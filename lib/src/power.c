#include <SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "player_data.h"
#include "power.h"
#include <SDL_image.h>

#define POWER_WINDOW_X1 64 //distance from left of window to left of field
#define POWER_WINDOW_X2 1236 //distance from left of window to right of field
#define POWER_WINDOW_Y1 114 //distance from top of window to top of field
#define POWER_WINDOW_Y2 765 //distance from top of window to bottom of field


typedef struct power {
    SDL_Texture *texture;
    SDL_Rect rect;
    SDL_Surface *surface;
    bool visible;
} Power;

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
    pPower->rect.w = 32;
    pPower->rect.h = 32;
    pPower->visible = false;

    return pPower;
}

void spawnPowerCube(Power *power) {
    if (!power) return;

    power->rect.x += POWER_WINDOW_X1 + rand() % (POWER_WINDOW_X2 - power->rect.w);
    power->rect.y += POWER_WINDOW_Y1 + rand() % (POWER_WINDOW_Y2 - power->rect.h);
    power->visible = true;
}

void handlePowerCubeCollision(Power *power, SDL_Rect playerRect, SDL_Rect powerRect) {
    if (power->visible && checkPowerCollision(playerRect, powerRect)) {
        power->visible = false; // Hide the power cube on collision
        SDL_AddTimer(10000, respawnPowerCubeCallback, power); // Respawn after 10 seconds
    }
}

Uint32 respawnPowerCubeCallback(Uint32 interval, void *param) {
    Power *power = (Power *)param;
    if (power) {
        spawnPowerCube(power);
    }
    return 0; // Stop the timer
}

void renderPowerCube(Power *power, SDL_Renderer *renderer) {
    if (power && power->visible) {
        SDL_RenderCopy(renderer, power->texture, NULL, &power->rect);
    }
}

void updatePowerCube(Power *power, SDL_Renderer *renderer, SDL_Rect playerRect) {
    handlePowerCubeCollision(power, playerRect, power->rect);
    renderPowerCube(power, renderer);
}

SDL_Texture *getPowerTexture(Power *pPower) {
    return pPower->texture;
}

SDL_Rect getPowerRect(Power *pPower) {
    return pPower->rect;
}

int checkPowerCollision(SDL_Rect rect1, SDL_Rect rect2) {
    if (rect1.y + rect1.h <= rect2.y) return 0; // Bottom is above top
    if (rect1.y >= rect2.y + rect2.h) return 0; // Top is below bottom
    if (rect1.x + rect1.w <= rect2.x) return 0; // Right is left of left
    if (rect1.x >= rect2.x + rect2.w) return 0; // Left is right of right
    return 1;
}

void destroyPowerCube(Power *power) {
    if (power->texture) SDL_DestroyTexture(power->texture);
    free(power);
}