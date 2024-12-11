#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GRID_WIDTH 200
#define GRID_HEIGHT 450
#define CELL_SIZE 4
#define GRAVITY 0.1f

// Particle structure
typedef struct {
    bool exists;     // Does the particle exist?
    float velocity;  // Vertical velocity
    SDL_Color color; // Particle color
} Particle;

// Global grid and SDL components
Particle grid[GRID_WIDTH][GRID_HEIGHT] = {0};
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// Initialize SDL
bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("Falling Sand Simulator", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        SCREEN_WIDTH, SCREEN_HEIGHT, 
        SDL_WINDOW_SHOWN);

    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

// Add sand at mouse position
void add_sand(int x, int y) {
    // Create a 5x5 area of sand
    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            int grid_x = (x / CELL_SIZE) + dx;
            int grid_y = (y / CELL_SIZE) + dy;

            // Boundary and randomness check
            if (grid_x >= 0 && grid_x < GRID_WIDTH && 
                grid_y >= 0 && grid_y < GRID_HEIGHT &&
                rand() % 100 < 75) {
                
                grid[grid_x][grid_y].exists = true;
                grid[grid_x][grid_y].velocity = 0;
                
                // Random sand-like colors
                grid[grid_x][grid_y].color.r = 200 + rand() % 55;
                grid[grid_x][grid_y].color.g = 150 + rand() % 55;
                grid[grid_x][grid_y].color.b = 100 + rand() % 55;
                grid[grid_x][grid_y].color.a = 255;
            }
        }
    }
}

// Physics update for sand
void update_sand() {
    // Temporary grid for next frame
    Particle next_grid[GRID_WIDTH][GRID_HEIGHT] = {0};

    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            // Skip if no particle here
            if (!grid[x][y].exists) continue;

            // Calculate new velocity and position
            float new_velocity = grid[x][y].velocity + GRAVITY;
            int new_y = y + (int)new_velocity;

            // Boundary check
            if (new_y >= GRID_HEIGHT) continue;

            // Check directly below
            if (!grid[x][new_y].exists) {
                next_grid[x][new_y] = grid[x][y];
                next_grid[x][new_y].velocity = new_velocity;
                continue;
            }

            // Try diagonal movement
            int dir = (rand() % 2 == 0) ? 1 : -1;
            int new_x = x + dir;

            // Check diagonal movement
            if (new_x >= 0 && new_x < GRID_WIDTH && 
                new_y >= 0 && new_y < GRID_HEIGHT &&
                !grid[new_x][new_y].exists) {
                next_grid[new_x][new_y] = grid[x][y];
                next_grid[new_x][new_y].velocity = new_velocity;
                continue;
            }

            // If can't move, stay in place
            next_grid[x][y] = grid[x][y];
            next_grid[x][y].velocity += GRAVITY;
        }
    }

    // Copy back to main grid
    memcpy(grid, next_grid, sizeof(next_grid));
}

// Render the grid
void render() {
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render particles
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            if (grid[x][y].exists) {
                SDL_Rect particle_rect = {
                    x * CELL_SIZE, 
                    y * CELL_SIZE, 
                    CELL_SIZE, 
                    CELL_SIZE
                };

                SDL_SetRenderDrawColor(renderer, 
                    grid[x][y].color.r, 
                    grid[x][y].color.g, 
                    grid[x][y].color.b, 
                    grid[x][y].color.a);
                SDL_RenderFillRect(renderer, &particle_rect);
            }
        }
    }

    // Update screen
    SDL_RenderPresent(renderer);
}

int main(int argc, char* args[]) {
    // Seed random number generator
    srand(time(NULL));

    // Initialize SDL
    if (!init()) {
        return 1;
    }

    // Main game loop
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            
            // Add sand on mouse press
            if (e.type == SDL_MOUSEBUTTONDOWN || 
                (e.type == SDL_MOUSEMOTION && 
                 e.button.state == SDL_BUTTON_LMASK)) {
                int mouse_x, mouse_y;
                SDL_GetMouseState(&mouse_x, &mouse_y);
                add_sand(mouse_x, mouse_y);
            }
        }

        // Update sand physics
        update_sand();

        // Render
        render();

        // Control frame rate
        SDL_Delay(16);  // ~60 FPS
    }

    // Clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
