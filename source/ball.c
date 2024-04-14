#include "../include/ball.h"
#include <SDL2/SDL_image.h>

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 800
#define MOVEMENT_SPEED 400

typedef struct ball {
    SDL_Texture *balltexture;
    SDL_Rect ballRect;
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

    SDL_Surface *ballSurface = IMG_Load("resources/ball1.png");
    if (!ballSurface) {
        fprintf(stderr, "Error loading ball texture: %s\n", SDL_GetError());
        free(ball);
        return NULL;
    }

    ball->balltexture = SDL_CreateTextureFromSurface(renderer, ballSurface);
    SDL_FreeSurface(ballSurface);
    if (!ball->balltexture) {
        fprintf(stderr, "Error creating ball texture: %s\n", SDL_GetError());
        free(ball);
        return NULL;
    }

    ball->ballRect.w = 32;
    ball->ballRect.h = 32;
    ball->ballRect.x = WINDOW_HEIGHT / 4;
    ball->ballRect.y = WINDOW_HEIGHT / 4; 
    ball->velocityX = 0;
    ball->velocityY = 0;
    ball->collided = false;

    return ball;
}

void updateBallPosition(Ball *ball) {
    ball->ballRect.x += ball->velocityX / 60;
    ball->ballRect.y += ball->velocityY / 60;
}

void destroyBall(Ball *ball) {
    if (ball->balltexture) SDL_DestroyTexture(ball->balltexture);
    free(ball);
}

SDL_Texture *getBallTexture(Ball *ball) {
    return ball->balltexture;
}

SDL_Rect getBallRect(Ball *ball) {
    return ball->ballRect;
}


void setBallVelocity(Ball *ball, float velocityX, float velocityY) {
    ball->velocityX = velocityX;
    ball->velocityY = velocityY;
}
