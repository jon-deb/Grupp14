#include <SDL2/SDL.h>
#include <SDL_image.h>
#include "../include/player.h"
struct player {
    float playerVelocityX;
    float playerVelocityY;
    SDL_Texture *playerTexture;
    SDL_Rect playerRect;
    SDL_Surface *playerSurface;
};

Player *createPlayer(SDL_Renderer *pGameRenderer, int w, int h) {
    Player *pPlayer = malloc(sizeof(struct player));
    pPlayer->playerRect.w = 64;
    pPlayer->playerRect.h = 64;
    pPlayer->playerRect.x = (w - pPlayer->playerRect.w) / 2;
    pPlayer->playerRect.y = (h - pPlayer->playerRect.h) / 2;
    pPlayer->playerVelocityX = 0;
    pPlayer->playerVelocityY = 0;
    //SDL_Surface *pPlayer->playerSurface = IMG_Load("resources/player.png");
    pPlayer->playerSurface = IMG_Load("resources/player.png");
    if (!pPlayer->playerSurface) {
        printf("Failed to load player image: %s\n", IMG_GetError());
        //closeGame(pGame);
        return NULL;    
    }

    pPlayer->playerTexture = SDL_CreateTextureFromSurface(pGameRenderer, pPlayer->playerSurface);
    SDL_FreeSurface(pPlayer->playerSurface);
    if (!pPlayer->playerTexture) {
        printf("Error: %s\n", SDL_GetError());
        //closeGame(pGame);
        return NULL;
    }
    return pPlayer;
}

void setPlayerX(Player *player) {
    player->playerRect.x += player->playerVelocityX / 60;
}

void setPlayerY(Player *player) {
    player->playerRect.y += player->playerVelocityY / 60;
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

int getPlayerSpeedY(Player *player) {
    if(player->playerVelocityY > 0) return 1;
    else return 0;
}

int getPlayerSpeedX(Player *player) {
    if(player->playerVelocityX > 0) return 1;
    else return 0;
}

void resetPlayerSpeed(Player *player, int x, int y) {
    if(x==1)player->playerVelocityX = 0;
    else if(y==1)player->playerVelocityY = 0;
}