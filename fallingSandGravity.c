// gcc -I src/include -L src/lib -o main new2.c -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer

// claude code
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>


// Screen dimension constants
#define SCREEN_WIDTH 1392
#define SCREEN_HEIGHT 744
#define PIXEL_SIZE 2
#define GRAVITY 0.5f
#define MAX_VELOCITY 10.0f

#define GRID_HEIGHT (SCREEN_HEIGHT / PIXEL_SIZE)
#define GRID_WIDTH (SCREEN_WIDTH / PIXEL_SIZE)

// Texture wrapper structure to hold texture data and dimensions
typedef struct {
    SDL_Texture* texture;
    int width;
    int height;
} LTexture;

SDL_Color textColor = {255, 255, 255}; // text color

//Code for the sandbox
typedef enum {
    EMPTY = 0,
    SAND = 1,
    WATER = 2
} PixelType;

typedef struct {
    PixelType type;          // Pixel type, e.g., EMPTY, SAND
    bool exists;
    float velocity;
    bool updated;
    SDL_Color color;   // Pixel color for rendering
} Pixel;

Pixel GRID[SCREEN_WIDTH][SCREEN_HEIGHT]; 

Pixel emptyPixel = {EMPTY, false, 0, false, {0, 0, 0, 255}};
Pixel sandPixel = {SAND, true, 0, false, {255, 100, 100, 255}};

// Function declarations (same as before)
bool init();
bool loadMedia();
void close();
bool loadFromFile(LTexture* lTexture, const char* path);
bool loadFromRenderedText(LTexture* lTexture, const char* textureText, SDL_Color textColor);
void freeTexture(LTexture* lTexture);
void renderTexture(LTexture* lTexture, int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip);
int getTextureWidth(LTexture* lTexture);
int getTextureHeight(LTexture* lTexture);

// Global variables for the SDL window, renderer, font, and text texture
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;
LTexture modeTextTexture;
LTexture SizeOfDropperTexture;

// Initializes SDL, creates window and renderer, sets up image and text libraries
bool init() {
    bool success = true; // Success flag for function

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    } else {
        // Set linear texture filtering
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
            printf("Warning: Linear texture filtering not enabled!\n");
        }

        // Create SDL window
        gWindow = SDL_CreateWindow("Sandbox", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL) {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        } else {
            // Create vsynced renderer for window
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (gRenderer == NULL) {
                printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            } else {
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF); // White background

                // Initialize SDL_image with PNG support
                int imgFlags = IMG_INIT_PNG;
                if (!(IMG_Init(imgFlags) & imgFlags)) {
                    printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
                    success = false;
                }

                // Initialize SDL_ttf for text rendering
                if (TTF_Init() == -1) {
                    printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
                    success = false;
                }
            }
        }
    }
    return success;
}

// Loads the necessary media resources such as fonts and text textures
bool loadMedia() {
    bool success = true;

    // Open the font file at size 28
    gFont = TTF_OpenFont("bit5x3.ttf", 15); //font size
    if (gFont == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        success = false;
    } else {

        // Render the text to create a texture
        if (!loadFromRenderedText(&modeTextTexture, " ", textColor)) {
            printf("Failed to render text texture!\n");
            success = false;
        }

        // Render the text to create a texture
        if (!loadFromRenderedText(&SizeOfDropperTexture, " ", textColor)) {
            printf("Failed to render text texture!\n");
            success = false;
        }
    }
    return success;
}

// Frees up resources and shuts down SDL libraries
void close() {
    freeTexture(&modeTextTexture); // Free text texture
    freeTexture(&SizeOfDropperTexture); 

    TTF_CloseFont(gFont); // Close font
    gFont = NULL;

    SDL_DestroyRenderer(gRenderer); // Destroy renderer
    SDL_DestroyWindow(gWindow); // Destroy window
    gWindow = NULL;
    gRenderer = NULL;

    // Quit SDL subsystems
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

// Loads an image from file, converts it to a texture, and sets the texture's width/height
bool loadFromFile(LTexture* lTexture, const char* path) {
    freeTexture(lTexture); // Free existing texture

    SDL_Texture* newTexture = NULL;

    // Load image as surface
    SDL_Surface* loadedSurface = IMG_Load(path);
    if (loadedSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
    } else {
        // Set color key (transparent) for the loaded image
        SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

        // Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == NULL) {
            printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
        } else {
            lTexture->width = loadedSurface->w;
            lTexture->height = loadedSurface->h;
        }

        SDL_FreeSurface(loadedSurface); // Free loaded surface
    }

    lTexture->texture = newTexture;
    return lTexture->texture != NULL;
}

// Renders text as a texture, stores it in the given LTexture struct
bool loadFromRenderedText(LTexture* lTexture, const char* textureText, SDL_Color textColor) {
    freeTexture(lTexture); // Free existing texture

    SDL_Surface* textSurface = TTF_RenderText_Blended(gFont, textureText, textColor);
    if (textSurface == NULL) {
        printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
    } else {
        lTexture->texture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
        if (lTexture->texture == NULL) {
            printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
        } else {
            lTexture->width = textSurface->w;
            lTexture->height = textSurface->h;
        }
        SDL_FreeSurface(textSurface); // Free surface
    }
    return lTexture->texture != NULL;
}

