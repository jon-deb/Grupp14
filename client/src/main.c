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
#define STRING_LENGTH 50

typedef struct game {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    SDL_Surface *pBackgroundSurface, *pIpSurface;
    SDL_Texture *backgroundTexture, *pIpTexture;
    TTF_Font *pFont, *pScoreboardFont, *pLobbyFont;

    Text *pOverTextA, *pOverTextB, *pOverTextC, *pOverTextD,*pMatchTimerText, *pGoalsTextTeamA, *pGoalsTextTeamB, *pHostSpotText, *pSpot1Text, *pSpot2Text, *pSpot3Text, *pSpot4Text, *pLobbyText, *pPowerUpText[NR_OF_POWERUPS], *pIpText, *pEnterIpText, *pOutputIpText;
    Player *pPlayer[MAX_PLAYERS];
    Ball *pBall;
    PowerUpBox *pPowerUpBox;
    GameState state;
    ClientData clients[MAX_PLAYERS];
    bool connected[MAX_PLAYERS];
    int hostConnected;

    char pIP[50];
    
    int teamA;
    int teamB;
    int nrOfPlayers, playerNr;
    int powerUpValue;

    UDPsocket pSocket;
	IPaddress serverAddress;
	UDPpacket *pPacket;
    Uint32 matchTime;
} Game;

int initiate(Game *pGame);
void run(Game *pGame);
void renderGame(Game *pGame);
void renderLobby(Game *pGame);
void handleInput(Game *pGame, SDL_Event *pEvent);
void handlePowerUpText(Game *pGame);
void updateWithServerData(Game *pGame);
Uint32 decreaseMatchTime(Uint32 interval, void *param);
void getInputIP(Game *pGame);

void handleGameOverText(Game *pGame);
void closeGame(Game *pGame);

