#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
/*#include <windows.h>*/
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_net.h>
#include <SDL_timer.h>
#include "ball.h"
#include "player_data.h"
#include "player.h"
#include "power.h"
#include "text.h"

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800
#define BALL_WINDOW_X1 64 //distance from left of window to left of field
#define BALL_WINDOW_X2 1236 //distance from left of window to right of field
#define BALL_WINDOW_Y1 114 //distance from top of window to top of field
#define BALL_WINDOW_Y2 765 //distance from top of window to bottom of field
#define MIDDLE_OF_FIELD_Y 440 //distance from top of window to mid point of field
#define MOVEMENT_SPEED 400
#define NR_OF_POWERUPS 2

typedef struct game {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    SDL_Surface *pBackgroundSurface;
    SDL_Texture *backgroundTexture;
    TTF_Font *pFont, *pScoreboardFont;

    Text *pStartText,*pWaitingText, *pOverText, *pMatchTimerText, *pGoalsTextTeamA, *pGoalsTextTeamB, *pPowerUpText[2], *pPowerFrozenText, *pPowerSpeedText;
    Player *pPlayer[MAX_PLAYERS];
    Ball *pBall;
    PowerUpBox *pPowerUpBox;
    GameState state;

    int teamA;
    int teamB;
    int nrOfPlayers, playerNr;

    UDPsocket pSocket;
	IPaddress serverAddress;
	UDPpacket *pPacket;
    Uint32 matchTime;
} Game;

int initiate(Game *pGame);
void run(Game *pGame);
void renderGame(Game *pGame);
void handleInput(Game *pGame, SDL_Event *pEvent);
void handlePowerUpText(Game *pGame);

void updateWithServerData(Game *pGame);

Uint32 decreaseMatchTime(Uint32 interval, void *param);

void closeGame(Game *pGame);

int main(int argc, char** argv) {
    Game g = {0};
    if (!initiate(&g)) return 1;
    run(&g);
    closeGame(&g);
    return 0;
}

int initiate(Game *pGame) {
    srand(time(NULL));
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return 0;
    }
    if(TTF_Init()!=0){
        printf("Error: %s\n",TTF_GetError());
        SDL_Quit();
        return 0;
    }
    if (SDLNet_Init()) {
		printf("SDLNet_Init: %s\n", SDLNet_GetError());
        TTF_Quit();
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

    pGame->pFont = TTF_OpenFont("../lib/resources/SnesItalic-1G9Be.ttf", 70);
    pGame->pScoreboardFont = TTF_OpenFont("../lib/resources/ManaspaceRegular-ZJwZ.ttf", 50);
    if(!pGame->pFont || !pGame->pScoreboardFont){
        printf("Error: %s\n",TTF_GetError());
        closeGame(pGame);
        return 0;
    }

    if (!(pGame->pSocket = SDLNet_UDP_Open(0))) {//0 means not a server
		printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		return 0;
	}
	if (SDLNet_ResolveHost(&(pGame->serverAddress), "127.0.0.1", 2000)) {
		printf("SDLNet_ResolveHost(127.0.0.1 2000): %s\n", SDLNet_GetError());
		return 0;
	}
    if (!(pGame->pPacket = SDLNet_AllocPacket(512))) {
		printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		return 0;
	}
    pGame->pPacket->address.host = pGame->serverAddress.host;
    pGame->pPacket->address.port = pGame->serverAddress.port;

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

    for (int i = 0; i < MAX_PLAYERS; i++) {
        pGame->pPlayer[i] = createPlayer(pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, i);
        if (!pGame->pPlayer[i]) {
            fprintf(stderr, "Failed to initialize player %d\n", i + 1);
            return 0;
        }
    }
    pGame->nrOfPlayers = MAX_PLAYERS;

    pGame->pBall = createBall(pGame->pRenderer);
    if (!pGame->pBall) {
        printf("Error initializing the ball.\n");
        closeGame(pGame);
        return 0;
    }

    pGame->pPowerUpBox = createPower(pGame->pRenderer);
    if (!pGame->pPowerUpBox) {
        printf("Failed to initialize power cube.\n");
        closeGame(pGame);
        return 0;
    }
    spawnPowerCube(pGame->pPowerUpBox);

    pGame->teamA = 0;
    pGame->teamB = 0;
    pGame->pStartText = createText(pGame->pRenderer,238,168,65,pGame->pFont,"Press space to join",WINDOW_WIDTH/2,WINDOW_HEIGHT/2);
    pGame->pOverText = createText(pGame->pRenderer,238,168,65,pGame->pFont,"Game Over",WINDOW_WIDTH/2,WINDOW_HEIGHT/2);
    pGame->pWaitingText = createText(pGame->pRenderer,238,168,65,pGame->pFont,"Waiting for server...",WINDOW_WIDTH/2,WINDOW_HEIGHT/2);
    if(!pGame->pStartText || !pGame->pWaitingText || !pGame->pOverText){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return 0;
    }
    //pGame->pPowerUpText[0] = createText(pGame->pRenderer,255,87,51,pGame->pFont,"FROZEN",WINDOW_WIDTH/2,WINDOW_HEIGHT-100);
    //pGame->pPowerUpText[1] = createText(pGame->pRenderer,238,168,65,pGame->pFont,"Speed increased!",WINDOW_WIDTH/2,WINDOW_HEIGHT-100); //random värden, testar bara
    for(int i=0;i<MAX_PLAYERS;i++){
        if(!pGame->pPlayer[i]){
            printf("Error: %s\n",SDL_GetError());
            closeGame(pGame);
            return 0;
        }
    }

    pGame->state = START;
    pGame->matchTime = 300000;
    return 1;
}

