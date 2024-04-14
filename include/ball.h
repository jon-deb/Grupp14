#ifndef BALL_H
#define BALL_H

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct ball Ball;

Ball *createBall(SDL_Renderer *renderer);
void updateBallPosition(Ball *ball);
void destroyBall(Ball *ball);
SDL_Texture *getBallTexture(Ball *ball);
SDL_Rect getBallRect(Ball *ball);
void setBallVelocity(Ball *ball, float velocityX, float velocityY);
void setBallVelocity(Ball *ball, float velocityX, float velocityY);

#endif /* BALL_H */