int main(int argc, char** argv) {
    Game g = {0};
    if (!initiate(&g)) return 1;
    srand(500);
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

    if (!(pGame->pSocket = SDLNet_UDP_Open(0))) {//0 means not a server
		printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		return 0;
	}
    if (!(pGame->pPacket = SDLNet_AllocPacket(512))) {
		printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		return 0;
	}
    pGame->pPacket->address.host = pGame->serverAddress.host;
    pGame->pPacket->address.port = pGame->serverAddress.port;

    pGame->pBackgroundSurface = IMG_Load("../lib/resources/field_v.3.png");
    pGame->pIpSurface = IMG_Load("../lib/resources/field_v.3.png");
    if (!pGame->pBackgroundSurface || !pGame->pIpSurface) {
        printf("Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return 0;    
    }
    pGame->backgroundTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pGame->pBackgroundSurface);
    SDL_FreeSurface(pGame->pBackgroundSurface);
    pGame->pIpTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pGame->pIpSurface);
    SDL_FreeSurface(pGame->pIpSurface);
    if (!pGame->backgroundTexture || !pGame->pIpTexture) {
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

    pGame->pFont = TTF_OpenFont("../lib/resources/SnesItalic-1G9Be.ttf", 100);
    pGame->pScoreboardFont = TTF_OpenFont("../lib/resources/ManaspaceRegular-ZJwZ.ttf", 50);
    pGame->pLobbyFont = TTF_OpenFont("../lib/resources/SnesItalic-1G9Be.ttf", 50);
    if(!pGame->pFont || !pGame->pScoreboardFont || !pGame->pLobbyFont){
        printf("Error: %s\n",TTF_GetError());
        closeGame(pGame);
        return 0;
    }
    pGame->pOverTextA = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "Team A Won!", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    pGame->pOverTextB = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "Team B Won!\n", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    pGame->pOverTextC = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "Game Draw!", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    pGame->pOverTextD = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "Closing in 5 sec...", WINDOW_WIDTH /2, WINDOW_HEIGHT / 2+100);
    if(!pGame->pOverTextA || !pGame->pOverTextB || !pGame->pOverTextC || !pGame->pOverTextD){
        printf("Error: %s\n",TTF_GetError());
        closeGame(pGame);
        return 0;
    }
    
    pGame->teamA = 0;
    pGame->teamB = 0;
    pGame->powerUpValue = 0;

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
    SDL_TimerID timerID = 0;
    int joining = 0;
    pGame->hostConnected = 0;
    strcpy(pGame->pIP, "Null");

    while (!close_requested) {
        switch(pGame->state) {
            case ONGOING:
                if(timerID == 0) timerID = SDL_AddTimer(1000, decreaseMatchTime, pGame);

                //currentTick = SDL_GetTicks();
                deltaTime = (currentTick - lastTick) / 1000.0f;
                lastTick = currentTick;
                while(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)) {
                    updateWithServerData(pGame);
                }

                while (SDL_PollEvent(&event)) {
                    if(event.type == SDL_QUIT) close_requested = 1;
                    else handleInput(pGame, &event);
                }

                for (int i = 0; i < MAX_PLAYERS; i++) {
                    updatePlayerPosition(pGame->pPlayer[i], deltaTime);
                    restrictPlayerWithinWindow(pGame->pPlayer[i], WINDOW_WIDTH, WINDOW_HEIGHT);
                }

                for (int i = 0; i < pGame->nrOfPlayers - 1; i++) {
                    for (int j = i + 1; j < pGame->nrOfPlayers; j++) {
                        handlePlayerCollision(pGame->pPlayer[i], pGame->pPlayer[j]);
                        freezeEnemyPlayer(pGame->pPlayer[i], pGame->pPlayer[j]);
                    }
                }

                for (int i = 0; i < pGame->nrOfPlayers; i++) {
                    SDL_Rect playerRect = getPlayerRect(pGame->pPlayer[i]);
                    SDL_Rect ballRect = getBallRect(pGame->pBall);
                    handlePlayerBallCollision(playerRect, ballRect, pGame->pBall);
                    if(checkCollision(playerRect, getPowerRect(pGame->pPowerUpBox))) {
                        updatePowerCube(pGame->pPowerUpBox, pGame->pRenderer, playerRect); 
                        assignPowerUp(pGame->powerUpValue, pGame->pPlayer[i]);
                    }
                }

                if (!goal(pGame->pBall)) {
                    restrictBallWithinWindow(pGame->pBall);
                } else {
                    for(int i = 0; i < pGame->nrOfPlayers; i++)
                        setStartingPosition(pGame->pPlayer[i], i, WINDOW_WIDTH, WINDOW_HEIGHT);

                    if (!goalScored(pGame->pBall)) {
                        pGame->teamA++;
                    } else {
                        pGame->teamB++;
                    }
                }
                applyFriction(pGame->pBall);
                renderGame(pGame);
                break;

        case GAME_OVER:
            Uint32 gameOverStartTime = SDL_GetTicks();
            handleGameOverText(pGame);
            while (!close_requested) {
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT || SDL_GetTicks() - gameOverStartTime >= 5000) {
                        close_requested = 1;
                        closeGame(pGame);
                    }
                }
            }
            break;
        case START:
            renderLobby(pGame);
            if(SDL_PollEvent(&event)) {
                if(event.type == SDL_QUIT) {
                    close_requested = 1;
                } 
                else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                    getInputIP(pGame);

                    if (SDLNet_ResolveHost(&(pGame->serverAddress), pGame->pIP, 2000)) {
                        printf("SDLNet_ResolveHost(%s 2000): %s\n", pGame->pIP, SDLNet_GetError());
                        closeGame(pGame);
                    }

                    // Update the packet address
                    pGame->pPacket->address.host = pGame->serverAddress.host;
                    pGame->pPacket->address.port = pGame->serverAddress.port;

                    if(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)) {
                        pGame->hostConnected = 1;
                        updateWithServerData(pGame);
                    }
                }
                else if(!joining || pGame->hostConnected == 0) {
                    joining = 1;
                    cData.command = READY;
                    cData.clientNumber = -1;
                    memcpy(pGame->pPacket->data, &cData, sizeof(ClientData));
                    pGame->pPacket->len = sizeof(ClientData);
                    for (int i = 0; i < 20; i++) {
                        SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
                    }
                }
            }
            if(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)) {
                updateWithServerData(pGame);
                if(pGame->state == ONGOING) joining = 0;
            }
            break;
        }
    }
    SDL_RemoveTimer(timerID);
}


