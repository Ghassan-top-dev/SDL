#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>



// Screen dimension constants
const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 750;
const int PIXEL_SIZE = 12; 

#define GRID_HEIGHT (SCREEN_HEIGHT / PIXEL_SIZE)
#define GRID_WIDTH (SCREEN_WIDTH / PIXEL_SIZE)

// Texture wrapper structure to hold texture data and dimensions
typedef struct {
    SDL_Texture* texture;
    int width;
    int height;
} LTexture;


//Code for the sandbox
typedef enum {
    EMPTY = 0,
    SAND = 1,
    WATER = 2,
    WOOD = 3,
    FIRE = 4
} PixelType;

typedef struct {
    PixelType type;          // Pixel type, e.g., EMPTY, SAND
    int lifetime;      // How long this pixel has existed
    int temperature;   // Temperature of the pixel (useful for fire)
    bool updatedYet;
    SDL_Color color;   // Pixel color for rendering
} Pixel;


Pixel GRID[SCREEN_WIDTH][SCREEN_HEIGHT]; 


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
// void setPixel(Pixel *cell, int type, x, y); 


// Global variables for the SDL window, renderer, font, and text texture
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;
LTexture gTextTexture; // Texture to display text

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
    gFont = TTF_OpenFont("/Users/ghassanmuradagha/Documents/pro/SDL_GENERAL/fonts/open-sans/OpenSans-Bold.ttf", 15); //font size
    if (gFont == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        success = false;
    } else {
        SDL_Color textColor = {255, 255, 255}; // text color

        // Render the text to create a texture
        if (!loadFromRenderedText(&gTextTexture, "CIRCLE", textColor)) {
            printf("Failed to render text texture!\n");
            success = false;
        }
    }
    return success;
}

