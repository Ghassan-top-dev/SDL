#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define FOV 360           // Field of View for the raycaster
#define WIDTH 800         // Screen width
#define HEIGHT 800        // Screen height
#define GRID_SIZE 8       // Grid dimensions
#define CELL_SIZE 100     // Size of each grid cell
#define PLAYER_RADIUS 25  // Radius of the player representation
#define PLAYER_SPEED 2    // Movement speed of the player
#define ROTATION_SPEED 0.01 // Rotation speed of the player

// Function to render walls based on the grid
void makeWall(int array[GRID_SIZE][GRID_SIZE], SDL_Renderer *renderer) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (array[i][j] == 1) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black for walls
                SDL_Rect rect = {j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

// Function to render the player and handle movement/rotation
void player(SDL_Renderer *renderer, bool forward, bool backward, int *cx, int *cy, double *angle, bool rotateRight, bool rotateLeft) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White for the player

    // Normalize the angle to keep it within 0 to 2*PI
    *angle = fmod(*angle, 2 * M_PI);
    if (*angle < 0) *angle += 2 * M_PI;

    // Movement handling
    if (forward) {
        *cx += PLAYER_SPEED * cos(*angle);
        *cy += PLAYER_SPEED * sin(*angle);
    }
    if (backward) {
        *cx -= PLAYER_SPEED * cos(*angle);
        *cy -= PLAYER_SPEED * sin(*angle);
    }

    // Rotation handling
    if (rotateRight) *angle += ROTATION_SPEED;
    if (rotateLeft) *angle -= ROTATION_SPEED;

    // Render player as a small rectangle
    SDL_Rect playerRect = {*cx - PLAYER_RADIUS, *cy - PLAYER_RADIUS, PLAYER_RADIUS * 2, PLAYER_RADIUS * 2};
    SDL_RenderFillRect(renderer, &playerRect);
}

// Function to cast rays and detect walls
void castRay(SDL_Renderer *renderer, int cx, int cy, double angle, int array[GRID_SIZE][GRID_SIZE]) {
    double rayAngle;
    double rayStep = (FOV * (M_PI / 180)) / 100; // Divide FOV into 400 sections

    for (int i = 0; i <= 400; i++) {
        rayAngle = angle - (FOV * M_PI / 360) + (i * rayStep);

        for (int length = 0; length < WIDTH; length++) {
            // Calculate ray position
            int xRay = cx + length * cos(rayAngle);
            int yRay = cy + length * sin(rayAngle);
            int gridX = xRay / CELL_SIZE;
            int gridY = yRay / CELL_SIZE;

            // Check for wall collision
            if (gridX >= 0 && gridX < GRID_SIZE && gridY >= 0 && gridY < GRID_SIZE && array[gridY][gridX] == 1) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow for rays
                SDL_RenderDrawLine(renderer, cx, cy, xRay, yRay);
                break;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window *window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Grid map: 1 represents walls, 0 represents empty spaces
    int array[GRID_SIZE][GRID_SIZE] = {
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1}
    };

    // Player state
    int cx = WIDTH / 2, cy = HEIGHT / 4;
    double angle = M_PI / 2; // Initial angle facing downwards
    bool forward = false, backward = false, rotateRight = false, rotateLeft = false;

    // Main loop
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_w) forward = true;
                    if (event.key.keysym.sym == SDLK_s) backward = true;
                    if (event.key.keysym.sym == SDLK_d) rotateRight = true;
                    if (event.key.keysym.sym == SDLK_a) rotateLeft = true;
                    break;
                case SDL_KEYUP:
                    if (event.key.keysym.sym == SDLK_w) forward = false;
                    if (event.key.keysym.sym == SDLK_s) backward = false;
                    if (event.key.keysym.sym == SDLK_d) rotateRight = false;
                    if (event.key.keysym.sym == SDLK_a) rotateLeft = false;
                    break;
            }
        }

        // Render scene
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Gray background
        SDL_RenderClear(renderer);

        // makeWall(array, renderer);
        player(renderer, forward, backward, &cx, &cy, &angle, rotateRight, rotateLeft);
        castRay(renderer, cx, cy, angle, array);

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
