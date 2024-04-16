#ifndef PLAYER_H
#define PLAYER_H

typedef struct player Player;

Player *createPlayer(SDL_Renderer *pGameRenderer, int w, int h);
void setPlayerPosition(Player *player, int x, int y);
void updatePlayerVelocity(Player *player, float vx, float vy);
SDL_Texture *getPlayerTexture(Player *player);
SDL_Rect getPlayerRect(Player *player);
void updatePlayerVUp(Player *player);
void updatePlayerVDown(Player *player);
void updatePlayerVLeft(Player *player);
void updatePlayerVRight(Player *player);
void updatePlayerPosition(Player *pPlayer, float deltaTime);
void resetPlayerSpeed(Player *player, int x, int y);
int getPlayerSpeedY(Player *player);
int getPlayerSpeedX(Player *player);
void restrictPlayerWithinWindow(Player *pPlayer, int w, int h);

#endif
