#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2_ttf.h>
#include <SDL2_net.h>
#include "player_data.h"
#include "../include/ball.h"
#include "../include/player.h"
#include "../include/player_data.h"
#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 800
#define MOVEMENT_SPEED 400
#define BALL_SPEED_AFTER_COLLISION 500
#define BORDER_SIZE 20
#define GOAL_TOP 352
#define GOAL_BOTTOM 448
#define MAX_PLAYERS 6

typedef struct game {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    SDL_Texture *backgroundTexture;
    SDL_Surface *pBackgroundSurface;
    int nrOfPlayers;
    Player *pPlayer[MAX_PLAYERS];
    Ball *pBall;
    UDPsocket pSocket;
	UDPpacket *pPacket;
    IPaddress clients[MAX_PLAYERS];
    int nrOfClients;
    ServerData sData;
} Game;

int initiate(Game *pGame);
void run(Game *pGame);
void closeGame(Game *pGame);
void handleInput(Game *pGame, SDL_Event *event);
void add(IPaddress address, IPaddress clients[],int *pNrOfClients);
void sendGameData(Game *pGame);
void executeCommand(Game *pGame,ClientData cData);

//bool checkCollision(SDL_Rect rect1, SDL_Rect rect2);

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
    if (SDLNet_Init()) {
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
        TTF_Quit();
        SDL_Quit();
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
    pGame->pBackgroundSurface = IMG_Load("resources/field.png");
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

    if (!(pGame->pSocket = SDLNet_UDP_Open(2000))) {
		printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		close(pGame);
        return 0;
	}
	if (!(pGame->pPacket = SDLNet_AllocPacket(512))) {
		printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		close(pGame);
        return 0;
	}

    pGame->nrOfPlayers = 2;
    for (int i = 0; i < pGame->nrOfPlayers; i++) {
        pGame->pPlayer[i] = createPlayer(pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, i);
        if (!pGame->pPlayer[i]) {
            fprintf(stderr, "Failed to initialize player %d\n", i + 1);
            close(pGame);
            return 0;
        }
    }
    pGame->pBall = createBall(pGame->pRenderer);
    if (!pGame->pBall) {
        printf("Error initializing the ball.\n");
        closeGame(pGame);
        return 0;
    }

    pGame->state = MENU;
    pGame->nrOfClients = 0;
    return 1;
}

void run(Game *pGame) {
    int close_requested = 0;
    SDL_Event event;
    Uint32 lastTick = SDL_GetTicks();
    Uint32 currentTick;
    float deltaTime;
    ClientData cData;

    while (!close_requested) {
        switch(pGame->state) {
            case MENU: 
                //drawText("press 1 to play multiplayer, q to exit)
                SDL_RenderPresent(pGame->pRenderer);
                
                if(SDL_PollEvent(&event)) {
                    if(event.type == SDL_QUIT || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) close_requested = 1;
                    else if(event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_1) {
                        if(SDLNet_UDP_Recv(pGame->pSocket,pGame->pPacket)==1){
                            add(pGame->pPacket->address, pGame->clients, &(pGame->nrOfClients));
                            if(pGame->nrOfClients==MAX_PLAYERS) setUpGame(pGame);
                        }
                    }
                }
                break;
            case PLAYING:
                sendGameData(pGame);
                currentTick = SDL_GetTicks();
                deltaTime = (currentTick - lastTick) / 1000.0f;
                lastTick = currentTick;
                
                while(SDLNet_UDP_Recv(pGame->pSocket,pGame->pPacket)==1) {
                    memcpy(&cData, pGame->pPacket->data, sizeof(ClientData));
                    executeCommand(pGame,cData);
                }
                while (SDL_PollEvent(&event)) {
                    if(event.type == SDL_QUIT) close_requested = 1;
                    //else handleInput(pGame, &event);
                }
                for (int i = 0; i < pGame->nrOfPlayers; i++) {
                    updatePlayerPosition(pGame->pPlayer[i], deltaTime);
                    restrictPlayerWithinWindow(pGame->pPlayer[i], WINDOW_WIDTH, WINDOW_HEIGHT);
                }
                //handleCollisionsAndPhysics(pGame);
                for (int i=0; i<pGame->nrOfPlayers; i++) {
                    for(int j=0; j<pGame->nrOfPlayers; j++) {
                        handlePlayerCollision(pGame->pPlayer[i], pGame->pPlayer[j]);
                    }
                }
                //om timern går ut case = GAMEOVER:
                // om ett visst antal mål görs case = GAMEOVER:
                renderGame(pGame);
                break;
            case GAMEOVER:
                //drawtext team X won!
                sendGameData(pGame);

                pGame->state = MENU;
                break;
        }
    }
}

