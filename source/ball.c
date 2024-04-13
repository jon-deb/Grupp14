#include "../include/ball.h"
#include <SDL2/SDL_image.h>


#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 800
#define MOVEMENT_SPEED 400


Ball *createBall(SDL_Renderer *renderer) {
    Ball *ball = malloc(sizeof(Ball));
    if (!ball) {
        fprintf(stderr, "Failed to allocate memory for ball.\n");
        return NULL;
    }

    SDL_Surface *ballSurface = IMG_Load("resources/ball1.png");
    if (!ballSurface) {
        fprintf(stderr, "Error loading ball texture: %s\n", SDL_GetError());
        free(ball);
        return NULL;
    }

    ball->texture = SDL_CreateTextureFromSurface(renderer, ballSurface);
    SDL_FreeSurface(ballSurface);
    if (!ball->texture) {
        fprintf(stderr, "Error creating ball texture: %s\n", SDL_GetError());
        free(ball);
        return NULL;
    }

    ball->rect.w = 32;
    ball->rect.h = 32;
    ball->rect.x = WINDOW_HEIGHT / 4;
    ball->rect.y = WINDOW_HEIGHT / 4; 
    ball->velocityX = 0;
    ball->velocityY = 0;
    ball->collided = false;

    return ball;
}

void updateBallPosition(Ball *ball) {
    ball->rect.x += ball->velocityX / 60;
    ball->rect.y += ball->velocityY / 60;
}

void destroyBall(Ball *ball) {
    if (ball->texture) SDL_DestroyTexture(ball->texture);
    free(ball);
}
