#ifndef player_h
#define player_h

typedef struct player Player;
typedef struct playerData PlayerData;

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
void setStartingPosition(Player *pPlayer, int playerIndex, int w, int h);
void resetPlayerSpeed(Player *pPlayer, int x, int y);
int getPlayerSpeedY(Player *pPlayer);
int getPlayerSpeedX(Player *pPlayer);
void restrictPlayerWithinWindow(Player *pPlayer, int w, int h);
void handlePlayerCollision(Player *pPlayer1, Player *pPlayer2);
void getPlayerSendData(Player *pPlayer, PlayerData *pPlayerData);
void updatePlayerWithRecievedData(Player *pPlayer, PlayerData *pPlayerData);
void assignPowerUp(int powerUpValue, Player *pPlayer);
int getCurrentPowerUp(Player *pPlayer);
Uint32 removePowerUp(Uint32 interval, void *param);
void freezeEnemyPlayer(Player *pPlayer1, Player *pPlayer2);

#endif
