#ifndef power_h
#define power_h

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct powerUpBox PowerUpBox;

PowerUpBox *createPower(SDL_Renderer *renderer);
void spawnPowerCube(PowerUpBox *pPowerUpBox);
Uint32 respawnPowerCubeCallback(Uint32 interval, void *param);
int handlePowerCubeCollision(PowerUpBox *pPowerUpBox, SDL_Rect playerRect);
void renderPowerCube(PowerUpBox *pPowerUpBox, SDL_Renderer *renderer);
void updatePowerCube(PowerUpBox *pPowerUpBox, SDL_Renderer *renderer, SDL_Rect playerRect);
SDL_Texture *getPowerTexture(PowerUpBox *pPowerUpBox);
SDL_Rect getPowerRect(PowerUpBox *pPowerUpBox);
void destroyPowerCube(PowerUpBox *pPowerUpBox);

#endif