void setUpGame(Game *pGame){
    for(int i=0;i<MAX_PLAYERS;i++) setStartingPosition(pGame->pPlayer[i], i, WINDOW_WIDTH, WINDOW_HEIGHT);
    //pGame->nrOfPlayers=PLAYER_MAX;
    pGame->state = PLAYING;
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

/*void handlePlayerCollision(Game *pGame) {
    for (int i=0; i<pGame->nrOfPlayers; i++) {
        for(int j=0; j<pGame->nrOfPlayers; j++) {
            Player *player1 = pGame->pPlayer[i];
            Player *player2 = pGame->pPlayer[j];
            SDL_Rect p1rect = getPlayerRect(pGame->pPlayer[i]);
            SDL_Rect p2rect = getPlayerRect(pGame->pPlayer[j]);
            if(checkCollision(p1rect, p2rect)) {
                updatePosAfterCollision(pGame->pPlayer[i]);
                updatePosAfterCollision(pGame->pPlayer[j]);
            }
        }
    }
}*/

void handleCollisionsAndPhysics(Game *pGame) {
        Player *currentPlayer = pGame->pPlayer[i];
        SDL_Rect playerRect = getPlayerRect(currentPlayer);
        SDL_Rect ballRect = getBallRect(pGame->pBall);
    
        if(checkCollision(playerRect, ballRect)) {
            // räknar mittpunkten för spelare och bollen
            float player1CenterX = playerRect.x + playerRect.w / 2;
            float player1CenterY = playerRect.y + playerRect.h / 2;
            float player2CenterX = ballRect.x + ballRect.w / 2;
            float player2CenterY = ballRect.y + ballRect.h / 2;

            // beräknar vektorn
            float collisionVectorX = player2CenterX - playerCenterX;
            float collisionVectorY = player2CenterY - playerCenterY;
            
            // räknar distansen
            float distance = sqrt(collisionVectorX * collisionVectorX + collisionVectorY * collisionVectorY);

            // normaliserar vektorn
            float normalX = collisionVectorX / distance;
            float normalY = collisionVectorY / distance;

            // update på hastigheten efter collision
            setBallVelocity(pGame->pBall, normalX * BALL_SPEED_AFTER_COLLISION, normalY * BALL_SPEED_AFTER_COLLISION);
        }
    applyFriction(pGame->pBall);
    updateBallPosition(pGame->pBall);
    if (!goal(pGame->pBall)) {
        restrictBallWithinWindow(pGame->pBall);
    }
    else {
        for (int i = 0; i < pGame->nrOfPlayers; i++)
        {
            resetPlayerPos(pGame->pPlayer[i], i, WINDOW_WIDTH, WINDOW_HEIGHT);
        }
    }
}

void handleInput(Game *pGame, SDL_Event *event) {
    switch (event->type) {
        case SDL_KEYDOWN:
            switch (event->key.keysym.scancode) {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    updatePlayerVUp(pGame->pPlayer[0]);
                    cData.command = UP;
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    updatePlayerVDown(pGame->pPlayer[0]);
                    cData.command = DOWN;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    updatePlayerVLeft(pGame->pPlayer[0]);
                    cData.command = LEFT;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    updatePlayerVRight(pGame->pPlayer[0]);
                    cData.command = RIGHT;
                    break;
            }
            break;
        case SDL_KEYUP:
            switch (event->key.keysym.scancode) {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    resetPlayerSpeed(pGame->playerNr 0, 1);
                    cData.command = STOPYVEL;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    resetPlayerSpeed(pGame->playerNr1, 0);
                    cData.command = STOPXVEL;
                    break;
            }
            break;
    }
    restrictPlayerWithinWindow(pGame->pPlayer[0], WINDOW_WIDTH, WINDOW_HEIGHT);
}

void executeCommand(Game *pGame,ClientData cData){
    switch (cData.command)
    {
        case UP:
            updatePlayerVUp(pGame->pPlayer.[cData.playerNumber]);
            break;
        case DOWN:
            updatePlayerVDown(Game->pPlayer.[cData.playerNumber]);
            break;
        case RIGHT:
            updatePlayerVRight(pGame->pPlayer.[cData.playerNumber]);
            break;
        case LEFT:
            updatePlayerVLeft(pGame->pPlayer.[cData.playerNumber]);
            break;
        case STOPYVEL:
            resetPlayerSpeed(pGame->pRocket[cData.playerNumber]);
            break;
        case STOPXVEL:
            resetPlayerSpeed(pGame->pRocket[cData.playerNumber]);
            break;
    }
}

void sendGameData(Game *pGame){
    pGame->sData.gState = pGame->state;
    for(int i=0;i<MAX_PLAYERS;i++){
        getPlayerSendData(pGame->pPlayer[i], &(pGame->sData.player[i]));
    }
    for(int i=0;i<MAX_PLAYERS;i++){
        pGame->sData.playerNr = i;
        memcpy(pGame->pPacket->data, &(pGame->sData), sizeof(ServerData));
		pGame->pPacket->len = sizeof(ServerData);
        pGame->pPacket->address = pGame->clients[i];
		SDLNet_UDP_Send(pGame->pSocket,-1,pGame->pPacket);
    }
}

void add(IPaddress address, IPaddress clients[],int *pNrOfClients){
	for(int i=0;i<*pNrOfClients;i++) if(address.host==clients[i].host &&address.port==clients[i].port) return;
	clients[*pNrOfClients] = address;
	(*pNrOfClients)++;
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

    if(pGame->pPacket) SDLNet_FreePacket(pGame->pPacket);
	if(pGame->pSocket) SDLNet_UDP_Close(pGame->pSocket);

    SDLNet_Quit();
    SDL_Quit();
}





