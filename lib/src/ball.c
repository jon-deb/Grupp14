#include <SDL.h>
#include "player_data.h"
#include "ball.h"
#include <SDL_image.h>
#include <stdbool.h>

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800
#define BALL_WINDOW_X1 64 //distance from left of window to left of field
#define BALL_WINDOW_X2 1236 //distance from left of window to right of field
#define BALL_WINDOW_Y1 114 //distance from top of window to top of field
#define BALL_WINDOW_Y2 765 //distance from top of window to bottom of field
#define MOVEMENT_SPEED 200
#define MIDDLE_OF_FIELD_Y 440 //distance from top of window to mid point of field
#define FRICTION_COEFFICIENT 0.94f
#define BALL_SPEED_AFTER_COLLISION 500
#define GOAL_TOP 357 //distance from top of window to northern goal post
#define GOAL_BOTTOM 522 //distance from top of window to southern goal post

struct ball {
    SDL_Texture *texture;
    SDL_Rect rect;
    SDL_Surface *surface;
    float velocityX, velocityY;
};

Ball *createBall(SDL_Renderer *renderer) {
    Ball *pBall = malloc(sizeof(Ball));
    if (!pBall) {
        fprintf(stderr, "Failed to allocate memory for pBall.\n");
        return NULL;
    }

    pBall->surface = IMG_Load("../lib/resources/ball2.png");
    if (!pBall->surface) {
        fprintf(stderr, "Error loading pBall texture: %s\n", SDL_GetError());
        free(pBall);
        return NULL;
    }

    pBall->texture = SDL_CreateTextureFromSurface(renderer, pBall->surface);
    SDL_FreeSurface(pBall->surface);
    if (!pBall->texture) {
        fprintf(stderr, "Error creating pBall texture: %s\n", SDL_GetError());
        free(pBall);
        return NULL;
    }

    pBall->rect.w = 32;
    pBall->rect.h = 32;
    pBall->rect.x = WINDOW_WIDTH / 2 - pBall->rect.w / 2;
    pBall->rect.y = MIDDLE_OF_FIELD_Y - pBall->rect.h / 2; 
    pBall->velocityX = 0;
    pBall->velocityY = 0;

    return pBall;
}

void updateBallPosition(Ball *pBall) {
    pBall->rect.x += pBall->velocityX / 60;
    pBall->rect.y += pBall->velocityY / 60;
}

SDL_Texture *getBallTexture(Ball *pBall) {
    return pBall->texture;
}

SDL_Rect getBallRect(Ball *pBall) {
    return pBall->rect;
}

void setBallVelocity(Ball *pBall, float velocityX, float velocityY) {
    pBall->velocityX = velocityX;
    pBall->velocityY = velocityY;
}

void setBallX(Ball *pBall, int x) {
    pBall->rect.x = x;
}

void setBallY(Ball *pBall, int y) {
    pBall->rect.y = y;
}

void applyFriction(Ball *pBall) {
    float vx = pBall->velocityX * FRICTION_COEFFICIENT;
    float vy = pBall->velocityY * FRICTION_COEFFICIENT;
    if(vx <=145) pBall->velocityX = 0;
    if(vy <=145) pBall->velocityY = 0;
    
    setBallVelocity(pBall, vx, vy);
}

void restrictBallWithinWindow(Ball *pBall) {
    SDL_Rect ballRect = getBallRect(pBall);
    if (ballRect.x < BALL_WINDOW_X1) {
        if(ballRect.y >= GOAL_TOP && ballRect.y <= GOAL_BOTTOM) 
            setBallX(pBall, 0);
        else setBallX(pBall, BALL_WINDOW_X1);
        
        pBall->velocityX = -pBall->velocityX;
    } 
    if (ballRect.x + ballRect.w > BALL_WINDOW_X2) {
        if(ballRect.y >= GOAL_TOP && ballRect.y <= GOAL_BOTTOM) 
            setBallX(pBall, WINDOW_WIDTH);
        else setBallX(pBall, BALL_WINDOW_X2-ballRect.w);

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

void handlePlayerBallCollision(SDL_Rect pRect, SDL_Rect bRect, Ball *pBall) {
    static float collisionTimer = 0.0f; 
    if(checkCollision(pRect, bRect)) {
        float playerCenterX = pRect.x + pRect.w / 2;
        float playerCenterY = pRect.y + pRect.h / 2;
        float ballCenterX = bRect.x + bRect.w / 2;
        float ballCenterY = bRect.y + bRect.h / 2;

        float collisionVectorX = ballCenterX - playerCenterX;
        float collisionVectorY = ballCenterY - playerCenterY;

        float distance = sqrt(collisionVectorX * collisionVectorX + collisionVectorY * collisionVectorY);

        float normalX = collisionVectorX / distance;
        float normalY = collisionVectorY / distance;

        
        setBallVelocity(pBall, normalX * BALL_SPEED_AFTER_COLLISION, normalY * BALL_SPEED_AFTER_COLLISION);
        collisionTimer = 1.2f; // 0.6 sekunder
    }
    
    updateBallPosition(pBall);
    
    collisionTimer -= 1.0f / 60.0f;
    if (collisionTimer <= 0.0f) {
        setBallVelocity(pBall, 0, 0);
        collisionTimer = 0.0f;
    }
}

int checkCollision(SDL_Rect rect1, SDL_Rect rect2) {
    return SDL_HasIntersection(&rect1, &rect2);
}

void getBallSendData(Ball *pBall, BallData *pBallData){
    pBallData->velocityX = pBall->velocityX;
    pBallData->velocityY = pBall->velocityY;
    pBallData->x = pBall->rect.x;
    pBallData->y = pBall->rect.y;
}

void updateBallWithRecievedData(Ball *pBall, BallData *pBallData){
    pBall->velocityX = pBallData->velocityX;
    pBall->velocityY = pBallData->velocityY; 
    pBall->rect.x = pBallData->x;
    pBall->rect.y = pBallData->y;
}

void destroyBall(Ball *pBall) {
    if (pBall->texture) SDL_DestroyTexture(pBall->texture);
    free(pBall);
}

bool goal(Ball *pBall) {
    SDL_Rect ballRect = getBallRect(pBall);
    if ((ballRect.x + ballRect.w < BALL_WINDOW_X1 || ballRect.x + ballRect.w > BALL_WINDOW_X2) && ballRect.y >= GOAL_TOP && ballRect.y <= GOAL_BOTTOM) {
        return true;
    }
    return false;
}

bool goalScored(Ball *pBall) {
    SDL_Rect ballRect = getBallRect(pBall);
    
    if (ballRect.x < WINDOW_WIDTH/2) { //ball is in left half of field
        setBallX(pBall, WINDOW_WIDTH / 2 - ballRect.w / 2);
        setBallY(pBall, MIDDLE_OF_FIELD_Y - ballRect.h / 2);
        setBallVelocity(pBall, 0, 0);
        pBall->velocityX = 0;
        pBall->velocityY = 0;
        return 0;
    }
    else { //ball is in right half of field
        setBallX(pBall, WINDOW_WIDTH / 2 - ballRect.w / 2);
        setBallY(pBall, MIDDLE_OF_FIELD_Y - ballRect.h / 2);
        setBallVelocity(pBall, 0, 0);
        pBall->velocityX = 0;
        pBall->velocityY = 0;
        return 1;
    }
}