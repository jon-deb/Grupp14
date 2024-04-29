#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_net.h>
#include <SDL_timer.h>
#include "ball.h"
#include "player.h"
#include "state.h"
#include "text.h"

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800
#define BALL_WINDOW_X1 64 //distance from left of window to left of field
#define BALL_WINDOW_X2 1236 //distance from left of window to right of field
#define BALL_WINDOW_Y1 114 //distance from top of window to top of field
#define BALL_WINDOW_Y2 765 //distance from top of window to bottom of field
#define MIDDLE_OF_FIELD_Y 440 //distance from top of window to mid point of field
#define MOVEMENT_SPEED 400
#define BALL_SPEED_AFTER_COLLISION 500
#define BORDER_SIZE 20
#define PLAYER_MAX 6

typedef struct game {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    SDL_Surface *pBackgroundSurface;
    SDL_Texture *backgroundTexture;
    TTF_Font *pFont, *pScoreboardFont; //, *pScoreFont; //*pTimerFont för annan storlek på timern
    Text *pStartText, *pClockText, *pScoreText, *pWaitingText; //, *pChooseTeamText, *pStartTimerText, *pMatchTimerText, *pScoreText, *pTeamNamesText;
    Player *pPlayer[PLAYER_MAX];
    Ball *pBall;
    int nrOfPlayers;
    GameState state;

    UDPsocket pSocket;
	IPaddress serverAddress;
	UDPpacket *pPacket;
} Game;

int initiate(Game *pGame);
void run(Game *pGame);
void closeGame(Game *pGame);
void handleInput(Game *pGame, SDL_Event *pEvent);
bool checkCollision(SDL_Rect rect1, SDL_Rect rect2);

void renderMenu(Game *pGame);
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
    if(TTF_Init()!=0){
        printf("Error: %s\n",TTF_GetError());
        SDL_Quit();
        return 0;
    }
    pGame->pWindow = SDL_CreateWindow("client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
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

    pGame->pFont = TTF_OpenFont("../lib/resources/ManaspaceRegular-ZJwZ.ttf", 20);
    pGame->pScoreboardFont = TTF_OpenFont("../lib/resources/ManaspaceRegular-ZJwZ.ttf", 50);
    if(!pGame->pFont || !pGame->pScoreboardFont){
        printf("Error: %s\n",TTF_GetError());
        closeGame(pGame);
        return 0;
    }

    pGame->pBackgroundSurface = IMG_Load("../lib/resources/field_v.3.png");
    if (!pGame->pBackgroundSurface) {
        printf("Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return 0;    
    }
    pGame->backgroundTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pGame->pBackgroundSurface);
    SDL_FreeSurface(pGame->pBackgroundSurface);
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

    pGame->pStartText = createText(pGame->pRenderer,238,168,65,pGame->pFont,"Press 1 to play multiplayer, q to exit",WINDOW_WIDTH/2,WINDOW_HEIGHT/2);
    pGame->pClockText = createText(pGame->pRenderer,227,220,198,pGame->pScoreboardFont,"05:00",790,63);
    pGame->pScoreText = createText(pGame->pRenderer,227,220,198,pGame->pScoreboardFont,"0-0",510,63);
    if(!pGame->pStartText || !pGame->pClockText || !pGame->pScoreText){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return 0;
    }

    pGame->state = MENU;
    return 1;
}

void run(Game *pGame) {
    int close_requested = 0;
    SDL_Event pEvent;
    Uint32 lastTick = SDL_GetTicks();
    Uint32 currentTick;
    float deltaTime;

    while (!close_requested) {
        switch(pGame->state) {
            case MENU: 
                if(SDL_PollEvent(&pEvent)) {
                    if(pEvent.type == SDL_QUIT || pEvent.key.keysym.scancode == SDL_SCANCODE_ESCAPE) close_requested = 1;
                    else if(pEvent.type == SDL_KEYDOWN && pEvent.key.keysym.scancode == SDL_SCANCODE_1) {
                        //användaren skriver in  ip adress
                        //när alla skrivit in rätt ip adress och connectat ändras state till playing
                        pGame->state=PLAYING;
                    }
                }
                renderMenu(pGame);
                break;
            case PLAYING:
                currentTick = SDL_GetTicks();
                deltaTime = (currentTick - lastTick) / 1000.0f;
                lastTick = currentTick;
                while (SDL_PollEvent(&pEvent)) {
                    if(pEvent.type == SDL_QUIT) close_requested = 1;
                    else handleInput(pGame, &pEvent);
                }
                for (int i = 0; i < pGame->nrOfPlayers; i++)
                {
                    updatePlayerPosition(pGame->pPlayer[i], deltaTime);
                    restrictPlayerWithinWindow(pGame->pPlayer[i], WINDOW_WIDTH, WINDOW_HEIGHT);
                    //updatePlayerPosition(pGame->pPlayer, deltaTime);
                }
                renderGame(pGame);
                break;
            case GAMEOVER:
                //drawtext team X won!
                pGame->state = MENU;
                break;
        }
    }
}

void renderMenu(Game *pGame){
    SDL_RenderClear(pGame->pRenderer);

    drawText(pGame->pStartText);

    SDL_RenderPresent(pGame->pRenderer);
}

void renderGame(Game *pGame) {
    SDL_RenderClear(pGame->pRenderer);
    SDL_RenderCopy(pGame->pRenderer, pGame->backgroundTexture, NULL, NULL);

    drawText(pGame->pClockText);
    drawText(pGame->pScoreText);
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

void handleInput(Game *pGame, SDL_Event *pEvent) {
    switch (pEvent->type) {
        case SDL_KEYDOWN:
            switch (pEvent->key.keysym.scancode) {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    updatePlayerVUp(pGame->pPlayer[0]);
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    updatePlayerVDown(pGame->pPlayer[0]);
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    updatePlayerVLeft(pGame->pPlayer[0]);
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    updatePlayerVRight(pGame->pPlayer[0]);
                    break;
            }
            break;
        case SDL_KEYUP:
            switch (pEvent->key.keysym.scancode) {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    resetPlayerSpeed(pGame->pPlayer[0], 0, 1);
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    resetPlayerSpeed(pGame->pPlayer[0], 1, 0);
                    break;
            }
            break;
    }
    restrictPlayerWithinWindow(pGame->pPlayer[0], WINDOW_WIDTH-300, WINDOW_HEIGHT-200);
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

    if(pGame->pStartText) destroyText(pGame->pStartText);   
    if(pGame->pFont) TTF_CloseFont(pGame->pFont);
    if(pGame->pClockText) destroyText(pGame->pClockText); 
    if(pGame->pScoreText) destroyText(pGame->pScoreText);   
    if(pGame->pScoreboardFont) TTF_CloseFont(pGame->pScoreboardFont);

    TTF_Quit();
    SDL_Quit();
}

bool checkCollision(SDL_Rect rect1, SDL_Rect rect2) {
    if (rect1.y + rect1.h <= rect2.y) return false; // Bottom is above top
    if (rect1.y >= rect2.y + rect2.h) return false; // Top is below bottom
    if (rect1.x + rect1.w <= rect2.x) return false; // Right is left of left
    if (rect1.x >= rect2.x + rect2.w) return false; // Left is right of right
    return true;
}