void run(Game *pGame) {
    int close_requested = 0;
    SDL_Event event;
    ClientData cData;

    Uint32 lastTick = SDL_GetTicks();
    Uint32 currentTick;
    currentTick = SDL_GetTicks();
    float deltaTime;
    //SDL_TimerID timerID = SDL_AddTimer(1000, decreaseMatchTime, &(pGame->matchTime));
    SDL_TimerID timerID = 0;
    int joining = 0;

    while (!close_requested) {
        switch(pGame->state) 
        {
            case ONGOING:   
            if(timerID == 0) {
               timerID = SDL_AddTimer(1000, decreaseMatchTime, &(pGame->matchTime));
            }
                deltaTime = (currentTick - lastTick) / 1000.0f;
                lastTick = currentTick;

                while(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)){
                    updateWithServerData(pGame);
                }

                while (SDL_PollEvent(&event)) {
                    if(event.type == SDL_QUIT) {
                        close_requested = 1;
                    } else {
                        handleInput(pGame, &event);
                    }
                }

                for (int i = 0; i < MAX_PLAYERS; i++)
                {
                    updatePlayerPosition(pGame->pPlayer[i], deltaTime);
                    restrictPlayerWithinWindow(pGame->pPlayer[i], WINDOW_WIDTH, WINDOW_HEIGHT);
                }
                for (int i = 0; i < pGame->nrOfPlayers - 1; i++) {
                    for (int j = i + 1; j < pGame->nrOfPlayers; j++) {
                        handlePlayerCollision(pGame->pPlayer[i], pGame->pPlayer[j]);
                        freezeEnemyPlayer(pGame->pPlayer[i], pGame->pPlayer[j]);
                        applyFriction(pGame->pBall);
                    }
                }
                for (int i=0; i<pGame->nrOfPlayers; i++) {
                    SDL_Rect playerRect = getPlayerRect(pGame->pPlayer[i]);
                    SDL_Rect ballRect = getBallRect(pGame->pBall);
                    handlePlayerBallCollision(playerRect, ballRect, pGame->pBall);
                    if(checkCollision(playerRect, getPowerRect(pGame->pPowerUpBox))) {
                        updatePowerCube(pGame->pPowerUpBox, pGame->pRenderer, playerRect); 
                        int powerUpValue = rand()%NR_OF_POWERUPS;
                        assignPowerUp(powerUpValue, pGame->pPlayer[i]);
                    }   
                }

                if (!goal(pGame->pBall)) {
                    restrictBallWithinWindow(pGame->pBall);
                }
                else {
                    for(int i = 0; i < pGame->nrOfPlayers; i++) 
                        setStartingPosition(pGame->pPlayer[i], i, WINDOW_WIDTH, WINDOW_HEIGHT);
                //false if team left (A) scored and true if team right (B) scored
                    if (!goalScored(pGame->pBall)) {
                        pGame->teamA++;
                    } else if (goalScored) {
                        pGame->teamB++;
                    }
                }
                renderGame(pGame);

                break;
            case GAME_OVER:
                drawText(pGame->pOverText);
                break;
            case START:
                if (!joining)
                {
                    drawText(pGame->pStartText);
                }else{
                    SDL_RenderClear(pGame->pRenderer);
                    drawText(pGame->pWaitingText);
                }
                SDL_RenderPresent(pGame->pRenderer);
                if(SDL_PollEvent(&event)){
                    if(event.type==SDL_QUIT) close_requested = 1;
                    else if(!joining && event.type==SDL_KEYDOWN && event.key.keysym.scancode==SDL_SCANCODE_SPACE){
                        joining = 1;
                        cData.command=READY;
                        cData.clientNumber=-1;
                        memcpy(pGame->pPacket->data, &cData, sizeof(ClientData));
		                pGame->pPacket->len = sizeof(ClientData);
                    }
                }
                if(joining) SDLNet_UDP_Send(pGame->pSocket, -1,pGame->pPacket);
                if(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)){
                    updateWithServerData(pGame);
                    if(pGame->state == ONGOING) joining = 0;
                }                
                break;
        }
                //SDL_Delay(1000/60-15);//might work when you run on different processors
    }
    SDL_RemoveTimer(timerID);
}

