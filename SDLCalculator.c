#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Screen dimension constants
const int SCREEN_WIDTH = 249;  
const int SCREEN_HEIGHT = 450;  
const int BUTTON_WIDTH = 83; 
const int BUTTON_HEIGHT = 60; 
const int NUM_BUTTONS = 18; 


//button width 83 (3 buttons horizontally)
//button height 66 (6 buttons vertically) //18 buttons total


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
int isWithinBoundaries(SDL_Rect rect, int mouseX, int mouseY); //check if mouse is within boundaries of rect
int whereIsMyMouse(SDL_Rect rect, int mouseX, int mouseY); //check where mouse is 
void buttonDrawer(SDL_Renderer* renderer, SDL_Rect buttons[], int count); 
void grid(); 


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
        gWindow = SDL_CreateWindow("SDL Calculator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
    gFont = TTF_OpenFont("/Users/ghassanmuradagha/Documents/pro/fonts/open-sans/OpenSans-Light.ttf", 20); //font size
    if (gFont == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        success = false;
    } else {
        SDL_Color textColor = {0, 0, 0}; // Black text color

        // Render the text to create a texture
        if (!loadFromRenderedText(&gTextTexture, "Answer: ", textColor)) {
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


int isWithinBoundaries(SDL_Rect rect, int mouseX, int mouseY){ //check if mouse is within boundaries (used for pressing)

    if (mouseX >= rect.x && mouseX <= rect.x + rect.w && mouseY >= rect.y && mouseY <= rect.y + rect.h) return 1; //make sure its in the y and x                                          
    
    else return 0;
    
    return -1;
}

int whereIsMyMouse(SDL_Rect rect, int mouseX, int mouseY){ //check if mouse is within boundaries (used for getting position)

    return (mouseX >= rect.x && mouseX <= rect.x + rect.w && mouseY >= rect.y && mouseY <= rect.y + rect.h); 


}


void buttonDrawer(SDL_Renderer* renderer, SDL_Rect buttons[], int count) {
    
    SDL_SetRenderDrawColor(renderer, 105, 105, 105, 255); // Button color
    for (int i = 0; i < count; i++) {
        SDL_RenderFillRect(renderer, &buttons[i]); // Draw each button


    }
}

void grid(){
    int spacer = 0; 
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255); // Button color

    // int SDL_RenderDrawLine(SDL_Renderer * renderer, int x1, int y1, int x2, int y2);

    for (int i = 0; i < 3; i++)
    {
        SDL_RenderDrawLine(gRenderer, BUTTON_WIDTH + spacer, SCREEN_HEIGHT, BUTTON_WIDTH + spacer, 90); 
        spacer += BUTTON_WIDTH;
    }
    spacer = 60; 

    for (int i = 0; i < 6; i++)
    {
        SDL_RenderDrawLine(gRenderer, 0, SCREEN_HEIGHT - spacer, SCREEN_WIDTH, SCREEN_HEIGHT - spacer); 
        spacer += BUTTON_HEIGHT;
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




            SDL_Rect buttons[NUM_BUTTONS]; 
            int spacerHor = 0; //testing for now
            int spacerVer = 60; 
            int exactButton = 0; //this is stupid but this gives memory to the inner for loop




            for (int c = 0; c < 6; c++)
            {
                for (int i = 0; i < 3; i++)
                {
                    buttons[i+exactButton].w = BUTTON_WIDTH; 
                    buttons[i+exactButton].h = BUTTON_HEIGHT; 
                    buttons[i+exactButton].y = SCREEN_HEIGHT - spacerVer; 
                    buttons[i+exactButton].x = spacerHor; 
                    spacerHor += 83; 

                }
                spacerHor = 0;
                spacerVer+=60;
                exactButton+=3;
            }
            
            

            

            

            while (!quit) {
                while (SDL_PollEvent(&e) != 0) { // Handle events

                //place controls below...
                    if (e.type == SDL_QUIT) {
                        quit = 1; // User requests quit
                    }

                    if (e.type == SDL_KEYDOWN) {
                      if (e.key.keysym.sym == SDLK_ESCAPE) {
                          quit = true; // Exit on pressing the escape key


                      }
                    
                    }
                    
                }

                // Clear screen with white background
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                SDL_RenderClear(gRenderer);

                //Maybe place shapes here?
                //rect:

                buttonDrawer(gRenderer, buttons, NUM_BUTTONS); 
                grid(); //this will draw the grid

                

                
                renderTexture(&gTextTexture, 0,60, NULL, 0, NULL, SDL_FLIP_NONE); //this is for text (dk, posx, posy, dw, dw, dw,dw); 
                SDL_RenderPresent(gRenderer); // Update screen
            }
        }
    }
    close(); // Free resources and close SDL

    return 0;
}
