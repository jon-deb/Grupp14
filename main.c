#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>




int main(int argv, char** args) {

    if(SDL_Init(SDL_INIT_VIDEO)!= 0) {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }
}

