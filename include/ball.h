#ifndef BALL_H
#define BALL_H

#include <stdbool.h>
#include <SDL2/SDL.h>

typedef struct ball Ball;

Ball *createBall(SDL_Renderer *renderer);
void updateBallPosition(Ball *ball);
void destroyBall(Ball *ball);

SDL_Rect getBallRect(Ball *ball);
SDL_Texture *getBallTexture(Ball *ball);

#endif /* BALL_H */
