#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "../include/ball.h"
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define MOVEMENT_SPEED 400


typedef struct game {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Ball *pBall;
    SDL_Texture *backgroundTexture;
}Game;

int initiate(Game *pGame);
void close(Game *pGame);
void run(Game *pGame);

int main(int argv, char** args) {
    Game g = {0};
    if(!initiate(&g)) return 1;
    run(&g);
    close(&g);
    
    return 0;
}

int initiate(Game *pGame) {
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0){
        printf("Error: %s\n",SDL_GetError());
        return 0;
    }
    pGame->pWindow = SDL_CreateWindow("Football Game",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WINDOW_WIDTH,WINDOW_HEIGHT,0);
    if(!pGame->pWindow){
        printf("Error: %s\n",SDL_GetError());
        close(pGame);
        return 0;
    }
    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if(!pGame->pRenderer){
        printf("Error: %s\n",SDL_GetError());
        close(pGame);
        return 0;    
    }
    SDL_Surface *backgroundSurface = IMG_Load("resources/topdownfield.png");
    if(!backgroundSurface){
        printf("Error: %s\n",SDL_GetError());
        close(pGame);
        return 0;    
    }

    pGame->backgroundTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, backgroundSurface);
    SDL_FreeSurface(backgroundSurface);
    if(!pGame->backgroundTexture){
        printf("Error: %s\n",SDL_GetError());
        close(pGame);
        return 0;    
    }

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
            if (event.type == SDL_QUIT) close_requested = 0;
            //else handleInput(pGame,&event);
        }
        //updateBall(pGame->pBall);
        SDL_RenderClear(pGame->pRenderer);
        SDL_RenderCopy(pGame->pRenderer, pGame->backgroundTexture, NULL, NULL);
        SDL_RenderPresent(pGame->pRenderer);
        SDL_Delay(1000/60-15);
    }
}

void close(Game *pGame) {
    if(pGame->pBall) destroyBall(pGame->pBall);
    /*for(int i=0;i<MAX_ASTEROIDS;i++){
        if(pGame->pAsteroids[i]) destroyAsteroid(pGame->pAsteroids[i]);
    }*/
    //if(pGame->pAsteroidImage) destroyAsteroidImage(pGame->pAsteroidImage);
    if(pGame->pBall) destroyBall(pGame->pBall);
    if(pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if(pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    SDL_Quit();
}