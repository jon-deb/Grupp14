#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
/*#include <windows.h>*/
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_net.h>
#include "player_data.h"
#include "player.h"
#include "text.h"
#include "ball.h"
#include "power.h"

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800
#define MIDDLE_OF_FIELD_Y 440 //distance from top of window to mid point of field
#define MOVEMENT_SPEED 400
#define NR_OF_POWERUPS 2

typedef struct game{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    SDL_Surface *pBackgroundSurface;
    SDL_Texture *backgroundTexture;
    TTF_Font *pFont, *pScoreboardFont, *pLobbyFont;
    
    Player *pPlayer[MAX_PLAYERS];
    Ball *pBall;
    PowerUpBox *pPowerUpBox;
    Text *pOverText, *pMatchTimerText, *pGoalsTextTeamA, *pGoalsTextTeamB, *pHostSpotText, *pSpot1Text, *pSpot2Text, *pSpot3Text, *pSpot4Text, *pLobbyText;
    GameState state;
    ServerData sData;
    
    bool connected[MAX_PLAYERS];
    int teamA;
    int teamB;
    int nrOfClients, nrOfPlayers;

    UDPsocket pSocket;
    UDPpacket *pPacket;
    Uint32 matchTime;
    IPaddress clients[MAX_PLAYERS];
} Game;

int initiate(Game *pGame);
void run(Game *pGame);
void renderGame(Game *pGame);
void renderLobby(Game *pGame);
void handleInput(Game *pGame, SDL_Event *pEvent);

void add(IPaddress address, IPaddress clients[],int *pNrOfClients, bool connected[]);
void sendGameData(Game *pGame);
void executeCommand(Game *pGame,ClientData cData);

Uint32 decreaseMatchTime(Uint32 interval, void *param);

void setUpGame(Game *pGame);
void closeGame(Game *pGame);

int main(int argv, char** args){
    Game g={0};
    if(!initiate(&g)) return 1;
    srand(500);
    run(&g);
    closeGame(&g);
    return 0;
}

int initiate(Game *pGame){
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0){
        printf("Error: %s\n",SDL_GetError());
        return 0;
    }
    if(TTF_Init()!=0){
        printf("Error: %s\n",TTF_GetError());
        SDL_Quit();
        return 0;
    }
    if (SDLNet_Init())
	{
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
        TTF_Quit();
        SDL_Quit();
		return 0;
	}
    pGame->pWindow = SDL_CreateWindow("Server",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WINDOW_WIDTH,WINDOW_HEIGHT,0);
    if(!pGame->pWindow){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return 0;
    }
    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if(!pGame->pRenderer){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return 0;    
    }

    pGame->pFont = TTF_OpenFont("../lib/resources/SnesItalic-1G9Be.ttf", 100);
    pGame->pScoreboardFont = TTF_OpenFont("../lib/resources/ManaspaceRegular-ZJwZ.ttf", 50);
    pGame->pLobbyFont = TTF_OpenFont("../lib/resources/SnesItalic-1G9Be.ttf", 50);
    if(!pGame->pFont || !pGame->pScoreboardFont || !pGame->pLobbyFont){
        printf("Error: %s\n",TTF_GetError());
        closeGame(pGame);
        return 0;
    }

    if (!(pGame->pSocket = SDLNet_UDP_Open(2000)))
	{
		printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		closeGame(pGame);
        return 0;
	}
	if (!(pGame->pPacket = SDLNet_AllocPacket(512)))
	{
		printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
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

    for(int i=0;i<MAX_PLAYERS;i++){
        if(!pGame->pPlayer[i]){
            printf("Error: %s\n",SDL_GetError());
            closeGame(pGame);
            return 0;
        }
    }

    pGame->state = START;
    pGame->nrOfClients = 0;
    for(int i = 0; i < MAX_PLAYERS; i++) {
        pGame->connected[i] = false;
    }
    pGame->matchTime = 300000;
    
    return 1;
}

