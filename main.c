#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>




int main(int argv, char** args) {

    if(SDL_Init(SDL_INIT_VIDEO)!= 0) {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* pWindow = SDL_CreateWindow("Enkelt exempel 1",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WINDOW_WIDTH,WINDOW_HEIGHT,0);
    if(!pWindow){
        printf("Error: %s\n",SDL_GetError());
        SDL_Quit();
        return 1;
    }
}

