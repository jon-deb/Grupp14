#ifndef power_h
#define power_h

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct power Power;

#define NR_OF_POWERUPS 2

Power *createPower(SDL_Renderer *renderer);
void spawnPowerCube(Power *power);
Uint32 respawnPowerCubeCallback(Uint32 interval, void *param);
int handlePowerCubeCollision(Power *power, SDL_Rect playerRect);
void renderPowerCube(Power *power, SDL_Renderer *renderer);
void updatePowerCube(Power *power, SDL_Renderer *renderer, SDL_Rect playerRect);
SDL_Texture *getPowerTexture(Power *power);
SDL_Rect getPowerRect(Power *power);
void destroyPowerCube(Power *power);

#endif