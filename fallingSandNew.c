#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// Screen dimension constants
#define SCREEN_WIDTH 1392
#define SCREEN_HEIGHT 744
#define PIXEL_SIZE 2
#define GRAVITY 0.1f


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
    int lifetime;      // How long this pixel has existed
    int temperature;   // Temperature of the pixel (useful for fire)
    bool exists;
    float velocity;
    SDL_Color color;   // Pixel color for rendering
} Pixel;


Pixel GRID[SCREEN_WIDTH][SCREEN_HEIGHT]; 

Pixel emptyPixel = {EMPTY, 0, 0, false, 0, {0, 0, 0, 255}};





// Function declarations for initialization, media loading, cleanup, and texture operations
bool init(); // Initializes SDL, window, and renderer
bool loadMedia(); // Loads media (e.g., font and text)
void close(); // Frees resources and shuts down SDL
bool loadFromFile(LTexture* lTexture, const char* path); // Loads image from file into texture
bool loadFromRenderedText(LTexture* lTexture, const char* textureText, SDL_Color textColor); // Renders text as texture
void freeTexture(LTexture* lTexture); // Frees texture memory
void renderTexture(LTexture* lTexture, int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip); // Renders texture to screen
int getTextureWidth(LTexture* lTexture); // Returns texture width
int getTextureHeight(LTexture* lTexture); // Returns texture height




// Global variables for the SDL window, renderer, font, and text texture
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;
LTexture modeTextTexture; // Texture to display text
LTexture SizeOfDropperTexture; // Texture to display text



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


// turns the pixels around the mouse to the substance
void instantiateSubstance(int x, int y) { 
    int spawn_range = 2;
    for (int dx = -spawn_range; dx <= spawn_range; dx++) {
        for (int dy = -spawn_range; dy <= spawn_range; dy++) {
            // Improved boundary check

            int pixelBlockX = (x / PIXEL_SIZE) + dx;
            int pixelBlockY = (y / PIXEL_SIZE) + dy;
            

            if (pixelBlockX  >= 0 && pixelBlockX < GRID_WIDTH && pixelBlockY >= 0 && pixelBlockY < GRID_HEIGHT && rand() % 100 < 75){
                // Only add if not already exists
                if (!GRID[pixelBlockX][pixelBlockY].exists) {
                    
                    GRID[pixelBlockX][pixelBlockY].exists = true;
                    GRID[pixelBlockX][pixelBlockY].velocity = 0;
                    
                    GRID[pixelBlockX][pixelBlockY].color.r = 200 + rand() % 55;
                    GRID[pixelBlockX][pixelBlockY].color.g = 150 + rand() % 55;
                    GRID[pixelBlockX][pixelBlockY].color.b = 100 + rand() % 55;
                    GRID[pixelBlockX][pixelBlockY].color.a = 255;
                }
            }
        }
    }
}


void setPixel(int x, int y) {
    SDL_SetRenderDrawColor(gRenderer, GRID[x][y].color.r, GRID[x][y].color.g, GRID[x][y].color.b, 255);
    SDL_Rect rectToBeRendered = {x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE}; 
    SDL_RenderFillRect(gRenderer, &rectToBeRendered);
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




//This is where the magic happens...
// Main function - sets up SDL, loads media, runs main loop, and cleans up
int main(int argc, char* args[]) {
    if (!init()) { // Initialize SDL and create window
        printf("Failed to initialize!\n");
    } else {
        if (!loadMedia()) { // Load media (text textures, fonts)
            printf("Failed to load media!\n");
        } else {
            int quit = 0; // Main loop flag
            SDL_Event event; // Event handler

            bool pressed = false;
            
            while (!quit) {
                while (SDL_PollEvent(&event) != 0) { // Handle events

                    //place controls below...
                    if (event.type == SDL_QUIT) quit = 1; // User requests quit
                    if (event.type == SDL_KEYDOWN){
                        if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1; // Exit on pressing the escape key
                        
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
                        // Get the mouse position regardless of motion
                        int mouseX, mouseY;
                        SDL_GetMouseState(&mouseX, &mouseY);
                        instantiateSubstance(mouseX, mouseY);
                    }

                }

                render(); 


                for (int y = 0; y < GRID_HEIGHT; y++){
                    for (int x = 0; x < GRID_WIDTH; x++){
                        
                        if (GRID[x][y].exists){
                        
                            setPixel(x, y);
                        }                         
                    }
                } 

                // //this is for text
                // renderTexture(&modeTextTexture, 0,0, NULL, 0, NULL, SDL_FLIP_NONE); 
                // renderTexture(&SizeOfDropperTexture, 200,0, NULL, 0, NULL, SDL_FLIP_NONE); 
                // SDL_RenderPresent(gRenderer); // Update screen


            }
        }
    }
    close(); // Free resources and close SDL

    return 0;
} 
