//I am pausing this project on nov 5, it seems I need structs to work with many balls.
//I'll get back to it soon after some knowledge in CIS-1300 :)
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

int stepperX = 300; //both of these are used for user input movement
int stepperY = 300;
int r = 0, g = 0, b = 0, checker = 0; //used for background 
int shapeSize = 10; //used for the shape of the size. ex. for circle radius


//PHYSICS
float gravity = 1.2; 
float velocityY = 0; //This is a constant? not too sure. - no this is a dynamic velocity
float velocityX = 0; 
float position = 0; //This is the position
float energyLoss = 0.7; //collision damping 

int balls[10]; 


// Texture wrapper structure to hold texture data and dimensions
typedef struct {
    SDL_Texture* texture;
    int width;
    int height;
} LTexture;
 
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
bool moveRight = false, moveLeft = false, moveUp = false, moveDown = false, dash = false; //here add the movements


//circle functions:

void DrawCircle(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius)
{
    const int32_t diameter = (radius * 2);

    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = (tx - diameter);

    while (x >= y)
    {
        // Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

        if (error <= 0)
        {
            ++y;
            error += ty;
            ty += 2;
        }

        if (error > 0)
        {
            --x;
            tx += 2;
            error += (tx - diameter);
        }
    }
}

void DrawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
    // Loop through the y-coordinates of the circle from the top to the bottom
    for (int y = -radius; y <= radius; y++) {
        // Calculate the width of each line at this y level (circle equation)
        int dx = (int)sqrt(radius * radius - y * y); //maybe implement fast inverse square root?

        // Draw a horizontal line for each row from left to right across the diameter
        SDL_RenderDrawLine(renderer, centerX - dx, centerY + y, centerX + dx, centerY + y);
    }
}


//collision function

// void collision(balls){



    






// }

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
        gWindow = SDL_CreateWindow("FLUID SIMULATOR", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
    gFont = TTF_OpenFont("/Users/ghassanmuradagha/Documents/pro/fonts/open-sans/OpenSans-Bold.ttf", 15); //font size
    if (gFont == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        success = false;
    } else {
        SDL_Color textColor = {0, 0, 0}; // text color

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

    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText, textColor);
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
            SDL_Event e; // Event handler

            while (!quit) {
                while (SDL_PollEvent(&e) != 0) { // Handle events

                //place controls below...
                    if (e.type == SDL_QUIT) {
                        quit = 1; // User requests quit
                    }

                    if (e.type == SDL_KEYDOWN) {
                      if (e.key.keysym.sym == SDLK_ESCAPE) {
                          quit = 1; // Exit on pressing the escape key
                      }
                    
                    
                    }
                    
                }
                // Clear screen with black background
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                SDL_RenderClear(gRenderer);     

                gravity+=0.01; //speed up as your going down - doesn't account for terminak velocity yet...
                velocityY += gravity; //velocity will always be dependant on gravity, gravity will always fight or help velocity
                position += velocityY; //position is dependant on velocity


                //boundaries?? - HELL YEAH
                // if (stepperX >= SCREEN_WIDTH - shapeSize) stepperX = SCREEN_WIDTH - shapeSize; //right boundary
                // if (stepperX <= shapeSize) stepperX = shapeSize; //left boundary
                // if (stepperY <= shapeSize) stepperY = shapeSize; //top boundary
                // if (position >= SCREEN_HEIGHT - shapeSize){  //bottom boundary
                //     velocityY *= -1 * energyLoss; //velocity goes in the other direction and apply energyLoss
                //     position = SCREEN_HEIGHT - shapeSize; //This is a stupid hacky way of doing things - it works

                // } 


                if (position >= SCREEN_WIDTH - shapeSize){ //right boundary

                    velocityX *= -1 * energyLoss; 
                    position = SCREEN_HEIGHT - shapeSize; 
                }
                
                if (position <= shapeSize){ //left boundary
                    velocityX *= -1 * energyLoss; 
                    velocityX = shapeSize; 
                }
                
                if (position <= shapeSize){ //top boundary
                    velocityY *= -1 * energyLoss; 
                    velocityY = shapeSize; 
                }

                if (position >= SCREEN_HEIGHT - shapeSize){  //bottom boundary
                    velocityY *= -1 * energyLoss; //velocity goes in the other direction and apply energyLoss
                    position = SCREEN_HEIGHT - shapeSize; //This is a stupid hacky way of doing things - it works

                } 
                
                


                //Maybe place shapes here?
                //circle
                SDL_SetRenderDrawColor(gRenderer, 65, 107, 223, 255); // Set color (r, g, b, a)
                DrawFilledCircle(gRenderer, stepperX, position, shapeSize); //This will draw filled circle
                // DrawCircle(gRenderer, stepperX, stepperY, 60); //This will draw a circle that isn't filled



                //this is for text
                renderTexture(&gTextTexture, 650,0, NULL, 0, NULL, SDL_FLIP_NONE); //this is for text (dk, posx, posy, dk, dk, dk,dk); 

                SDL_RenderPresent(gRenderer); // Update screen
            }
        }
    }
    close(); // Free resources and close SDL

    return 0;
}
