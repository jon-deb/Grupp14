#include <stdio.h>
#include <SDL2/SDL.h>
//#include "ball.h"
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define MOVEMENT_SPEED 400


typedef struct game {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Ball *pBall;
}Game;

int initiate(Game *pGame);
void exit(Game *pGame);

int main() {
    Game g = {0};
    if(!initiate(&g)) return 1;
    run(&g);
    exit(&g);
    
    return 0;
}

int initiate(Game *pGame) {
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0){
        printf("Error: %s\n",SDL_GetError());
        return 0;
    }
    pGame->pWindow = SDL_CreateWindow("Rocket Game",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WINDOW_WIDTH,WINDOW_HEIGHT,0);
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

    pGame->pBall = createBall(pGame->pRenderer);
    //pGame->pPlayer = createAsteroidImage(pGame->pRenderer);

    if(!pGame->pBall /*|| !pGame->pPlayer*/){
        printf("Error: %s\n",SDL_GetError());
        close(pGame);
        return 0;
    }

    /*for(int i=0;i<NROFPLAYERS;i++){
        pGame->pAsteroids[i] = createAsteroid(pGame->pAsteroidImage,WINDOW_WIDTH,WINDOW_HEIGHT);
    }*/

    return 1;
}

void exit(Game *pGame) {
    if(pGame->pBall) destroyBall(pGame->pBall);
    /*for(int i=0;i<MAX_ASTEROIDS;i++){
        if(pGame->pAsteroids[i]) destroyAsteroid(pGame->pAsteroids[i]);
    }*/
    //if(pGame->pAsteroidImage) destroyAsteroidImage(pGame->pAsteroidImage);
    if(pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if(pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    SDL_Quit();
}