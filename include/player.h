#ifndef PLAYER_H
#define PLAYER_H

typedef struct player Player;
#define MOVEMENT_SPEED 400

Player *createPlayer(SDL_Renderer *pGameRenderer,  int w, int h);
void setPlayerX(Player *player);
void setPlayerY(Player *player);
void updatePlayerVUp(Player *player);
void updatePlayerVDown(Player *player);
void updatePlayerVLeft(Player *player);
void updatePlayerVRight(Player *player);
void resetPlayerSpeed(Player *player, int x, int y);
int getPlayerSpeedY(Player *player);
int getPlayerSpeedX(Player *player);
SDL_Texture *getPlayerTexture(Player *player);
SDL_Rect getPlayerRect(Player *player);

#endif