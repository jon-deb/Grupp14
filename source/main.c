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
#define BORDER_SIZE 20
#define GOAL_TOP 352
#define GOAL_BOTTOM 448
#define PLAYER_MAX 6

typedef struct game {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Ball *pBall;
    SDL_Texture *backgroundTexture;
    Player *pPlayer[PLAYER_MAX];
    int nrOfPlayers;
} Game;

int initiate(Game *pGame);
void run(Game *pGame);
void closeGame(Game *pGame);
void handleInput(Game *pGame, SDL_Event *event);
bool checkCollision(SDL_Rect rect1, SDL_Rect rect2);

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
    SDL_Surface *backgroundSurface = IMG_Load("resources/newfield.png");
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

    pGame->nrOfPlayers = 2;
    for (int i = 0; i < pGame->nrOfPlayers; i++) {
        pGame->pPlayer[i] = createPlayer(pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, i);
        if (!pGame->pPlayer[i]) {
            fprintf(stderr, "Failed to initialize player %d\n", i + 1);

            return 0;
        }
    }

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
        deltaTime = (currentTick - lastTick) / 1000.0f;
        lastTick = currentTick;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) close_requested = 1;
            else handleInput(pGame, &event);
        }
        for (int i = 0; i < pGame->nrOfPlayers; i++)
        {
            updatePlayerPosition(pGame->pPlayer[i], deltaTime);
            restrictPlayerWithinWindow(pGame->pPlayer[i], WINDOW_WIDTH, WINDOW_HEIGHT);
            //updatePlayerPosition(pGame->pPlayer, deltaTime);
        }
        renderGame(pGame);
    }
}

void renderGame(Game *pGame) {
    SDL_RenderClear(pGame->pRenderer);
    SDL_RenderCopy(pGame->pRenderer, pGame->backgroundTexture, NULL, NULL);

    for (int i = 0; i < pGame->nrOfPlayers; i++) {
        Player *player = pGame->pPlayer[i];
        SDL_Rect playerRect = getPlayerRect(player);
        SDL_Texture *playerTexture = getPlayerTexture(player);
        SDL_RenderCopy(pGame->pRenderer, playerTexture, NULL, &playerRect);
    }

    SDL_Rect ballRect = getBallRect(pGame->pBall);
    SDL_Texture *ballTexture = getBallTexture(pGame->pBall);
    SDL_RenderCopy(pGame->pRenderer, ballTexture, NULL, &ballRect);

    SDL_RenderPresent(pGame->pRenderer);
    SDL_Delay(1000/60); 
    handleCollisionsAndPhysics(pGame);
}

void handleCollisionsAndPhysics(Game *pGame) {
    for (int i = 0; i < pGame->nrOfPlayers; i++) {
        Player *currentPlayer = pGame->pPlayer[i];
        SDL_Rect playerRect = getPlayerRect(currentPlayer);
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
    }
    applyFriction(pGame->pBall);
    updateBallPosition(pGame->pBall);
    if (!goal(pGame->pBall))
    {
        restrictBallWithinWindow(pGame->pBall);
    }
    else
    {
        for (int i = 0; i < pGame->nrOfPlayers; i++)
        {
            resetPlayerPos(pGame->pPlayer[i], i, WINDOW_WIDTH, WINDOW_HEIGHT);
        }
    }
}

void handleInput(Game *pGame, SDL_Event *event) {
    for (int i = 0; i < pGame->nrOfPlayers; i++) {
        Player *player = pGame->pPlayer[i];
        switch (event->type) {
            case SDL_KEYDOWN:
                if (i == 0) {
                    switch (event->key.keysym.scancode) {
                        case SDL_SCANCODE_W: updatePlayerVUp(player); break;
                        case SDL_SCANCODE_S: updatePlayerVDown(player); break;
                        case SDL_SCANCODE_A: updatePlayerVLeft(player); break;
                        case SDL_SCANCODE_D: updatePlayerVRight(player); break;
                    }
                } else if (i == 1) {
                    switch (event->key.keysym.scancode) {
                        case SDL_SCANCODE_UP: updatePlayerVUp(player); break;
                        case SDL_SCANCODE_DOWN: updatePlayerVDown(player); break;
                        case SDL_SCANCODE_LEFT: updatePlayerVLeft(player); break;
                        case SDL_SCANCODE_RIGHT: updatePlayerVRight(player); break;
                    }
                }
                break;
            case SDL_KEYUP:
                if (i == 0) {
                    switch (event->key.keysym.scancode) {
                        case SDL_SCANCODE_W:
                        case SDL_SCANCODE_S:
                            resetPlayerSpeed(pGame->pPlayer[i], 0, 1);
                            break;
                        case SDL_SCANCODE_A:
                        case SDL_SCANCODE_D:
                            resetPlayerSpeed(pGame->pPlayer[i], 1, 0);
                            break;
                    }   
                } else if (i == 1) {
                    switch (event->key.keysym.scancode) {
                        case SDL_SCANCODE_UP:
                        case SDL_SCANCODE_DOWN:
                            resetPlayerSpeed(pGame->pPlayer[i], 0, 1);
                            break;
                        case SDL_SCANCODE_LEFT:
                        case SDL_SCANCODE_RIGHT:
                            resetPlayerSpeed(pGame->pPlayer[i], 1, 0);
                            break;
                    }
                }
                break;
        }
        restrictPlayerWithinWindow(pGame->pPlayer[i], WINDOW_WIDTH, WINDOW_HEIGHT);
    }
}

void closeGame(Game *pGame) {
    for (int i = 0; i < pGame->nrOfPlayers; i++) {
        if (pGame->pPlayer[i]) {
            destroyPlayer(pGame->pPlayer[i]);
        }
    }
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
    return true;
}



