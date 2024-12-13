#include <SDL2/SDL.h> 
#include <stdio.h>

int main() { 
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Mouse Events Example with Renderer",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          640, 480, SDL_WINDOW_SHOWN);

    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Event event;
    int running = 1;
    int mouseX, mouseY;
    int mouseColorR = 0, mouseColorG = 0, mouseColorB = 0;

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mouseColorR = 200; // Change color to red on left-click
                        mouseColorG = 200;
                        mouseColorB = 200;
                        printf("Left button pressed at (%d, %d)\n", event.button.x, event.button.y);
                    } else if (event.button.button == SDL_BUTTON_RIGHT) {
                        mouseColorB = 255; // Change color to blue on right-click
                        printf("Right button pressed at (%d, %d)\n", event.button.x, event.button.y);
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    mouseColorR = mouseColorG = mouseColorB = 0; // Reset color on release
                    printf("Mouse button released at (%d, %d)\n", event.button.x, event.button.y);
                    break;
                case SDL_MOUSEMOTION:
                    mouseX = event.motion.x;
                    mouseY = event.motion.y;
                    printf("Mouse moved to (%d, %d)\n", mouseX, mouseY);
                    break;
                case SDL_MOUSEWHEEL:
                    printf("Mouse wheel scrolled %s by %d units\n",
                           event.wheel.y > 0 ? "up" : "down", event.wheel.y);
                    break;
            }
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 225, 10, 10, 255); // Clear with red
        SDL_RenderClear(renderer);

        // Draw a rectangle at the mouse position with color based on the mouse button state
        SDL_SetRenderDrawColor(renderer, mouseColorR, mouseColorG, mouseColorB, 255);
        SDL_Rect mouseRect = {mouseX - 10, mouseY - 10, 20, 20};
        SDL_RenderFillRect(renderer, &mouseRect);

        SDL_RenderPresent(renderer); // Present the updated renderer
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
