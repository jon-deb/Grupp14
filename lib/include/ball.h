#ifndef ball_h
#define ball_h

#include <stdbool.h>

typedef struct ball Ball;
typedef struct ballData BallData;

Ball *createBall(SDL_Renderer *renderer);
void updateBallPosition(Ball *pBall);
void destroyBall(Ball *pBall);
SDL_Texture *getBallTexture(Ball *pBall);
SDL_Rect getBallRect(Ball *pBall);
void setBallVelocity(Ball *pBall, float velocityX, float velocityY);
void setBallX(Ball *pBall, int x);
void setBallY(Ball *pBall, int y);
void applyFriction(Ball *pBall);
void restrictBallWithinWindow(Ball *pBall);
void handlePlayerBallCollision(SDL_Rect pRect, SDL_Rect bRect, Ball *pBall);
int checkCollision(SDL_Rect rect1, SDL_Rect rect2);
bool goal(Ball *pBall);
void getBallSendData(Ball *pBall, BallData *pBallData);
void updateBallWithRecievedData(Ball *pBall, BallData *pBallData);
bool goalScored(Ball *pBall);
#endif /* ball_h */