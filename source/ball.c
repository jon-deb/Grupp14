#include "../include/ball.h"
#include <SDL2/SDL_image.h>

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 800
#define BALL_WINDOW_X1 62
#define BALL_WINDOW_X2 1338
#define BALL_WINDOW_Y1 42
#define BALL_WINDOW_Y2 758
#define MOVEMENT_SPEED 400
#define FRICTION_COEFFICIENT 0.95f
#define GOAL_TOP 300
#define GOAL_BOTTOM 500

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


void applyFriction(Ball *pBall) {
    // skapar variabel och sätter till hastigheterna
    float vx = pBall->velocityX;
    float vy = pBall->velocityY;
    
    // sänker hastigheten 
    vx *= FRICTION_COEFFICIENT;
    vy *= FRICTION_COEFFICIENT;

    // ny hastighet
    setBallVelocity(pBall, vx, vy);
}


void restrictBallWithinWindow(Ball *pBall) {
    SDL_Rect ballRect = getBallRect(pBall);
    if (ballRect.x < BALL_WINDOW_X1) {
        if (ballRect.y >= GOAL_TOP && ballRect.y <= GOAL_BOTTOM)
        {
            setBallX(pBall, 0);
        }
        else
        {
            setBallX(pBall, BALL_WINDOW_X1);
        }
        pBall->velocityX = -pBall->velocityX;
    } if (ballRect.x + ballRect.w > BALL_WINDOW_X2) {
        if (ballRect.y >= GOAL_TOP && ballRect.y <= GOAL_BOTTOM)
        {
            setBallX(pBall, WINDOW_WIDTH);
        }
        else
        {
            setBallX(pBall, BALL_WINDOW_X2-ballRect.w);
        }
        pBall->velocityX = -pBall->velocityX;
    }
    if (ballRect.y < BALL_WINDOW_Y1) {
        setBallY(pBall, BALL_WINDOW_Y1);
        pBall->velocityY = -pBall->velocityY;
    } else if (ballRect.y + ballRect.h > BALL_WINDOW_Y2) {
        setBallY(pBall, BALL_WINDOW_Y2 - ballRect.h);
        pBall->velocityY = -pBall->velocityY;
    }
}



bool goal(Ball *pBall) {
    SDL_Rect ballRect = getBallRect(pBall);
    if ((ballRect.x < 0 || ballRect.x + ballRect.w > WINDOW_WIDTH) && ballRect.y >= GOAL_TOP && ballRect.y <= GOAL_BOTTOM) {
        setBallX(pBall, WINDOW_WIDTH / 2 - ballRect.w / 2);
        setBallY(pBall, WINDOW_HEIGHT / 2 - ballRect.h / 2);
        pBall->velocityX = 0;
        pBall->velocityY = 0;
        return true;
    }
    return false;
}

