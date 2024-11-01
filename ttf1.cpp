/*
to run:

game: 
	g++ main.cpp -o play -I include -I /opt/homebrew/include/SDL2 -L lib -L /opt/homebrew/lib -l SDL2-2.0.0 -l SDL2_ttf




*/

// C++ Standard Libraries
#include <iostream>
// Third Party
#include <SDL2/SDL.h> 
// Include our new font library
#include <SDL2/SDL_ttf.h>



int main(int argc, char* argv[]){
    // Create a window data type
    // This pointer will point to the 
    // window that is allocated from SDL_CreateWindow
    SDL_Window* window=nullptr;
    SDL_Color textColor = {255, 255, 255};  // Define the color separately


    // Initialize the video subsystem.
    // iF it returns less than 1, then an
    // error code will be received.
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cout << "SDL could not be initialized: " <<
                  SDL_GetError();
    }else{
        std::cout << "SDL video system is ready to go\n";
    }
    // Request a window to be created for our platform
    // The parameters are for the title, x and y position,
    // and the width and height of the window.
    window = SDL_CreateWindow("C++ SDL2 Window",20, 20, 640,480,SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = nullptr;
    renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);

    // Just like when we initialize our graphics subsystem,
    // we need to do the same for our font system.
    if(TTF_Init() == -1){
        std::cout << "Could not initailize SDL2_ttf, error: " << TTF_GetError() << std::endl; 
    }else{
        std::cout << "SDL2_ttf system ready to go!" << std::endl;
    }

    // Load our font file and set the font size
    TTF_Font* ourFont = TTF_OpenFont("/Users/ghassanmuradagha/Documents/fonts/open-sans/OpenSans-Bold.ttf",32);
    // Confirm that it was loaded
    if(ourFont == nullptr){
        std::cout << "Could not load font" << std::endl;
        exit(1);
    }
    // Pixels from our text
    SDL_Surface* surfaceText = TTF_RenderText_Solid(ourFont, "Mike SDL2 Series", textColor);

    // Setup the texture
    SDL_Texture* textureText = SDL_CreateTextureFromSurface(renderer,surfaceText);

    // Free the surface
    // We are done with it after we have uploaded to
    // the texture
    SDL_FreeSurface(surfaceText); 

    // Create a rectangle to draw on
    SDL_Rect rectangle;
    rectangle.x = 10;
    rectangle.y = 10;
    rectangle.w = 400;
    rectangle.h = 100;


    // Infinite loop for our application
    bool gameIsRunning = true;
    // Main application loop
    while(gameIsRunning){
        SDL_Event event;

        // (1) Handle Input
        // Start our event loop
        while(SDL_PollEvent(&event)){
            // Handle each specific event
            if(event.type == SDL_QUIT){
                gameIsRunning= false;
            }

        }
        // (2) Handle Updates
        
        // (3) Clear and Draw the Screen
        // Gives us a clear "canvas"
        SDL_SetRenderDrawColor(renderer,0,0,0xFF,SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // Render our text on a rectangle
        SDL_RenderCopy(renderer,textureText,NULL,&rectangle);

        // Finally show what we've drawn
        SDL_RenderPresent(renderer);
    }

    // Destroy our textured text
    SDL_DestroyTexture(textureText);

    // We destroy our window. We are passing in the pointer
    // that points to the memory allocated by the 
    // 'SDL_CreateWindow' function. Remember, this is
    // a 'C-style' API, we don't have destructors.
    SDL_DestroyWindow(window);
    
    // Close our font subsystem
    TTF_CloseFont(ourFont);

    // Quit our program.
    SDL_Quit();
    return 0;
}
