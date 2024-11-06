#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


int main(void)
{

    // Screen dimension constants
    const int SCREEN_WIDTH = 200;
    const int SCREEN_HEIGHT = 400;

    //used for creating window
    SDL_Window      *window = NULL;
    SDL_Renderer    *renderer = NULL;
    
    SDL_Init(SDL_INIT_VIDEO); //This is just for window creation. you can add audio and joystick


    // Create the window
    window = SDL_CreateWindow("SDL Calculator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // Create the renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    
    
    bool quit = false; //used for event loop

    SDL_Event eventHandler; //cool event handler

    //While application is running
    while( !quit ){
        //Handle events on queue
        while( SDL_PollEvent( &eventHandler) != 0 ){ // poll for event
                    
            //User requests quit
            if( eventHandler.type == SDL_QUIT ){ // unless player manually quits
    
                quit = true;
            }

            if (eventHandler.type == SDL_KEYDOWN) {
                      
                if (eventHandler.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true; // Exit on pressing the escape key
                }

            }       
                  
        }
    

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //color
        SDL_RenderClear(renderer); //clears the whole screen with it
        SDL_RenderPresent(renderer); //updates the screen
    
    
    }


    //This stuff will destroy and end the program appropriatly
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return (0);
}
