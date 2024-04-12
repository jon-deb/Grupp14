#include <SDL2/SDL.h>
#include <SDL_image.h>
#include "../include/ball.h"
#include <string.h>

struct ball{
    float x, y, vx, vy; //koordinater samt hastighet i y- och x-led
    int hasCollided;
    char currentBallImage[4][20];
};

Ball *createBall(SDL_Renderer *pRenderer) {
    Ball *pBall = malloc(sizeof(struct ball));
    pBall->x = pBall->y = pBall->vy = pBall->vx = 0; //initierar alla värden till 0
    strcpy(pBall->currentBallImage[0], "resources/ball0.png");
    SDL_Surface *pSurface = IMG_Load(pBall->currentBallImage[0]);
    if(!pSurface) {
        printf("Error: %s\n", SDL_GetError());
        return NULL; //använd NULL istället för 0 i funktioner som returnerar pekare
    }
    return pBall;
}
void destroyBall(Ball *pBall) {
    free(pBall);
}