// Frees texture memory if it exists
void freeTexture(LTexture* lTexture) {
    if (lTexture->texture != NULL) {
        SDL_DestroyTexture(lTexture->texture);
        lTexture->texture = NULL;
        lTexture->width = 0;
        lTexture->height = 0;
    }
}

// Renders a texture with optional clipping, rotation, and flipping
void renderTexture(LTexture* lTexture, int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip) {
    SDL_Rect renderQuad = {x, y, lTexture->width, lTexture->height}; // Set rendering space

    if (clip != NULL) {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }

    SDL_RenderCopyEx(gRenderer, lTexture->texture, clip, &renderQuad, angle, center, flip);
}

// Gets the width of a texture
int getTextureWidth(LTexture* lTexture) {
    return lTexture->width;
}

// Gets the height of a texture
int getTextureHeight(LTexture* lTexture) {
    return lTexture->height;
}


void render() {
    // Clear screen
    SDL_SetRenderDrawColor(gRenderer, 110, 110, 110, 255);
    SDL_RenderClear(gRenderer);

    // Render particles
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            if (GRID[x][y].exists) {
                SDL_Rect particle_rect = {
                    x * PIXEL_SIZE, 
                    y * PIXEL_SIZE, 
                    PIXEL_SIZE, 
                    PIXEL_SIZE
                };

                SDL_SetRenderDrawColor(gRenderer, 
                    GRID[x][y].color.r, 
                    GRID[x][y].color.g, 
                    GRID[x][y].color.b, 
                    GRID[x][y].color.a);
                SDL_RenderFillRect(gRenderer, &particle_rect);
            }
        }
    }

    // Update screen
    SDL_RenderPresent(gRenderer);
}


// New function to update sand pixels with physics
void updateSandPhysics() {
    // Reset update flags
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            GRID[x][y].updated = false;
        }
    }

    // Update from bottom to top to simulate gravity
    for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (GRID[x][y].exists && GRID[x][y].type == SAND && !GRID[x][y].updated) {
                // Apply gravity
                GRID[x][y].velocity += GRAVITY;
                
                // Cap maximum velocity
                if (GRID[x][y].velocity > MAX_VELOCITY) {
                    GRID[x][y].velocity = MAX_VELOCITY;
                }

                int newY = y + floor(GRID[x][y].velocity);
                
                // Check if can fall straight down
                if (newY < GRID_HEIGHT && !GRID[x][newY].exists) {
                    // Move pixel down
                    GRID[x][newY] = GRID[x][y];
                    GRID[x][y] = emptyPixel;
                    GRID[x][newY].updated = true;
                }
                // Check diagonal falling (with some randomness)
                else if (newY < GRID_HEIGHT) {
                    int fallDirection = (rand() % 2 == 0) ? -1 : 1;
                    int newX = x + fallDirection;
                    
                    if (newX >= 0 && newX < GRID_WIDTH && y < GRID_HEIGHT && !GRID[newX][y+1].exists) {
                        // Move pixel diagonally
                        GRID[newX][y+1] = GRID[x][y];
                        GRID[x][y] = emptyPixel;
                        GRID[newX][y+1].updated = true;
                    }
                    else {
                        // If can't fall, reset velocity
                        GRID[x][y].velocity = 0;
                    }
                }
            }
        }
    }
}

// Modified instantiate substance to reset velocity
void instantiateSubstance(int x, int y) { 
    int spawn_range = 2;
    for (int dx = -spawn_range; dx <= spawn_range; dx++) {
        for (int dy = -spawn_range; dy <= spawn_range; dy++) {
            int pixelBlockX = (x / PIXEL_SIZE) + dx;
            int pixelBlockY = (y / PIXEL_SIZE) + dy;
            
            if (pixelBlockX >= 0 && pixelBlockX < GRID_WIDTH && pixelBlockY >= 0 && pixelBlockY < GRID_HEIGHT && rand() % 100 < 75) {
                if (!GRID[pixelBlockX][pixelBlockY].exists) {
                    Pixel newSandPixel = sandPixel;
                    newSandPixel.velocity = 0; // Initialize with zero velocity
                    GRID[pixelBlockX][pixelBlockY] = newSandPixel;
                }
            }
        }
    }
}

// Rest of the rendering functions remain the same

// Modified main loop to include physics update
int main(int argc, char* args[]) {
    // Seed random number generator
    srand(time(NULL));

    if (!init()) {
        printf("Failed to initialize!\n");
    } else {
        if (!loadMedia()) {
            printf("Failed to load media!\n");
        } else {
            int quit = 0;
            SDL_Event event;
            bool pressed = false;
            
            while (!quit) {
                while (SDL_PollEvent(&event) != 0) {
                    if (event.type == SDL_QUIT) quit = 1;
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
                    }

                    if (event.type == SDL_MOUSEBUTTONDOWN) {
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            pressed = true;
                        }
                    }

                    if (event.type == SDL_MOUSEBUTTONUP) {
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            pressed = false;
                        }
                    }

                    if (pressed) {
                        int mouseX, mouseY;
                        SDL_GetMouseState(&mouseX, &mouseY);
                        instantiateSubstance(mouseX, mouseY);
                    }
                }

                // Update sand physics
                updateSandPhysics();

                // Render
                render();

                // Optional: Add a small delay to control simulation speed
                SDL_Delay(16);
            }
        }
    }
    close();
    return 0;
}
