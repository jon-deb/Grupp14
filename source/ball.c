#include "../include/ball.h"
#include <SDL2/SDL_image.h>

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 800
#define MOVEMENT_SPEED 400

typedef struct ball {
    SDL_Texture *texture;
    SDL_Rect rect;
    SDL_Surface *surface;
    float velocityX;
    float velocityY;
    bool collided;
} Ball;

Ball *createBall(SDL_Renderer *renderer) {
    Ball *ball = malloc(sizeof(Ball));
    if (!ball) {
        fprintf(stderr, "Failed to allocate memory for ball.\n");
        return NULL;
    }

    ball->surface = IMG_Load("resources/ball1.png");
    if (!ball->surface) {
        fprintf(stderr, "Error loading ball texture: %s\n", SDL_GetError());
        free(ball);
        return NULL;
    }

    ball->texture = SDL_CreateTextureFromSurface(renderer, ball->surface);
    SDL_FreeSurface(ball->surface);
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

SDL_Texture *getBallTexture(Ball *ball) {
    return ball->texture;
}

SDL_Rect getBallRect(Ball *ball) {
    return ball->rect;
}

void setBallVelocity(Ball *ball, float velocityX, float velocityY) {
    ball->velocityX = velocityX;
    ball->velocityY = velocityY;
}

void setBallX(Ball *ball, int x) {
    ball->rect.x = x;
}

void setBallY(Ball *ball, int y) {
    ball->rect.y = y;
}


