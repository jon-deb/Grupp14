#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "../include/ball.h"
#include "../include/player.h"

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 800
#define MOVEMENT_SPEED 400
#define BALL_SPEED_AFTER_COLLISION 500
#define BORDER_SIZE 20 // Adjust this value as needed

typedef struct game {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Ball *pBall;
    SDL_Texture *backgroundTexture;
    Player *pPlayer;
} Game;

int initiate(Game *pGame);
void run(Game *pGame);
void closeGame(Game *pGame);
void handleInput(Game *pGame, SDL_Event *event);
bool checkCollision(SDL_Rect rect1, SDL_Rect rect2);
void restrictPlayerWithinWindow(Player *pPlayer);
void restrictBallWithinWindow(Ball *pBall);
void updatePlayerPosition(Player *pPlayer, float deltaTime);
void renderGame(Game *pGame);
void handleCollisionsAndPhysics(Game *pGame);

int main(int argc, char** argv) {
    Game g = {0};
    if (!initiate(&g)) return 1;
    run(&g);
    closeGame(&g);
    return 0;
}

int initiate(Game *pGame) {
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return 0;
    }
    pGame->pWindow = SDL_CreateWindow("Football Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!pGame->pWindow) {
        printf("Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return 0;
    }
    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if (!pGame->pRenderer) {
        printf("Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return 0;    
    }
    SDL_Surface *backgroundSurface = IMG_Load("resources/field.png");
    if (!backgroundSurface) {
        printf("Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return 0;    
    }
    pGame->backgroundTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, backgroundSurface);
    SDL_FreeSurface(backgroundSurface);
    if (!pGame->backgroundTexture) {
        printf("Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return 0;    
    }

    pGame->pPlayer = createPlayer(pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    pGame->pBall = createBall(pGame->pRenderer);
    if (!pGame->pBall) {
        printf("Error initializing the ball.\n");
        closeGame(pGame);
        return 0;
    }
    return 1;
}

void run(Game *pGame) {
    int close_requested = 0;
    SDL_Event event;
    Uint32 lastTick = SDL_GetTicks();
    Uint32 currentTick;
    float deltaTime;

    while (!close_requested) {
        currentTick = SDL_GetTicks();
        deltaTime = (currentTick - lastTick) / 1000.0f; // Calculate delta time in seconds
        lastTick = currentTick;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) close_requested = 1;
            else handleInput(pGame, &event);
        }
        updatePlayerPosition(pGame->pPlayer, deltaTime);
        restrictPlayerWithinWindow(pGame->pPlayer);
        renderGame(pGame);
    }
}

void updatePlayerPosition(Player *pPlayer, float deltaTime) {
    SDL_Rect playerRect = getPlayerRect(pPlayer);
    int newX = playerRect.x + pPlayer->playerVelocityX * deltaTime;
    int newY = playerRect.y + pPlayer->playerVelocityY * deltaTime;
    setPlayerPosition(pPlayer, newX, newY);
}


void renderGame(Game *pGame) {
    SDL_Rect playerRect = getPlayerRect(pGame->pPlayer);
    SDL_Texture *playerTexture = getPlayerTexture(pGame->pPlayer);
    SDL_Rect ballRect = getBallRect(pGame->pBall);
    SDL_Texture *ballTexture = getBallTexture(pGame->pBall);

    SDL_RenderClear(pGame->pRenderer);
    SDL_RenderCopy(pGame->pRenderer, pGame->backgroundTexture, NULL, NULL);
    SDL_RenderCopy(pGame->pRenderer, playerTexture, NULL, &playerRect);
    SDL_RenderCopy(pGame->pRenderer, ballTexture, NULL, &ballRect);
    SDL_RenderPresent(pGame->pRenderer);
    SDL_Delay(1000/60); // Approximately 60 FPS
    handleCollisionsAndPhysics(pGame);
}

void handleCollisionsAndPhysics(Game *pGame) {
    SDL_Rect playerRect = getPlayerRect(pGame->pPlayer);
    SDL_Rect ballRect = getBallRect(pGame->pBall);
     if(checkCollision(playerRect, ballRect)) {
            // räknar mittpunkten för spelare och bollen
            float playerCenterX = playerRect.x + playerRect.w / 2;
            float playerCenterY = playerRect.y + playerRect.h / 2;
            float ballCenterX = ballRect.x + ballRect.w / 2;
            float ballCenterY = ballRect.y + ballRect.h / 2;

            // beräknar vektorn
            float collisionVectorX = ballCenterX - playerCenterX;
            float collisionVectorY = ballCenterY - playerCenterY;
            
            // räknar distansen
            float distance = sqrt(collisionVectorX * collisionVectorX + collisionVectorY * collisionVectorY);

            // normaliserar vektorn
            float normalX = collisionVectorX / distance;
            float normalY = collisionVectorY / distance;

            // update på hastigheten efter collision
            setBallVelocity(pGame->pBall, normalX * BALL_SPEED_AFTER_COLLISION, normalY * BALL_SPEED_AFTER_COLLISION);
        }
    applyFriction(pGame->pBall); // Slow down the ball
    updateBallPosition(pGame->pBall);
    restrictBallWithinWindow(pGame->pBall);
}

void handleInput(Game *pGame, SDL_Event *event) {
    switch (event->type) {
        case SDL_KEYDOWN:
            switch (event->key.keysym.scancode) {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    updatePlayerVUp(pGame->pPlayer);
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    updatePlayerVDown(pGame->pPlayer);
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    updatePlayerVLeft(pGame->pPlayer);
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    updatePlayerVRight(pGame->pPlayer);
                    break;
            }
            break;
        case SDL_KEYUP:
            // Key release handling to stop movement
            switch (event->key.keysym.scancode) {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    resetPlayerSpeed(pGame->pPlayer, 0, 1);
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    resetPlayerSpeed(pGame->pPlayer, 1, 0);
                    break;
            }
            break;
    }
    restrictPlayerWithinWindow(pGame->pPlayer);
}

void closeGame(Game *pGame) {
    if (pGame->pBall) destroyBall(pGame->pBall);
    if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if (pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    SDL_Quit();
}

bool checkCollision(SDL_Rect rect1, SDL_Rect rect2) {
    if (rect1.y + rect1.h <= rect2.y) return false; // Bottom is above top
    if (rect1.y >= rect2.y + rect2.h) return false; // Top is below bottom
    if (rect1.x + rect1.w <= rect2.x) return false; // Right is left of left
    if (rect1.x >= rect2.x + rect2.w) return false; // Left is right of right
    return true; // Collided
}

void restrictPlayerWithinWindow(Player *pPlayer) {
    SDL_Rect playerRect = getPlayerRect(pPlayer);
    if (playerRect.x < 0) {
        setPlayerPosition(pPlayer, 0, playerRect.y);
    } else if (playerRect.x + playerRect.w > WINDOW_WIDTH) {
        setPlayerPosition(pPlayer, WINDOW_WIDTH - playerRect.w, playerRect.y);
    }
    if (playerRect.y < 0) {
        setPlayerPosition(pPlayer, playerRect.x, 0);
    } else if (playerRect.y + playerRect.h > WINDOW_HEIGHT) {
        setPlayerPosition(pPlayer, playerRect.x, WINDOW_HEIGHT - playerRect.h);
    }
}

void restrictBallWithinWindow(Ball *pBall) {
    SDL_Rect ballRect = getBallRect(pBall);
    if (ballRect.x < 0) {
        setBallX(pBall, 0);
        pBall->velocityX = -pBall->velocityX;
    } else if (ballRect.x + ballRect.w > WINDOW_WIDTH) {
        setBallX(pBall, WINDOW_WIDTH - ballRect.w);
        pBall->velocityX = -pBall->velocityX;
    }
    if (ballRect.y < 0) {
        setBallY(pBall, 0);
        pBall->velocityY = -pBall->velocityY;
    } else if (ballRect.y + ballRect.h > WINDOW_HEIGHT) {
        setBallY(pBall, WINDOW_HEIGHT - ballRect.h);
        pBall->velocityY = -pBall->velocityY;
    }
}