void renderLobby(Game *pGame){
    SDL_RenderClear(pGame->pRenderer);

    char lobbyString[50];
    char hostSpotString[50];
    char spot1String[50];
    char spot2String[50];
    char spot3String[50];
    char spot4String[50];
    char inputIpString[50];

    if (pGame->hostConnected)
    {
        snprintf(lobbyString, sizeof(lobbyString), "Lobby. You are player %d", pGame->playerNr+1);
        snprintf(hostSpotString, sizeof(hostSpotString), "Host is connected");
    }
    else
    {
        snprintf(lobbyString, sizeof(lobbyString), "Lobby");
        snprintf(hostSpotString, sizeof(hostSpotString), "Looking for host...");
    }
    

    if(pGame->connected[0] == true) {
        snprintf(spot1String, sizeof(spot1String), "Player 1 is connected");
    } else {
        snprintf(hostSpotString, sizeof(hostSpotString), "Waiting for host to connect");
        snprintf(spot1String, sizeof(spot1String), "Spot 1 is available");
    }
    if(pGame->connected[1] == true) {
        snprintf(spot2String, sizeof(spot2String), "Player 2 is connected");
    } else {
        snprintf(spot2String, sizeof(spot2String), "Spot 2 is available");
    }
    if(pGame->connected[2] == true) {
        snprintf(spot3String, sizeof(spot3String), "Player 3 is connected");
    } else {
        snprintf(spot3String, sizeof(spot3String), "Spot 3 is available");
    }
    if(pGame->connected[3] == true) {
        snprintf(spot4String, sizeof(spot4String), "Player 4 is connected");
    } else {
        snprintf(spot4String, sizeof(spot4String), "Spot 4 is available");
    }

    snprintf(inputIpString, sizeof(inputIpString), "Press Enter to edit IP");

    pGame->pLobbyText = createText(pGame->pRenderer, 227, 220, 198, pGame->pLobbyFont, lobbyString, WINDOW_WIDTH/2, 100);
    pGame->pHostSpotText = createText(pGame->pRenderer, 227, 220, 198, pGame->pLobbyFont, hostSpotString, WINDOW_WIDTH/2, 200);
    pGame->pSpot1Text = createText(pGame->pRenderer, 250, 220, 198, pGame->pLobbyFont, spot1String, WINDOW_WIDTH/2, 300);
    pGame->pSpot2Text = createText(pGame->pRenderer, 227, 220, 198, pGame->pLobbyFont, spot2String, WINDOW_WIDTH/2, 350);
    pGame->pSpot3Text = createText(pGame->pRenderer, 227, 220, 198, pGame->pLobbyFont, spot3String, WINDOW_WIDTH/2, 400);
    pGame->pSpot4Text = createText(pGame->pRenderer, 227, 220, 198, pGame->pLobbyFont, spot4String, WINDOW_WIDTH/2, 450);
    pGame->pEnterIpText = createText(pGame->pRenderer, 227, 220, 198, pGame->pLobbyFont, inputIpString, WINDOW_WIDTH/2, 600);
    pGame->pOutputIpText = createText(pGame->pRenderer, 227, 220, 198, pGame->pLobbyFont, pGame->pIP, WINDOW_WIDTH/2, 650);
    
    drawText(pGame->pOutputIpText);

    drawText(pGame->pLobbyText);
    drawText(pGame->pHostSpotText);
    drawText(pGame->pSpot1Text);
    drawText(pGame->pSpot2Text);
    drawText(pGame->pSpot3Text);
    drawText(pGame->pSpot4Text);
    drawText(pGame->pEnterIpText);
    destroyText(pGame->pHostSpotText);
    destroyText(pGame->pSpot1Text);
    destroyText(pGame->pSpot2Text);
    destroyText(pGame->pSpot3Text);
    destroyText(pGame->pSpot4Text);
    destroyText(pGame->pLobbyText);
    destroyText(pGame->pEnterIpText);
    destroyText(pGame->pOutputIpText);
    
    SDL_RenderPresent(pGame->pRenderer);
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
    pGame->powerUpValue = sData.powerUpValue;
    for(int i=0;i<MAX_PLAYERS;i++){
        updatePlayerWithRecievedData(pGame->pPlayer[i],&(sData.players[i]));
        pGame->connected[i] = sData.connected[i];
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
    if(getCurrentPowerUp(pGame->pPlayer[pGame->playerNr]) == FROZEN)
        drawText(pGame->pPowerUpText[0]);
    else if(getCurrentPowerUp(pGame->pPlayer[pGame->playerNr]) == SPEED_BOOST)
        drawText(pGame->pPowerUpText[1]);

    for(int i=0; i<NR_OF_POWERUPS; i++) {
        if(pGame->pPowerUpText[i]) destroyText(pGame->pPowerUpText[i]);
    }
}

void handleInput(Game *pGame, SDL_Event *pEvent) {
    ClientData cData;
    cData.clientNumber = pGame->playerNr;
    
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

void handleGameOverText(Game *pGame) {
    if(pGame->teamA > pGame->teamB) {
        SDL_RenderClear(pGame->pRenderer);
        drawText(pGame->pOverTextA);
        drawText(pGame->pOverTextD);
        SDL_RenderPresent(pGame->pRenderer);  
    } else if(pGame->teamB > pGame->teamA) {
        SDL_RenderClear(pGame->pRenderer);
        drawText(pGame->pOverTextB);
        drawText(pGame->pOverTextD);
        SDL_RenderPresent(pGame->pRenderer);
    } else {
        SDL_RenderClear(pGame->pRenderer);
        drawText(pGame->pOverTextC);
        drawText(pGame->pOverTextD);
        SDL_RenderPresent(pGame->pRenderer);
    }
}

Uint32 decreaseMatchTime(Uint32 interval, void *param) {
    Game *pGame = (Game *)param;
    if (pGame->matchTime > 0) {
        pGame->matchTime -= 1000;
    } else {
        pGame->state = GAME_OVER;
    }
    return interval;
}

void getInputIP(Game *pGame){
    SDL_StartTextInput();
    SDL_RenderClear(pGame->pRenderer);
    int close_requested = 0;
    SDL_Event event;
    char inputText[STRING_LENGTH];
    char inputIpString[STRING_LENGTH];
    char outputIpString[STRING_LENGTH];
    strcpy(inputText, "");
                        
    while (!close_requested)
    {
        while (SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                close_requested = 1;
            }
            else if (event.type == SDL_TEXTINPUT)
            {
                if (strlen(inputText) + strlen(event.text.text) < STRING_LENGTH - 1)
                {
                    strcat(inputText, event.text.text);
                }
            }
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(inputText) > 0)
                {
                    inputText[strlen(inputText) - 1] = '\0';
                }
                else if (event.key.keysym.sym == SDLK_RETURN)
                {
                    if (strlen(inputText) > 0)
                    {
                        strcpy(pGame->pIP, inputText);
                    }
                    close_requested = 1;
                }
            }
        }
        SDL_RenderClear(pGame->pRenderer);
        
        snprintf(inputIpString, sizeof(inputIpString), "Enter IP");
        pGame->pEnterIpText = createText(pGame->pRenderer, 227, 220, 198, pGame->pLobbyFont, inputIpString, WINDOW_WIDTH/2, 600);
        drawText(pGame->pEnterIpText);
        destroyText(pGame->pEnterIpText);
        
        snprintf(outputIpString, sizeof(outputIpString), " %s", inputText);
        pGame->pOutputIpText = createText(pGame->pRenderer, 227, 220, 198, pGame->pLobbyFont, outputIpString, WINDOW_WIDTH/2, 650);
        drawText(pGame->pOutputIpText);
        destroyText(pGame->pOutputIpText);
        SDL_RenderPresent(pGame->pRenderer);
    }
    SDL_StopTextInput();
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

    if(pGame->pLobbyText) destroyText(pGame->pLobbyText);
    if(pGame->pSpot1Text) destroyText(pGame->pSpot1Text);
    if(pGame->pSpot2Text) destroyText(pGame->pSpot2Text);
    if(pGame->pSpot3Text) destroyText(pGame->pSpot3Text);
    if(pGame->pSpot4Text) destroyText(pGame->pSpot4Text);
    if(pGame->pHostSpotText) destroyText(pGame->pHostSpotText);
    if(pGame->pOverTextA) destroyText(pGame->pOverTextA); 
    if(pGame->pOverTextB) destroyText(pGame->pOverTextB); 
    if(pGame->pOverTextC) destroyText(pGame->pOverTextC); 
    if(pGame->pOverTextD) destroyText(pGame->pOverTextD);
    if(pGame->pMatchTimerText) destroyText(pGame->pMatchTimerText);
    if(pGame->pGoalsTextTeamA) destroyText(pGame->pGoalsTextTeamA);
    if(pGame->pGoalsTextTeamB) destroyText(pGame->pGoalsTextTeamB);
    if(pGame->pIpText) destroyText(pGame->pIpText);
    if(pGame->pEnterIpText) destroyText(pGame->pEnterIpText);


    for(int i=0; i<NR_OF_POWERUPS; i++) 
        if(pGame->pPowerUpText[i]) destroyText(pGame->pPowerUpText[i]);
    if(pGame->pFont) TTF_CloseFont(pGame->pFont);
    if(pGame->pScoreboardFont) TTF_CloseFont(pGame->pScoreboardFont);

    SDLNet_Quit();
    TTF_Quit();
    SDL_Quit();
}