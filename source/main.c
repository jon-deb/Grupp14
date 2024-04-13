#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "../include/ball.h"

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 800
#define MOVEMENT_SPEED 400

typedef struct game {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Ball *pBall;
    SDL_Texture *backgroundTexture;
    SDL_Texture *playerTexture;
    SDL_Rect playerRect;
    float playerVelocityX;
    float playerVelocityY;
} Game;

int initiate(Game *pGame);
void closeGame(Game *pGame);
void run(Game *pGame);
void handleInput(Game *pGame, SDL_Event *event);

int main(int argc, char** argv) {
    Game g = {0};
    if (!initiate(&g)) return 1;
    run(&g);
    closeGame(&g);
    
    return 0;
}

int initiate(Game *pGame) {
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return 0;
    }
    pGame->pWindow = SDL_CreateWindow("Football Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!pGame->pWindow) {
        printf("Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return 0;
    }
    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if (!pGame->pRenderer) {
        printf("Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return 0;    
    }
    SDL_Surface *backgroundSurface = IMG_Load("resources/field.png");
    if (!backgroundSurface) {
        printf("Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return 0;    
    }
    pGame->backgroundTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, backgroundSurface);
    SDL_FreeSurface(backgroundSurface);
    if (!pGame->backgroundTexture) {
        printf("Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return 0;    
    }

    SDL_Surface *playerSurface = IMG_Load("resources/player.png");
    if (!playerSurface) {
        printf("Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return 0;    
    }

    pGame->playerTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, playerSurface);
    SDL_FreeSurface(playerSurface);
    if (!pGame->playerTexture) {
        printf("Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return 0;    
    }


    pGame->playerRect.w = 64;
    pGame->playerRect.h = 64;
    pGame->playerRect.x = (WINDOW_WIDTH - pGame->playerRect.w) / 2;
    pGame->playerRect.y = (WINDOW_HEIGHT - pGame->playerRect.h) / 2;
    pGame->playerVelocityX = 0;
    pGame->playerVelocityY = 0;

    //pGame->pBall = createBall(pGame->pRenderer);
    //pGame->pPlayer = createAsteroidImage(pGame->pRenderer);

    /*if(!pGame->pBall /*|| !pGame->pPlayer){
        printf("Error: %s\n",SDL_GetError());
        close(pGame);
        return 0;
    }*/

    /*for(int i=0;i<NROFPLAYERS;i++){
        pGame->pAsteroids[i] = createAsteroid(pGame->pAsteroidImage,WINDOW_WIDTH,WINDOW_HEIGHT);
    }*/

    return 1;
}

void run(Game *pGame) {
    int close_requested = 0;
    SDL_Event event;

    while (!close_requested) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) close_requested = 1;
            else handleInput(pGame, &event);
        }
        //updateBall(pGame->pBall);

        pGame->playerRect.x += pGame->playerVelocityX / 60;
        pGame->playerRect.y += pGame->playerVelocityY / 60;

        SDL_RenderClear(pGame->pRenderer);
        SDL_RenderCopy(pGame->pRenderer, pGame->backgroundTexture, NULL, NULL);
        SDL_RenderCopy(pGame->pRenderer, pGame->playerTexture, NULL, &pGame->playerRect);
        SDL_RenderPresent(pGame->pRenderer);
        SDL_Delay(1000/60 - 15);
    }
}

void handleInput(Game *pGame, SDL_Event *event) {
    switch (event->type) {
        case SDL_KEYDOWN:
            switch (event->key.keysym.scancode) {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    pGame->playerVelocityY = -MOVEMENT_SPEED;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    pGame->playerVelocityX = -MOVEMENT_SPEED;
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    pGame->playerVelocityY = MOVEMENT_SPEED;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    pGame->playerVelocityX = MOVEMENT_SPEED;
                    break;
            }
            break;
        case SDL_KEYUP:
            switch (event->key.keysym.scancode) {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    if (pGame->playerVelocityY < 0) pGame->playerVelocityY = 0;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    if (pGame->playerVelocityX < 0) pGame->playerVelocityX = 0;
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    if (pGame->playerVelocityY > 0) pGame->playerVelocityY = 0;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    if (pGame->playerVelocityX > 0) pGame->playerVelocityX = 0;
                    break;
            }
            break;
    }
}

void closeGame(Game *pGame) {
    if (pGame->pBall) destroyBall(pGame->pBall);
    /*for(int i=0;i<MAX_ASTEROIDS;i++){
        if(pGame->pAsteroids[i]) destroyAsteroid(pGame->pAsteroids[i]);
    }*/
    //if(pGame->pAsteroidImage) destroyAsteroidImage(pGame->pAsteroidImage);
    if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if (pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    SDL_Quit();
}
