#ifndef ball_h
#define ball_h

typedef struct ball Ball;

Ball *createBall(SDL_Renderer *pRenderer);
void destroyBall(Ball *pBall);

#endif