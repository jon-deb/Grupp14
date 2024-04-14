#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>

// Define the Player struct in the header so it is visible to other files.
typedef struct player {
    float playerVelocityX;
    float playerVelocityY;
    SDL_Texture *playerTexture;
    SDL_Rect playerRect;
} Player;

Player *createPlayer(SDL_Renderer *pGameRenderer, int w, int h);
void setPlayerPosition(Player *player, int x, int y);
void updatePlayerVelocity(Player *player, float vx, float vy);
SDL_Texture *getPlayerTexture(Player *player);
SDL_Rect getPlayerRect(Player *player);
void updatePlayerVUp(Player *player);
void updatePlayerVDown(Player *player);
void updatePlayerVLeft(Player *player);
void updatePlayerVRight(Player *player);
void resetPlayerSpeed(Player *player, int x, int y);
int getPlayerSpeedY(Player *player);
int getPlayerSpeedX(Player *player);

#endif
