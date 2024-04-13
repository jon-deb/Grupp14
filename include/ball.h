#ifndef ball_h
#define ball_h

#define BALL_FRAMES 5

typedef struct ball Ball;

Ball *createBall(SDL_Renderer *pRenderer);
void updateBall(Ball *pBall);
void destroyBall(Ball *pBall);

#endif