void renderGame(Game *pGame) {
    SDL_RenderClear(pGame->pRenderer);
    SDL_RenderCopy(pGame->pRenderer, pGame->backgroundTexture, NULL, NULL);

    int minutes = pGame->matchTime / 60000;
    int seconds = (pGame->matchTime % 60000) / 1000;
    
    char timeString[10];
    char goalsStringTeamA[5];
    char goalsStringTeamB[5];

    sprintf(timeString, "%02d:%02d", minutes, seconds);
    snprintf(goalsStringTeamB, sizeof(goalsStringTeamB), "%d", pGame->teamB);
    snprintf(goalsStringTeamA, sizeof(goalsStringTeamA), "%d:", pGame->teamA);

    pGame->pMatchTimerText = createText(pGame->pRenderer, 227, 220, 198, pGame->pScoreboardFont, timeString, 790, 64);
    pGame->pGoalsTextTeamA = createText(pGame->pRenderer, 227, 220, 198, pGame->pScoreboardFont, goalsStringTeamA, 494, 64);
    pGame->pGoalsTextTeamB = createText(pGame->pRenderer, 227, 220, 198, pGame->pScoreboardFont, goalsStringTeamB, 542, 64); 

    if(!pGame->pMatchTimerText || !pGame->pGoalsTextTeamA || !pGame->pGoalsTextTeamB){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
    } 
    drawText(pGame->pGoalsTextTeamA);
    drawText(pGame->pGoalsTextTeamB);
    drawText(pGame->pMatchTimerText);
    handlePowerUpText(pGame);
    destroyText(pGame->pMatchTimerText);
    destroyText(pGame->pGoalsTextTeamA);
    destroyText(pGame->pGoalsTextTeamB);
    
    for (int i = 0; i < MAX_PLAYERS; i++) {
        Player *player = pGame->pPlayer[i];
        SDL_Rect playerRect = getPlayerRect(player);
        SDL_Texture *playerTexture = getPlayerTexture(player);
        SDL_RenderCopy(pGame->pRenderer, playerTexture, NULL, &playerRect);
    }

    SDL_Rect ballRect = getBallRect(pGame->pBall);
    SDL_Texture *ballTexture = getBallTexture(pGame->pBall);
    SDL_RenderCopy(pGame->pRenderer, ballTexture, NULL, &ballRect);
    renderPowerCube(pGame->pPowerUpBox, pGame->pRenderer);
    SDL_RenderPresent(pGame->pRenderer);
    SDL_Delay(1000/60); 
}

void updateWithServerData(Game *pGame){
    ServerData sData;
    memcpy(&sData, pGame->pPacket->data, sizeof(ServerData));
    pGame->playerNr = sData.clientNr;
    pGame->state = sData.gState;
    for(int i=0;i<MAX_PLAYERS;i++){
        updatePlayerWithRecievedData(pGame->pPlayer[i],&(sData.players[i]));
    }
    updateBallWithRecievedData(pGame->pBall,&(sData.ball));
}

void handlePowerUpText(Game *pGame) {
    pGame->pPowerUpText[0] = createText(pGame->pRenderer,255,87,51,pGame->pFont,"FROZEN",WINDOW_WIDTH/2,WINDOW_HEIGHT-100);
    pGame->pPowerUpText[1] = createText(pGame->pRenderer,238,168,65,pGame->pFont,"Speed increased!",WINDOW_WIDTH/2,WINDOW_HEIGHT-100); //random värden, testar bara
    for(int i=0; i<NR_OF_POWERUPS; i++) {
        if(!pGame->pPowerUpText[i]) {
            printf("Error: %s\n",SDL_GetError());
            closeGame(pGame);
        }
    }
    if(getCurrentPowerUp(pGame->pPlayer[pGame->playerNr]) == FROZEN) {
        /*if(!pGame->pPowerUpText[0]) {
            printf("Error: %s\n",SDL_GetError());
            closeGame(pGame);
        }*/
        drawText(pGame->pPowerUpText[0]);
    }
    else if(getCurrentPowerUp(pGame->pPlayer[pGame->playerNr]) == SPEED_BOOST) {
        /*if(!pGame->pPowerUpText[1]) {
            printf("Error: %s\n",SDL_GetError());
            closeGame(pGame);
        }*/
        drawText(pGame->pPowerUpText[1]);
    }
    for(int i=0; i<NR_OF_POWERUPS; i++) destroyText(pGame->pPowerUpText[i]);
}

