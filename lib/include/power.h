#ifndef power_h
#define power_h

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct power Power;

Power *createPower(SDL_Renderer *renderer);
void spawnPowerCube(Power *power);
Uint32 respawnPowerCubeCallback(Uint32 interval, void *param);
void handlePowerCubeCollision(Power *power, SDL_Rect playerRect);
void renderPowerCube(Power *power, SDL_Renderer *renderer);
void updatePowerCube(Power *power, SDL_Renderer *renderer, SDL_Rect playerRect);
SDL_Texture *getPowerTexture(Power *power);
SDL_Rect getPowerRect(Power *power);
int checkPowerCollision(SDL_Rect rect1, SDL_Rect rect2);
void destroyPowerCube(Power *power);

#endif