// Frees up resources and shuts down SDL libraries
void close() {
    freeTexture(&gTextTexture); // Free text texture

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


void setPixel(Pixel rect, int x, int y) {
    SDL_SetRenderDrawColor(gRenderer, rect.color.r, rect.color.g, rect.color.b, rect.color.a);
    SDL_Rect rectToBeRendered = {x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE}; 
    SDL_RenderFillRect(gRenderer, &rectToBeRendered);
}

// void dropPixels(Pixel rect, int x, int y) {
//     SDL_SetRenderDrawColor(gRenderer, rect.color.r, rect.color.g, rect.color.b, rect.color.a);
//     SDL_Rect rectToBeRendered = {x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE}; 
//     SDL_RenderFillRect(gRenderer, &rectToBeRendered);
// }







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

            Pixel sandPixel = {SAND, 0, 25, false, {194, 178, 128, 255}};
            Pixel emptyPixel = {EMPTY, 0, 0, false, {0, 0, 0, 255}};


            for (int y = 0; y < GRID_HEIGHT; y++) {
                for (int x = 0; x < GRID_WIDTH; x++) {
                    GRID[x][y] = emptyPixel;
                }
            }


            bool upDated = true; 


            
            
            while (!quit) {
                while (SDL_PollEvent(&event) != 0) { // Handle events

                //place controls below...
                    if (event.type == SDL_QUIT) quit = 1; // User requests quit
                    if (event.type == SDL_KEYDOWN){
                        if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1; // Exit on pressing the escape key
                    }
                

                    switch (event.type) {
                        case SDL_MOUSEBUTTONDOWN:
                            if (event.button.button == SDL_BUTTON_LEFT) {

                                int x = event.button.x / PIXEL_SIZE;
                                int y = event.button.y / PIXEL_SIZE;

                                if (GRID[x][y].type == EMPTY){
                                    GRID[x][y].type = sandPixel.type; 
                                    GRID[x][y].color = sandPixel.color; 
                                }

                            }
                            break;
                        case SDL_MOUSEBUTTONUP:
                           
                            break;
                        case SDL_MOUSEMOTION:
                            
                            break;
                    }

                }
                // Clear screen with black background
                SDL_SetRenderDrawColor(gRenderer, 110, 110, 110, 255);
                SDL_RenderClear(gRenderer); 




                if (upDated) {
                    for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
                        for (int x = 0; x < GRID_WIDTH; x++) {
                            // Check if the current cell is not EMPTY
                            if (GRID[x][y].type != EMPTY) {
                                // Ensure we're not at the bottom row
                                if (y + 1< GRID_HEIGHT && GRID[x][y + 1].type == EMPTY) {
                                    // Move the block down
                                    GRID[x][y + 1] = GRID[x][y]; // Copy current cell to the cell below
                                    GRID[x][y] = emptyPixel;    // Set current cell to EMPTY
                                }
                            }
                        }
                    }

                }






                
                for (int y = 0; y < GRID_HEIGHT; y++){
                    for (int x = 0; x < GRID_WIDTH; x++){
                        Pixel pixelRect = GRID[x][y]; 
                        
                        if (GRID[x][y].type != EMPTY ){
                        
                            pixelRect.type = sandPixel.type;
                            setPixel(pixelRect, x, y);
                        }                         
                    }
                } 

                upDated = true; 
                


               

                //this is for text
                renderTexture(&gTextTexture, 0,0, NULL, 0, NULL, SDL_FLIP_NONE); //this is for text (dk, posx, posy, dk, dk, dk,dk); 
                SDL_RenderPresent(gRenderer); // Update screen
            }
        }
    }
    close(); // Free resources and close SDL

    return 0;
}







                // // for (int y = 0; y < SCREEN_HEIGHT; y++){
                // //     for (int x = 0; x < SCREEN_WIDTH; x++){
                // //        Pixel pixelRect = GRID[x][y];  // Get the pixel at this position


                // //         if (GRID[x][y].type != EMPTY && GRID[x][y+1].type == EMPTY) {
                            
                // //             GRID[x][y].type = EMPTY; 
                // //             SDL_SetRenderDrawColor(gRenderer, GRID[x][y].color.r, GRID[x][y].color.g, GRID[x][y].color.b, GRID[x][y].color.a);
                // //             SDL_Rect rectToBeRendered = {x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE}; 
                // //             SDL_RenderFillRect(gRenderer, &rectToBeRendered);
                // //             setPixel(pixelRect, x, y+1); 

                // //         }

                // //     }
                // // }   




                // for (int y = SCREEN_HEIGHT; y >= 0; y--){
                //     for (int x = 0; x < SCREEN_WIDTH; x++){
                //        Pixel pixelRect = GRID[x][y];  // Get the pixel at this position

                //         // If the pixel isn't EMPTY, render it
                //         if (GRID[x][y].type != EMPTY) {
                //             pixelRect.type = sandPixel.type;
                //             setPixel(pixelRect, x, y); 
                //         }
                //     }
                // }


                // for (int y = SCREEN_HEIGHT; y > 0; y--){
                //     for (int x = 0; x < SCREEN_WIDTH; x++){
                //        Pixel pixelRect = GRID[x][y];  // Get the pixel at this position

                //         // If the pixel isn't EMPTY, render it
                //         if (GRID[x][y].type != EMPTY) {

                //             if (GRID[x][y+1].type == EMPTY)
                //             {
                //                 GRID[x][y].type = emptyPixel.type;
                //                 GRID[x][y+1].type = emptyPixel.type;
                //                 setPixel(pixelRect, x, y+1); 
                //             }
                            
                            
                //         }
                //     }
                // }

                // // for (int y = SCREEN_HEIGHT; y < 0; y--){
                // //     for (int x = SCREEN_WIDTH; x < SCREEN_WIDTH; x--){
                // //        Pixel pixelRect = GRID[x][y];  // Get the pixel at this position

                // //         if (GRID[x][y].type != EMPTY && GRID[x][y+1].type == EMPTY) {

                // //             GRID[x][y].type = emptyPixel.type; 
                // //             GRID[x][y+1].type = sandPixel.type; 
                // //             setPixel(pixelRect, x, y); 
                // //             setPixel(pixelRect, x, y+1); 
                           
                // //         }
                // //     }
                // // }

                // // for (int y = 0; y < SCREEN_HEIGHT; y++){
                // //     for (int x = 0; x < SCREEN_WIDTH; x++){
                // //        Pixel pixelRect = GRID[x][y];  // Get the pixel at this position
                // //        pixelRect.updatedYet = false; 
                // //     }
                // // }
                
