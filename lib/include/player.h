#ifndef player_h
#define player_h

typedef struct player Player;

Player *createPlayer(SDL_Renderer *pGameRenderer, int w, int h, int playerIndex);
void setPlayerPosition(Player *pPlayer, int x, int y);
void destroyPlayer(Player *pPlayer);
void updatePlayerVelocity(Player *pPlayer, float vx, float vy);
SDL_Texture *getPlayerTexture(Player *pPlayer);
SDL_Rect getPlayerRect(Player *pPlayer);
void updatePlayerVUp(Player *pPlayer);
void updatePlayerVDown(Player *pPlayer);
void updatePlayerVLeft(Player *pPlayer);
void updatePlayerVRight(Player *pPlayer);
void updatePlayerPosition(Player *pPlayer, float deltaTime);
void resetPlayerSpeed(Player *pPlayer, int x, int y);
int getPlayerSpeedY(Player *pPlayer);
int getPlayerSpeedX(Player *pPlayer);
void restrictPlayerWithinWindow(Player *pPlayer, int w, int h);
void resetPlayerPos(Player *pPlayer, int playerIndex, int w, int h);
//void getPlayerSendData(Player *pPlayer, PlayerData *pPlayerData);
//void updatePlayerWithRecievedData(Player *pPlayer, PlayerData *pPlayerData);

#endif