void run(Game *pGame){
    int close_requested = 0;
    SDL_Event event;
    ClientData cData;

    Uint32 lastTick = SDL_GetTicks();
    Uint32 currentTick;
    float deltaTime;

    SDL_TimerID timerID = 0;

    while(!close_requested){
        switch (pGame->state)
        {
            case ONGOING:
            if(timerID == 0) {
               timerID = SDL_AddTimer(1000, decreaseMatchTime, &(pGame->matchTime));
            }
                currentTick = SDL_GetTicks();
                deltaTime = (currentTick - lastTick) / 1000.0f;
                lastTick = currentTick;

                sendGameData(pGame);

                while(SDLNet_UDP_Recv(pGame->pSocket,pGame->pPacket)==1){
                    memcpy(&cData, pGame->pPacket->data, sizeof(ClientData));
                    executeCommand(pGame,cData);
                }

                if(SDL_PollEvent(&event)) {
                    if(event.type==SDL_QUIT) {
                        close_requested = 1;
                    }
                }
                for(int i=0;i<MAX_PLAYERS;i++)
                {
                    updatePlayerPosition(pGame->pPlayer[i], deltaTime);
                    restrictPlayerWithinWindow(pGame->pPlayer[i], WINDOW_WIDTH, WINDOW_HEIGHT);
                }
                for (int i = 0; i < pGame->nrOfPlayers - 1; i++) {
                    for (int j = i + 1; j < pGame->nrOfPlayers; j++) {
                        handlePlayerCollision(pGame->pPlayer[i], pGame->pPlayer[j]);
                        freezeEnemyPlayer(pGame->pPlayer[i], pGame->pPlayer[j]);
                    }
                }
                for (int i=0; i<pGame->nrOfPlayers; i++) {
                    SDL_Rect playerRect = getPlayerRect(pGame->pPlayer[i]);
                    SDL_Rect ballRect = getBallRect(pGame->pBall);
                    handlePlayerBallCollision(playerRect, ballRect, pGame->pBall);
                    if(checkCollision(playerRect, getPowerRect(pGame->pPowerUpBox))) {
                        updatePowerCube(pGame->pPowerUpBox, pGame->pRenderer, playerRect);
                        assignPowerUp(pGame->sData.powerUpValue, pGame->pPlayer[i]);
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
                applyFriction(pGame->pBall);
                renderGame(pGame);
                
                break;
            case GAME_OVER:
                drawText(pGame->pOverText);
                sendGameData(pGame);
                if(pGame->nrOfClients==MAX_PLAYERS) pGame->nrOfClients = 0;
            case START:
                renderLobby(pGame);
                SDL_RenderPresent(pGame->pRenderer);
                if(SDL_PollEvent(&event) && event.type==SDL_QUIT) close_requested = 1;
                if(SDLNet_UDP_Recv(pGame->pSocket,pGame->pPacket)==1){
                    add(pGame->pPacket->address,pGame->clients,&(pGame->nrOfClients), pGame->connected);
                    if(pGame->nrOfClients==MAX_PLAYERS) setUpGame(pGame);
                }
                sendGameData(pGame);
                //printf("nrOfClients: %d\n", pGame->nrOfClients);
                break;
        }
        //SDL_Delay(1000/60-15);//might work when you run on different processors
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

    snprintf(lobbyString, sizeof(lobbyString), "Lobby");
    

    if(pGame->connected[0] == true) {
        snprintf(hostSpotString, sizeof(hostSpotString), "Connection established");
        snprintf(spot1String, sizeof(spot1String), "Player 1 is connected");
    } else {
        snprintf(hostSpotString, sizeof(hostSpotString), "Waiting for players...");
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

    pGame->pLobbyText = createText(pGame->pRenderer, 227, 220, 198, pGame->pLobbyFont, lobbyString, WINDOW_WIDTH/2, 100);
    pGame->pHostSpotText = createText(pGame->pRenderer, 227, 220, 198, pGame->pLobbyFont, hostSpotString, WINDOW_WIDTH/2, 200);
    pGame->pSpot1Text = createText(pGame->pRenderer, 250, 220, 198, pGame->pLobbyFont, spot1String, WINDOW_WIDTH/2, 300);
    pGame->pSpot2Text = createText(pGame->pRenderer, 227, 220, 198, pGame->pLobbyFont, spot2String, WINDOW_WIDTH/2, 350);
    pGame->pSpot3Text = createText(pGame->pRenderer, 227, 220, 198, pGame->pLobbyFont, spot3String, WINDOW_WIDTH/2, 400);
    pGame->pSpot4Text = createText(pGame->pRenderer, 227, 220, 198, pGame->pLobbyFont, spot4String, WINDOW_WIDTH/2, 450);

    drawText(pGame->pLobbyText);
    drawText(pGame->pHostSpotText);
    drawText(pGame->pSpot1Text);
    drawText(pGame->pSpot2Text);
    drawText(pGame->pSpot3Text);
    drawText(pGame->pSpot4Text);
    destroyText(pGame->pHostSpotText);
    destroyText(pGame->pSpot1Text);
    destroyText(pGame->pSpot2Text);
    destroyText(pGame->pSpot3Text);
    destroyText(pGame->pSpot4Text);
    destroyText(pGame->pLobbyText);
    
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

void setUpGame(Game *pGame){
    for(int i=0;i<MAX_PLAYERS;i++) setStartingPosition(pGame->pPlayer[i], i, WINDOW_WIDTH, WINDOW_HEIGHT);
    pGame->nrOfPlayers=MAX_PLAYERS;
    pGame->state = ONGOING;
}

void sendGameData(Game *pGame){
    pGame->sData.gState = pGame->state;
    pGame->sData.powerUpValue = rand()%NR_OF_POWERUPS;
    for(int i=0;i<MAX_PLAYERS;i++){
        getPlayerSendData(pGame->pPlayer[i], &(pGame->sData.players[i]));
    }
    for (int i = 0; i < pGame->nrOfPlayers; i++)
    {
        pGame->sData.connected[i] = pGame->connected[i];
    }
    getBallSendData(pGame->pBall, &(pGame->sData.ball));
    for(int i=0;i<MAX_PLAYERS;i++){
        pGame->sData.clientNr = i;
        memcpy(pGame->pPacket->data, &(pGame->sData), sizeof(ServerData));
		pGame->pPacket->len = sizeof(ServerData);
        pGame->pPacket->address = pGame->clients[i];
		SDLNet_UDP_Send(pGame->pSocket,-1,pGame->pPacket);
    }
}

void add(IPaddress address, IPaddress clients[], int *pNrOfClients, bool connected[]){
    for(int i=0; i<*pNrOfClients; i++) {
        if(address.host==clients[i].host && address.port==clients[i].port) {
            return;
        }
    }
    clients[*pNrOfClients] = address;
    connected[*pNrOfClients] = true;
    (*pNrOfClients)++;
}

void executeCommand(Game *pGame,ClientData cData){
    switch (cData.command)
    {
        case UP:
            updatePlayerVUp(pGame->pPlayer[cData.clientNumber]);
            break;
        case DOWN:
            updatePlayerVDown(pGame->pPlayer[cData.clientNumber]);
            break;
        case LEFT:
            updatePlayerVLeft(pGame->pPlayer[cData.clientNumber]);
            break;
        case RIGHT:
            updatePlayerVRight(pGame->pPlayer[cData.clientNumber]);
            break;
        case RESET_Y_VEL:
            resetPlayerSpeed(pGame->pPlayer[cData.clientNumber], 0, 1);
            break;
        case RESET_X_VEL:
            resetPlayerSpeed(pGame->pPlayer[cData.clientNumber], 1, 0);
            break;
        case RESTRICT_PLAYER:
            restrictPlayerWithinWindow(pGame->pPlayer[cData.clientNumber], WINDOW_WIDTH, WINDOW_HEIGHT);
            break;
    }
}

Uint32 decreaseMatchTime(Uint32 interval, void *param) {
    Uint32 *pMatchTime = (Uint32 *)param;
    if (*pMatchTime > 0) {
        *pMatchTime -= 1000;
    }
    return interval;
}

void closeGame(Game *pGame){
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (pGame->pPlayer[i]) {
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
    if(pGame->pOverText) destroyText(pGame->pOverText); 
    if(pGame->pMatchTimerText) destroyText(pGame->pMatchTimerText);
    if(pGame->pGoalsTextTeamA) destroyText(pGame->pGoalsTextTeamA);
    if(pGame->pGoalsTextTeamB) destroyText(pGame->pGoalsTextTeamB);
    if(pGame->pFont) TTF_CloseFont(pGame->pFont);
    if(pGame->pScoreboardFont) TTF_CloseFont(pGame->pScoreboardFont);

    SDLNet_Quit();
    TTF_Quit();
    SDL_Quit();
}