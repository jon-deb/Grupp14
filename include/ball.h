#ifndef BALL_H
#define BALL_H

#include <stdbool.h>
#include <SDL2/SDL.h>

typedef struct ball {
    SDL_Texture *texture;
    SDL_Rect rect;
    float velocityX;
    float velocityY;
    bool collided;
} Ball;

Ball *createBall(SDL_Renderer *renderer);
void updateBallPosition(Ball *ball);
void destroyBall(Ball *ball);

#endif /* BALL_H */
