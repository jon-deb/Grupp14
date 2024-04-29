#ifndef state_h
#define state_h

#include <SDL.h>
#include <stdbool.h>

enum gameState {
    MENU = 0,           
    PLAYING = 1,        //tills tiden är ute
    GAMEOVER = 2,       //return to menu
    LOCALPLAY = 3,      //spela solo för testning
}; typedef enum gameState GameState;

#endif