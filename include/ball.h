#ifndef BALL_H
#define BALL_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#define FRICTION_COEFFICIENT 0.95f

typedef struct ball {
    SDL_Texture *texture;
    SDL_Rect rect;
    SDL_Surface *surface;
    float velocityX;
    float velocityY;
    bool collided;
} Ball;

Ball *createBall(SDL_Renderer *renderer);
void updateBallPosition(Ball *ball);
void destroyBall(Ball *ball);
SDL_Texture *getBallTexture(Ball *ball);
SDL_Rect getBallRect(Ball *ball);
void setBallVelocity(Ball *ball, float velocityX, float velocityY);
void setBallX(Ball *ball, int x);
void setBallY(Ball *ball, int y);
void applyFriction(Ball *pBall);


#endif /* BALL_H */