void handleInput(Game *pGame, SDL_Event *pEvent) {
    ClientData cData;
    cData.clientNumber = pGame->playerNr;
    
    //PowerUp currentPower;
    //currentPower = getCurrentPowerUp(pGame->pPlayer[pGame->playerNr]);
    if(getCurrentPowerUp(pGame->pPlayer[pGame->playerNr]) != FROZEN) {
        if (pEvent->type == SDL_KEYDOWN) {
            switch (pEvent->key.keysym.scancode) {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    updatePlayerVUp(pGame->pPlayer[pGame->playerNr]);
                    cData.command = UP;
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    updatePlayerVDown(pGame->pPlayer[pGame->playerNr]);
                    cData.command = DOWN;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    updatePlayerVLeft(pGame->pPlayer[pGame->playerNr]);
                    cData.command = LEFT;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    updatePlayerVRight(pGame->pPlayer[pGame->playerNr]);
                    cData.command = RIGHT;
                    break;
            }
            memcpy(pGame->pPacket->data, &cData, sizeof(ClientData));
            pGame->pPacket->len = sizeof(ClientData);
            SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
        } else if (pEvent->type == SDL_KEYUP) {
            bool sendUpdate = false;
            switch (pEvent->key.keysym.scancode) {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    if (!SDL_GetKeyboardState(NULL)[SDL_SCANCODE_W] && !SDL_GetKeyboardState(NULL)[SDL_SCANCODE_UP] &&
                        !SDL_GetKeyboardState(NULL)[SDL_SCANCODE_S] && !SDL_GetKeyboardState(NULL)[SDL_SCANCODE_DOWN]) {
                        resetPlayerSpeed(pGame->pPlayer[pGame->playerNr], 0, 1);
                        cData.command = RESET_Y_VEL;
                        sendUpdate = true;
                    }
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    if (!SDL_GetKeyboardState(NULL)[SDL_SCANCODE_A] && !SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LEFT] &&
                        !SDL_GetKeyboardState(NULL)[SDL_SCANCODE_D] && !SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RIGHT]) {
                        resetPlayerSpeed(pGame->pPlayer[pGame->playerNr], 1, 0);
                        cData.command = RESET_X_VEL;
                        sendUpdate = true;
                    }
                    break;
            }
            if (sendUpdate) {
                memcpy(pGame->pPacket->data, &cData, sizeof(ClientData));
                pGame->pPacket->len = sizeof(ClientData);
                SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
            }
        }
    }
}

Uint32 decreaseMatchTime(Uint32 interval, void *param) {
    Uint32 *pMatchTime = (Uint32 *)param;
    if (*pMatchTime > 0) {
        *pMatchTime -= 1000;
    }
    return interval;
}

void closeGame(Game *pGame) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if(pGame->pPlayer[i]) {
            destroyPlayer(pGame->pPlayer[i]);
        }
    }
    if (pGame->pBall) destroyBall(pGame->pBall);
    if (pGame->pPowerUpBox) destroyPowerCube(pGame->pPowerUpBox);
    if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if (pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);

    if(pGame->pStartText) destroyText(pGame->pStartText);   
    if(pGame->pWaitingText) destroyText(pGame->pWaitingText); 
    if(pGame->pOverText) destroyText(pGame->pOverText); 
    if(pGame->pMatchTimerText) destroyText(pGame->pMatchTimerText);
    if(pGame->pGoalsTextTeamA) destroyText(pGame->pGoalsTextTeamA);
    if(pGame->pGoalsTextTeamB) destroyText(pGame->pGoalsTextTeamB);
    for(int i=0; i<NR_OF_POWERUPS; i++) destroyText(pGame->pPowerUpText[i]);
    if(pGame->pFont) TTF_CloseFont(pGame->pFont);
    if(pGame->pScoreboardFont) TTF_CloseFont(pGame->pScoreboardFont);

    SDLNet_Quit();
    TTF_Quit();
    SDL_Quit();
}