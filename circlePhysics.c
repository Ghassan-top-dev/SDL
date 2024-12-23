// gcc -O3 -I src/include -L src/lib -o main circlePhysics.c -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer -mwindows

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
// the size of the screen
#define SCREEN_WIDTH 1400 
#define SCREEN_HEIGHT 750


// the rate of change of the velocity each frame
#define GRAVITY 0.5f
#define MAX_VELOCITY 10.0f
#define ENERGY_LOSS 0.7

// Struct for storing circle data
typedef struct {
    int posX, posY;        // Position
    int velocityY, velocityX;      // Velocity
    int radius;      // Radius
    float mass; // mass
} Circle;

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
        gWindow = SDL_CreateWindow("Elastic Physics", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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


//circle functions:

void DrawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
    // Loop through the vertical positions (y-coordinates) of the circle from the top to the bottom
    for (int y = -radius; y <= radius; y++) {
        // Calculate the horizontal distance (half-width) at this y-level using the circle equation
        int dx = (int)sqrt(radius * radius - y * y);  // Could use a faster square root approximation

        // Draw a horizontal line across the diameter of the circle at the current y-level
        SDL_RenderDrawLine(renderer, centerX - dx, centerY + y, centerX + dx, centerY + y);
    }
}

void HandleCircleCollision(Circle* c1, Circle* c2) {
    // Calculate the distance between the two circles
    int dx = c2->posX - c1->posX;
    int dy = c2->posY - c1->posY;
    int dist = sqrt(dx * dx + dy * dy);

    // Check if the circles are colliding (i.e., distance is less than sum of radii)
    if (dist < c1->radius + c2->radius) {
        // Calculate the normal vector (direction of collision)
        float nx = dx / (float)dist;
        float ny = dy / (float)dist;

        // Relative velocity in the normal direction
        float vn = (c2->velocityX - c1->velocityX) * nx + (c2->velocityY - c1->velocityY) * ny;

        // Only perform collision if the balls are moving towards each other
        if (vn < 0) {
            // Calculate impulse scalar using the masses and relative velocity
            float impulse = (2 * vn) / (c1->mass + c2->mass);

            // Update velocities of the balls after the collision
            c1->velocityX -= impulse * c2->mass * nx;
            c1->velocityY -= impulse * c2->mass * ny;
            c2->velocityX += impulse * c1->mass * nx;
            c2->velocityY += impulse * c1->mass * ny;

            // Move the balls out of overlap by adjusting their positions
            int overlap = (c1->radius + c2->radius) - dist;
            c1->posX -= overlap * (c1->radius / (float)(c1->radius + c2->radius)) * nx;
            c1->posY -= overlap * (c1->radius / (float)(c1->radius + c2->radius)) * ny;
            c2->posX += overlap * (c2->radius / (float)(c1->radius + c2->radius)) * nx;
            c2->posY += overlap * (c2->radius / (float)(c1->radius + c2->radius)) * ny;
        }
    }
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

            // Create multiple circles
            // posX, posY, velocityX, velocityY, radius, mass
            Circle circles[2] = {
                {100, 100, 2, 3, 30, 80},
                {200, 200, -3, 2, 60, 200}
            };

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
                // Clear screen with white background
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                SDL_RenderClear(gRenderer);     

                // Update and draw each circle
                for (int i = 0; i < 2; i++) {
                    // Update position based on velocity
                    circles[i].posX += circles[i].velocityX;
                    circles[i].posY += circles[i].velocityY;

                    // Handle boundaries
                    // left and right
                    if (circles[i].posX >= SCREEN_WIDTH - circles[i].radius || circles[i].posX <= circles[i].radius) {
                        circles[i].velocityX *= -1;
                    }
                    // top and bottom
                    if (circles[i].posY >= SCREEN_HEIGHT - circles[i].radius || circles[i].posY <= circles[i].radius) {
                        circles[i].velocityY *= -1;
                    }

                    // Set color for each circle
                    SDL_SetRenderDrawColor(gRenderer, 65, 107, 223, 255);
                    // Draw the circle
                    DrawFilledCircle(gRenderer, circles[i].posX, circles[i].posY, circles[i].radius);
                }

                for (int i = 0; i < 2; i++) {
                    for (int j = i + 1; j < 2; j++) {
                        HandleCircleCollision(&circles[i], &circles[j]);
                    }
                }
                
                //this is for text
                renderTexture(&gTextTexture, 0,0, NULL, 0, NULL, SDL_FLIP_NONE); //this is for text (dk, posx, posy, dk, dk, dk,dk); 
                SDL_RenderPresent(gRenderer); // Update screen

                // Optional: Add a small delay to control simulation speed
                SDL_Delay(16);
            }
        }
    }
    close(); // Free resources and close SDL

    return 0;
}
