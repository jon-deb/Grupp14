#ifndef ball_h
#define ball_h

#include <stdbool.h>

typedef struct ball Ball;

Ball *createBall(SDL_Renderer *renderer);
void updateBallPosition(Ball *ball);
void destroyBall(Ball *ball);
SDL_Texture *getBallTexture(Ball *ball);
SDL_Rect getBallRect(Ball *ball);
void setBallVelocity(Ball *ball, float velocityX, float velocityY);
void setBallX(Ball *ball, int x);
void setBallY(Ball *ball, int y);
void applyFriction(Ball *pBall);
void restrictBallWithinWindow(Ball *pBall);
bool goal(Ball *pBall);
void getBallSendData(Ball *pBall, BallData *pBallData);
void updateBallWithRecievedData(Ball *pBall, BallData *pBallData);

#endif /* ball_h */