#include <SDL2/SDL.h>
#include <stdio.h>

int isWithinBoundaries(SDL_Rect rect, int mouseX, int mouseY); //check if mouse is within boundaries
int whereIsMyMouse(SDL_Rect rect, int mouseX, int mouseY); //check if mouse is within boundaries


int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Button Click Example",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          640, 480, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Rect buttonRect = {200, 150, 100, 50}; // Button positioned at (200, 150) with width 100 and height 50

    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        // Check if mouse click is within button boundaries
                        if (isWithinBoundaries(buttonRect, event.button.x, event.button.y)){//make sure its in the y {//make sure its in the y
                            printf("Button clicked!\n");
                            // Trigger button action here
                        }
                    }
                    break;
            }
        
        }
        // Get mouse position
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        // Check if the mouse is hovering over the button
        int isHovered = whereIsMyMouse(buttonRect, mouseX, mouseY);


        // Render button
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Clear screen with black
        SDL_RenderClear(renderer);

        if (isHovered) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Hover color (green)
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Normal color (red)
        }

        // Set button color and render it
        SDL_RenderFillRect(renderer, &buttonRect);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

int isWithinBoundaries(SDL_Rect rect, int mouseX, int mouseY){ //check if mouse is within boundaries (used for pressing)

    if (mouseX >= rect.x && mouseX <= rect.x + rect.w && mouseY >= rect.y && mouseY <= rect.y + rect.h) return 1; //make sure its in the y and x                                          
    
    else return 0;
    
    return -1;

    //EX: have this in event handler loop
    // if (isWithinBoundaries(buttonRect, event.button.x, event.button.y)){//make sure its in the y {//make sure its in the y
    //                         printf("Button clicked!\n");
    //                         // Trigger button action here
    //  }
}

int whereIsMyMouse(SDL_Rect rect, int mouseX, int mouseY){ //check if mouse is within boundaries (used for getting position)

    return (mouseX >= rect.x && mouseX <= rect.x + rect.w && mouseY >= rect.y && mouseY <= rect.y + rect.h); 

    // EX: have this outside event handler loop in while loop
    // // Get mouse position
    //     int mouseX, mouseY;
    //     SDL_GetMouseState(&mouseX, &mouseY);

    //     // Check if the mouse is hovering over the button
    //     int isHovered = whereIsMyMouse(buttonRect, mouseX, mouseY);


    //     // Render button
    //     SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Clear screen with black
    //     SDL_RenderClear(renderer);

    //     if (isHovered) {
    //         SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Hover color (green)
    //     } else {
    //         SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Normal color (red)
    //     }

}
