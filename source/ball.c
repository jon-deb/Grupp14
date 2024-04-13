#include <SDL2/SDL.h>
#include <SDL_image.h>
#include "../include/ball.h"

struct ball{
    float x, y, vx, vy; //koordinater samt hastighet i y- och x-led
    int hasCollided;
    SDL_Texture *ballTextures[BALL_FRAMES];
};

Ball *createBall(SDL_Renderer *pRenderer) {
    Ball *pBall = malloc(sizeof(struct ball));
    pBall->x = pBall->y = pBall->vy = pBall->vx = 0; //initierar alla värden till 0
    strcpy(pBall->ballTextures[0], "resources/ball0.png");
    SDL_Surface *pSurface = IMG_Load(pBall->ballTextures[0]);
    if(!pSurface) {
        printf("Error: %s\n", SDL_GetError());
        return NULL; //använd NULL istället för 0 i funktioner som returnerar pekare
    }
    return pBall;
}
void destroyBall(Ball *pBall) {
    free(pBall);
}
void updateBall(Ball *pBall) {
    for (int i = 0; i < BALL_FRAMES; ++i) {
        char filename[50];
        sprintf(filename, "resources/ball%d.png", i + 1);
        SDL_Surface *ballSurface = IMG_Load(filename);
        if (!ballSurface) {
            printf("Error: %s\n", SDL_GetError());
            for (int j = 0; j < i; ++j) {
                SDL_DestroyTexture(pBall->ballTextures[j]);
            }
            SDL_DestroyRenderer(pRenderer);
            SDL_DestroyWindow(pWindow);
            SDL_Quit();
            return 1;
        }
        pBall->ballTextures[i] = SDL_CreateTextureFromSurface(pRenderer, ballSurface);
        SDL_FreeSurface(ballSurface);
        if (!ballTextures[i]) {
            printf("Error: %s\n", SDL_GetError());
            for (int j = 0; j < i; ++j) {
                SDL_DestroyTexture(pBall->ballTextures[j]);
            }
            SDL_DestroyRenderer(pRenderer);
            SDL_DestroyWindow(pWindow);
            SDL_Quit();
            return 1;
        }
    }
}