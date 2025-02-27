/*///////////////////////////////////////////////////
  /$$$$$$  /$$                                                                         
 /$$__  $$| $$                                                                         
| $$  \__/| $$$$$$$   /$$$$$$   /$$$$$$$ /$$$$$$$  /$$$$$$  /$$$$$$$                   
| $$ /$$$$| $$__  $$ |____  $$ /$$_____//$$_____/ |____  $$| $$__  $$                  
| $$|_  $$| $$  \ $$  /$$$$$$$|  $$$$$$|  $$$$$$   /$$$$$$$| $$  \ $$                  
| $$  \ $$| $$  | $$ /$$__  $$ \____  $$\____  $$ /$$__  $$| $$  | $$                  
|  $$$$$$/| $$  | $$|  $$$$$$$ /$$$$$$$//$$$$$$$/|  $$$$$$$| $$  | $$                  
 \______/ |__/  |__/ \_______/|_______/|_______/  \_______/|__/  |__/                  
                                                                                       
                                                                                       
                                                                                       
 /$$      /$$                                    /$$                                   
| $$$    /$$$                                   | $$                                   
| $$$$  /$$$$ /$$   /$$  /$$$$$$  /$$$$$$   /$$$$$$$                                   
| $$ $$/$$ $$| $$  | $$ /$$__  $$|____  $$ /$$__  $$                                   
| $$  $$$| $$| $$  | $$| $$  \__/ /$$$$$$$| $$  | $$                                   
| $$\  $ | $$| $$  | $$| $$      /$$__  $$| $$  | $$                                   
| $$ \/  | $$|  $$$$$$/| $$     |  $$$$$$$|  $$$$$$$                                   
|__/     |__/ \______/ |__/      \_______/ \_______/                                   
                                                                                       
                                                                                       
                                                                                       
  /$$$$$$            /$$                                                               
 /$$__  $$          | $$                                                               
| $$  \ $$  /$$$$$$ | $$$$$$$   /$$$$$$                                                
| $$$$$$$$ /$$__  $$| $$__  $$ |____  $$                                               
| $$__  $$| $$  \ $$| $$  \ $$  /$$$$$$$                                               
| $$  | $$| $$  | $$| $$  | $$ /$$__  $$                                               
| $$  | $$|  $$$$$$$| $$  | $$|  $$$$$$$                                               
|__/  |__/ \____  $$|__/  |__/ \_______/                                               
           /$$  \ $$                                                                   
          |  $$$$$$/                                                                   
           \______/                                                                    
                                                                                       

Free for full use...

Email: ghassanjmurad@gmail.com

Linkedin: https://www.linkedin.com/in/ghassan-murad-agha-579a3730b/

GitHub: https://github.com/Ghassan-top-dev
                                                                       
                                                                                                                                                                                                     
                                                                                                                                                                


///////////////////////////////////////////////////*/


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
#define SCREEN_WIDTH 1392 
#define SCREEN_HEIGHT 744
// the size of the "cell" in this case for ex: 4 would mean a 4*4 pixel cell composed of 16 pixels
#define PIXEL_SIZE 4
// the rate of change of the velocity each frame
#define GRAVITY 0.5f
#define MAX_VELOCITY 10.0f

// this is the grid of the actual cells, if grid width is 100 for example, then you can have 100
// cells horizontally
#define GRID_HEIGHT (SCREEN_HEIGHT / PIXEL_SIZE)
#define GRID_WIDTH (SCREEN_WIDTH / PIXEL_SIZE)

// Texture wrapper structure to hold texture data and dimensions
typedef struct {
    SDL_Texture* texture;
    int width;
    int height;
} LTexture;

SDL_Color textColor = {255, 255, 255}; // text color
SDL_Color textColorForSubstances = {255, 255, 255};


//Code for the sandbox
// enumeration of the different values that a cell could contain
typedef enum {
    EMPTY = 0,
    SAND = 1,
    WATER = 2,
    WOOD = 3,
    FIRE = 4,
    STEAM = 5

} PixelType;

typedef struct {
    int r; // Red component
    int g; // Green component
    int b; // Blue component
    int a; // Alpha component
} Color;

typedef struct {
    PixelType type;          // Pixel type, e.g., EMPTY, SAND
    bool exists;             // all true except for the emptyPixel
    float velocity;          // velocity member
    bool updated;            // very important member that skips a cell if it has been updated this frame
    int lifetime;            // how many frames before this cell decays into another cell
    int howManyFramesNearBurnable;  // this applies to fire, this is the amount of frames that wood is near fire
    Color colour;   // Pixel color for rendering
} Pixel;

// COLOR VARIABLES

int s1 = 0, s2 = 0, s3 = 0; // sand colors

const Color colors[] = {
    // Sand Colors
    {234, 225, 176, 255}, // Sand color 1
    {229, 216, 144, 255}, // Sand color 2
    {195, 184, 124, 255}, // Sand color 3
    {225, 200, 20, 255},  // Sand color 4
    {207, 224, 227, 255}, // Sand color 5

    // Fire Colors
    {161, 0, 0, 255},     // Fire color 1
    {234, 35, 0, 255},    // Fire color 2
    {255, 129, 0, 255},   // Fire color 3
    {242, 85, 0, 255},    // Fire color 4
    {216, 0, 0, 255},     // Fire color 5

    // Wood Color
    {39, 24, 16, 255},    // wood color 1
    {79, 32, 15, 255},    // wood color 2
    {149, 69, 32, 255},   // wood color 3
    {199, 108, 63, 255},  // wood color 4
    {189, 148, 118, 255}  // wood color 5
};

// this is for the text and the color of the text for each substance
typedef struct {
    const char *name;
    SDL_Color color;
} Substance;


// Array for the 8 possible directions + the current position itself (optional depending on needs)
int offsets[8][2] = {
    {-1, -1}, {0, -1}, {1, -1}, // Top-left, Top, Top-right
    {-1,  0},          {1,  0}, // Left,        , Right
    {-1,  1}, {0,  1}, {1,  1}  // Bottom-left, Bottom, Bottom-right
};

// Main grid
Pixel GRID[GRID_WIDTH][GRID_HEIGHT]; 
// this is for clearing the screen
Pixel EMPTY_GRID[GRID_WIDTH][GRID_HEIGHT];

// substances
Pixel emptyPixel = {EMPTY, false, 0, false, -1, -1, {0, 0, 0, 255}};
Pixel sandPixel = {SAND, true, 0, false, -1, -1, {100, 100, 100, 255}};
Pixel waterPixel = {WATER, true, 0, false, -1, -1, {15, 94, 156, 255}};
Pixel woodPixel = {WOOD, true, 0, false, -1, -1, {222, 184, 135, 255}};
Pixel firePixel = {FIRE, true, 0, false, 16, 20, {128, 9, 9, 255}};
Pixel steamPixel = {STEAM, true, 0, false, 400, -1, {75, 80, 75, 25}};

// Function declarations
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
    gFont = TTF_OpenFont("bit5x3.ttf", 21); //font size
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
    // Render particles
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            if (GRID[x][y].exists) {
                SDL_Rect particle_rect = {x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE};

                SDL_SetRenderDrawColor(gRenderer, 
                    GRID[x][y].colour.r, 
                    GRID[x][y].colour.g, 
                    GRID[x][y].colour.b, 
                    GRID[x][y].colour.a);
                SDL_RenderFillRect(gRenderer, &particle_rect);
            }
        }
    }
}


// this is a function that takes a random integer and that decides which color will be used 
// from a predefined set of colors. it also takes the 3 variables that will change the color
// of the substance. lastly it takes the current mode and that will decide which set of colors to choose from
// This color stuff is complicated
// 'colors' is the constant array. 'colour' is the variable. 'Color' is the typdef for the struct
void randColor(int *v1, int *v2, int *v3, int subMode) {
    int randSandNum;
    switch (subMode)
    {
    case 1:
        randSandNum = rand() % 5;  // Random index (0 to 4)
        sandPixel.colour = colors[randSandNum];
        break;
    case 3:
        randSandNum = 10 + (rand() % 5); // Random index (10 to 14)
        woodPixel.colour = colors[randSandNum];
        break;
    case 4:
        randSandNum = 5 + (rand() % 5); // Random index (5 to 9)
        firePixel.colour = colors[randSandNum];
        break;
        
    
    default:
        break;
    }
    
    
}


// New function to update all substances. It works by alternating between scanning from right to left and 
// scanning from left to right on the horizontal depending on if 'y' is even or odd. once a cell is updated
// its updated flag is set to true then every frame all flags are reset to false
void updatePhysics() {
    // Reset update flags and decrease lifetime
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            GRID[x][y].updated = false;
            GRID[x][y].lifetime--;
        }
    }
    
    // Update from bottom to top to simulate gravity
    for (int y = GRID_HEIGHT - 1; y >= 0; y--) {

        if (y % 2 == 0) // Scan left to right
        {
            for (int x = 0; x < GRID_WIDTH; x++) {


                // very straightforward checks
                if (GRID[x][y].type == steamPixel.type && GRID[x][y].lifetime == 0){

                    if (rand() % 100 < 75) GRID[x][y] = emptyPixel;
                    else GRID[x][y].lifetime = 100;
                    continue;

                }

                if (GRID[x][y].type == firePixel.type && GRID[x][y].lifetime == 0){

                    GRID[x][y] = steamPixel;

                }
                else if(GRID[x][y].type == firePixel.type){

                    for (int i = 0; i < 8; i++) {
                        int nx = x + offsets[i][0]; // Neighbor's x-coordinate
                        int ny = y + offsets[i][1]; // Neighbor's y-coordinate

                        // Check bounds
                        if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT) {
                            if (GRID[x][y].howManyFramesNearBurnable == 0 && GRID[nx][ny].type == woodPixel.type)
                            {
                                
                                randColor(&s1, &s2, &s3, 4); 
                                GRID[nx][ny] = firePixel; // Example: Convert wood to fire
                            
                                continue;
                            }
                            
                            else if (GRID[x][y].howManyFramesNearBurnable > 0 && GRID[nx][ny].type == woodPixel.type) {
                                // Process wood interaction
                                GRID[x][y].howManyFramesNearBurnable--;
                                continue;

                            }
                        }
                    }
                    continue;

                }


                if (GRID[x][y].type == sandPixel.type) // THIS IS FOR SAND (1/2)
                {
                    if (GRID[x][y].exists && !GRID[x][y].updated) {
                        // Apply gravity
                        GRID[x][y].velocity += GRAVITY;
                        
                        // Cap maximum velocity
                        if (GRID[x][y].velocity > MAX_VELOCITY) GRID[x][y].velocity = MAX_VELOCITY;

                        // Find maximum falling distance
                        int maxFallDistance = (int)GRID[x][y].velocity;
                        int fallDistance = 0;
                        
                        // Check falling distance
                        // ADD OTHER SUBSTANCES THAT INTERACT WITH SAND HERE FOR GOING STRAIGHT DOWN
                        for (int dy = 1; dy <= maxFallDistance; dy++) {
                            if (y + dy < GRID_HEIGHT && (GRID[x][y + dy].type == emptyPixel.type || GRID[x][y + dy].type == waterPixel.type || GRID[x][y + dy].type == steamPixel.type) ) fallDistance = dy;
                            
                            else break;

                        }

                        // If we can fall
                        // here add the other substances that sand can fall through
                        if (fallDistance > 0) {

                            if (GRID[x][y + fallDistance].type == waterPixel.type || GRID[x][y + fallDistance].type == steamPixel.type) {
                                // Swap sand and water
                                Pixel temp = GRID[x][y + fallDistance];
                                GRID[x][y + fallDistance] = GRID[x][y];
                                GRID[x][y] = temp;
                            } 
                            else {
                                // Move sand down
                                GRID[x][y + fallDistance] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                            }
                            GRID[x][y + fallDistance].updated = true;
                                                        


                        
                        }
                        // If can't fall straight, try diagonal
                        else {
                            int fallDirection = (rand() % 2 == 0) ? -1 : 1;
                            int newX = x + fallDirection;
                            
                            // Check diagonal falling
                            if (newX >= 0 && newX < GRID_WIDTH && 
                                y + 1 < GRID_HEIGHT && 
                                !GRID[newX][y + 1].exists) {
                                // Move pixel diagonally
                                GRID[newX][y + 1] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                                GRID[newX][y + 1].updated = true;
                                
                                // Reduce velocity when falling diagonally
                                GRID[newX][y + 1].velocity *= 0.7;
                            }
                            else {
                                // If can't fall, reduce velocity
                                GRID[x][y].velocity *= 0.5;
                                if (GRID[x][y].velocity < 0.1) GRID[x][y].velocity = 0;

                            }
                        }
                    }
                }

                else if (GRID[x][y].type == waterPixel.type){ // THIS IS FOR WATER (1/2)
                    if (GRID[x][y].exists && !GRID[x][y].updated) {
                        // Apply gravity
                        GRID[x][y].velocity += GRAVITY;
                        
                        // Cap maximum velocity
                        if (GRID[x][y].velocity > MAX_VELOCITY) GRID[x][y].velocity = MAX_VELOCITY;

                        // Find maximum falling distance
                        int maxFallDistance = (int)GRID[x][y].velocity;
                        int fallDistance = 0;
                        
                        // Check falling distance
                        for (int dy = 1; dy <= maxFallDistance; dy++) {
                            if (y + dy < GRID_HEIGHT && (GRID[x][y + dy].type == emptyPixel.type || GRID[x][y + dy].type == steamPixel.type) ) fallDistance = dy;

                            else break;

                        }

                        // If we can fall
                        if (fallDistance > 0) {

                            if (GRID[x][y + fallDistance].type == steamPixel.type) {
                                // Swap sand and water
                                Pixel temp = GRID[x][y + fallDistance];
                                GRID[x][y + fallDistance] = GRID[x][y];
                                GRID[x][y] = temp;
                            } 
                            else {
                                // Move sand down
                                GRID[x][y + fallDistance] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                            }
                            GRID[x][y + fallDistance].updated = true;
                        
                        }
                        // If can't fall straight, try diagonal
                        else {
                            int fallDirection = (rand() % 2) * 2 - 1; // -1 or 1

                            int newX = x + fallDirection;
                            
                            if (newX >= 0 && newX < GRID_WIDTH && !GRID[newX][y].exists) {
                                GRID[newX][y] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                            }
                            
                            // Try opposite direction
                            newX = x - fallDirection;
                            if (newX >= 0 && newX < GRID_WIDTH && !GRID[newX][y].exists) {
                                GRID[newX][y] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                            }
                            
                            // Try diagonal movement if horizontal movement wasn't possible
                            if (y + 1 < GRID_HEIGHT) {
                                if (x + 1 < GRID_WIDTH && !GRID[x + 1][y + 1].exists) {
                                    GRID[x + 1][y + 1] = GRID[x][y];
                                    GRID[x][y] = emptyPixel;
                                } 
                                else if (x - 1 >= 0 && !GRID[x - 1][y + 1].exists) {
                                    GRID[x - 1][y + 1] = GRID[x][y];
                                    GRID[x][y] = emptyPixel;
                                }
                            }
                            else {
                                // If can't fall, reduce velocity
                                GRID[x][y].velocity *= 0.5;
                                if (GRID[x][y].velocity < 0.1) GRID[x][y].velocity = 0;

                            }
                        }
                    }
                }

                else if (GRID[x][y].type == steamPixel.type){// THIS IS FOR STEAM (1/2)

                    if (GRID[x][y].exists && !GRID[x][y].updated) {

                        // If we can go up
                        if (GRID[x][y - 1].type == EMPTY && y - 1 >= 0) {
                            // Move pixel down
                            GRID[x][y - 1] = GRID[x][y];
                            GRID[x][y] = emptyPixel;
                            GRID[x][y - 1].updated = true;
                            
                        }
                        // If can't fall straight, try diagonal
                        else {
                            int fallDirection = (rand() % 2) * 2 - 1; // -1 or 1

                            int newX = x + fallDirection;
                            
                            if (newX >= 0 && newX < GRID_WIDTH && !GRID[newX][y].exists) {
                                GRID[newX][y] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                            }
                            
                            // Try opposite direction
                            newX = x - fallDirection;
                            if (newX >= 0 && newX < GRID_WIDTH && !GRID[newX][y].exists) {
                                GRID[newX][y] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                            }
                            
                            // Try diagonal movement if horizontal movement wasn't possible
                            if (y - 1 >= 0) {
                                if (x + 1 < GRID_WIDTH && !GRID[x + 1][y - 1].exists) {
                                    GRID[x + 1][y - 1] = GRID[x][y];
                                    GRID[x][y] = emptyPixel;
                                } 
                                else if (x - 1 >= 0 && !GRID[x - 1][y - 1].exists) {
                                    GRID[x - 1][y - 1] = GRID[x][y];
                                    GRID[x][y] = emptyPixel;
                                }
                            }
                        }
                    }


                }

            } 
            

        }
        else{ // Scan right to left

            for (int x = GRID_WIDTH - 1; x >= 0; --x) {


                if (GRID[x][y].type == steamPixel.type && GRID[x][y].lifetime == 0){

                    if (rand() % 100 < 75) GRID[x][y] = emptyPixel;
                    else GRID[x][y].lifetime = 100;
                    continue;

                }

                if (GRID[x][y].type == firePixel.type && GRID[x][y].lifetime == 0){

                    GRID[x][y] = steamPixel;

                }
                else if(GRID[x][y].type == firePixel.type){

                    for (int i = 0; i < 8; i++) {
                        int nx = x + offsets[i][0]; // Neighbor's x-coordinate
                        int ny = y + offsets[i][1]; // Neighbor's y-coordinate

                        // Check bounds
                        if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT) {
                            if (GRID[x][y].howManyFramesNearBurnable == 0 && GRID[nx][ny].type == woodPixel.type)
                            {
                                
                                randColor(&s1, &s2, &s3, 4); 
                                GRID[nx][ny] = firePixel; // Example: Convert wood to fire
                                                       
                                continue;
                            }
                            
                            else if (GRID[x][y].howManyFramesNearBurnable > 0 && GRID[nx][ny].type == woodPixel.type) {
                                // Process wood interaction
                                GRID[x][y].howManyFramesNearBurnable--;
                                continue;

                            }
                        }
                    }
                    continue;

                }

                if (GRID[x][y].type == sandPixel.type) // THIS IS FOR SAND (2/2)
                {
                    if (GRID[x][y].exists && !GRID[x][y].updated) {
                        // Apply gravity
                        GRID[x][y].velocity += GRAVITY;
                        
                        // Cap maximum velocity
                        if (GRID[x][y].velocity > MAX_VELOCITY) GRID[x][y].velocity = MAX_VELOCITY;

                        // Find maximum falling distance
                        int maxFallDistance = (int)GRID[x][y].velocity;
                        int fallDistance = 0;
                        
                        // Check falling distance
                        for (int dy = 1; dy <= maxFallDistance; dy++) {
                            if (y + dy < GRID_HEIGHT && (GRID[x][y + dy].type == emptyPixel.type || GRID[x][y + dy].type == waterPixel.type || GRID[x][y + dy].type == steamPixel.type) ) fallDistance = dy;
                            
                            else break;

                        }

                        // If we can fall
                        // here add the other substances that sand can fall through
                        if (fallDistance > 0) {

                            if (GRID[x][y + fallDistance].type == waterPixel.type || GRID[x][y + fallDistance].type == steamPixel.type) {
                                // Swap sand and water
                                Pixel temp = GRID[x][y + fallDistance];
                                GRID[x][y + fallDistance] = GRID[x][y];
                                GRID[x][y] = temp;
                            } 
                            else {
                                // Move sand down
                                GRID[x][y + fallDistance] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                            }
                            GRID[x][y + fallDistance].updated = true;
                                                        


                        
                        }
                        // If can't fall straight, try diagonal
                        else {
                            int fallDirection = (rand() % 2 == 0) ? -1 : 1;
                            int newX = x + fallDirection;
                            
                            // Check diagonal falling
                            if (newX >= 0 && newX < GRID_WIDTH && 
                                y + 1 < GRID_HEIGHT && 
                                !GRID[newX][y + 1].exists) {
                                // Move pixel diagonally
                                GRID[newX][y + 1] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                                GRID[newX][y + 1].updated = true;
                                
                                // Reduce velocity when falling diagonally
                                GRID[newX][y + 1].velocity *= 0.7;
                            }
                            else {
                                // If can't fall, reduce velocity
                                GRID[x][y].velocity *= 0.5;
                                if (GRID[x][y].velocity < 0.1) GRID[x][y].velocity = 0;
                            }
                        }
                    }
                }





                else if (GRID[x][y].type == waterPixel.type){ // THIS IS FOR WATER (2/2)
                    if (GRID[x][y].exists && !GRID[x][y].updated) {
                        // Apply gravity
                        GRID[x][y].velocity += GRAVITY;
                        
                        // Cap maximum velocity
                        if (GRID[x][y].velocity > MAX_VELOCITY) GRID[x][y].velocity = MAX_VELOCITY;

                        // Find maximum falling distance
                        int maxFallDistance = (int)GRID[x][y].velocity;
                        int fallDistance = 0;
                        
                        // Check falling distance
                        for (int dy = 1; dy <= maxFallDistance; dy++) {
                            if (y + dy < GRID_HEIGHT && (GRID[x][y + dy].type == emptyPixel.type || GRID[x][y + dy].type == steamPixel.type) ) fallDistance = dy;

                            else break;

                        }

                        // If we can fall
                        if (fallDistance > 0) {

                            if (GRID[x][y + fallDistance].type == steamPixel.type) {
                                // Swap sand and water
                                Pixel temp = GRID[x][y + fallDistance];
                                GRID[x][y + fallDistance] = GRID[x][y];
                                GRID[x][y] = temp;
                            } 
                            else {
                                // Move sand down
                                GRID[x][y + fallDistance] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                            }
                            GRID[x][y + fallDistance].updated = true;
                        
                        }
                        // If can't fall straight, try diagonal
                        else {
                            int fallDirection = (rand() % 2) * 2 - 1; // -1 or 1

                            int newX = x + fallDirection;
                            
                            if (newX >= 0 && newX < GRID_WIDTH && !GRID[newX][y].exists) {
                                GRID[newX][y] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                            }
                            
                            // Try opposite direction
                            newX = x - fallDirection;
                            if (newX >= 0 && newX < GRID_WIDTH && !GRID[newX][y].exists) {
                                GRID[newX][y] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                            }
                            
                            // Try diagonal movement if horizontal movement wasn't possible
                            if (y + 1 < GRID_HEIGHT) {
                                if (x + 1 < GRID_WIDTH && !GRID[x + 1][y + 1].exists) {
                                    GRID[x + 1][y + 1] = GRID[x][y];
                                    GRID[x][y] = emptyPixel;
                                } 
                                else if (x - 1 >= 0 && !GRID[x - 1][y + 1].exists) {
                                    GRID[x - 1][y + 1] = GRID[x][y];
                                    GRID[x][y] = emptyPixel;
                                }
                            }
                            else {
                            
                                // If can't fall, reduce velocity
                                GRID[x][y].velocity *= 0.5;
                                if (GRID[x][y].velocity < 0.1)  GRID[x][y].velocity = 0;

                            }
                        }
                    }
                }

                else if (GRID[x][y].type == steamPixel.type){// THIS IS FOR STEAM (2/2)

                    if (GRID[x][y].exists && !GRID[x][y].updated) {

                        // If we can go up
                        if (GRID[x][y - 1].type == EMPTY) {
                            // Move pixel down
                            GRID[x][y - 1] = GRID[x][y];
                            GRID[x][y] = emptyPixel;
                            GRID[x][y - 1].updated = true;
                            
                        }
                        // If can't fall straight, try diagonal
                        else {
                            int fallDirection = (rand() % 2) * 2 - 1; // -1 or 1

                            int newX = x + fallDirection;
                            
                            if (newX >= 0 && newX < GRID_WIDTH && !GRID[newX][y].exists) {
                                GRID[newX][y] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                            }
                            
                            // Try opposite direction
                            newX = x - fallDirection;
                            if (newX >= 0 && newX < GRID_WIDTH && !GRID[newX][y].exists) {
                                GRID[newX][y] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                            }
                            
                            // Try diagonal movement if horizontal movement wasn't possible
                            if (y - 1 >= 0) {
                                if (x + 1 < GRID_WIDTH && !GRID[x + 1][y - 1].exists) {
                                    GRID[x + 1][y - 1] = GRID[x][y];
                                    GRID[x][y] = emptyPixel;
                                } 
                                else if (x - 1 >= 0 && !GRID[x - 1][y - 1].exists) {
                                    GRID[x - 1][y - 1] = GRID[x][y];
                                    GRID[x][y] = emptyPixel;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


// this function takes the position of the mouse, and the choice of substance and turns the area of 'dropperSize' into 
// that substance before it is rendered or updated
void instantiateSubstance(int x, int y, int dropperSize, int substanceMode) { 
    // how big you want the spawner to be
    int spawn_range = dropperSize;
    // for all the squares pixels in that square
    for (int dx = -spawn_range; dx <= spawn_range; dx++) {
        for (int dy = -spawn_range; dy <= spawn_range; dy++) {
            int pixelBlockX = (x / PIXEL_SIZE) + dx;
            int pixelBlockY = (y / PIXEL_SIZE) + dy;
            
            if (pixelBlockX >= 0 && pixelBlockX < GRID_WIDTH && pixelBlockY >= 0 && pixelBlockY < GRID_HEIGHT) {
                if (!GRID[pixelBlockX][pixelBlockY].exists) {
                    // instantiate the substance along with its designated color
                    switch (substanceMode)
                    {
                    case 1:
                        randColor(&s1, &s2, &s3, 1); 
                        if (rand() % 100 < 75) GRID[pixelBlockX][pixelBlockY] = sandPixel;
                        break;
                    case 2:
                        if (rand() % 100 < 75) GRID[pixelBlockX][pixelBlockY] = waterPixel;
                        break;
                    case 3:
                        randColor(&s1, &s2, &s3, 3); 
                        GRID[pixelBlockX][pixelBlockY] = woodPixel;
                        break;
                    case 4:
                        randColor(&s1, &s2, &s3, 4); 
                        if (rand() % 100 < 55) GRID[pixelBlockX][pixelBlockY] = firePixel;
                        break;
                    
                    default:
                        break;
                    }
                }
                // erase cells
                else if(GRID[pixelBlockX][pixelBlockY].exists && substanceMode == 0){

                    GRID[pixelBlockX][pixelBlockY] = emptyPixel;


                }
            }
        }
    }
}

// main function
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
            // handles mouse presses
            bool pressed = false;
            
            // text variables
            int mode = 0; char modePresented[32], lastMode = -1; //which substance
            const Substance lookUpOfSubstances[] = {
                {"Erase", {255, 255, 255, 255}}, // White for erase
                {"Sand", {234, 225, 176, 255}}, // Sand color
                {"Water", {0, 0, 255, 255}},    // Blue for water
                {"Wood", {139, 69, 19, 255}},   // Brown for wood
                {"Fire", {255, 0, 0, 255}}      // Red for fire
            };
            // initial size of the dropper
            int sizeOfDropping = 2; 

            while (!quit) {
                while (SDL_PollEvent(&event) != 0) {
                    // controls
                    if (event.type == SDL_QUIT) quit = 1;
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
                        if (event.key.keysym.sym == SDLK_c) memcpy(GRID, EMPTY_GRID, sizeof(GRID));

                        // mode for which substance will be dropped
                        if (event.key.keysym.sym == SDLK_RIGHT && mode+1 <= 4) mode+=1;
                        if (event.key.keysym.sym == SDLK_LEFT && mode-1 >= 0) mode-=1;

                        // this changes the size of the dropper
                        if (event.key.keysym.sym == SDLK_UP) sizeOfDropping+=1;
                        if (event.key.keysym.sym == SDLK_DOWN && sizeOfDropping-1 > 0) sizeOfDropping-=1;

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

                }
                
                if (pressed) {
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    instantiateSubstance(mouseX, mouseY, sizeOfDropping, mode);

                }
                // this chooses the mode and presents it
                if (mode != lastMode)
                {
                    const Substance *currentSubstance = (mode >= 1 && mode <= 4) ? &lookUpOfSubstances[mode] : &lookUpOfSubstances[0];
                    loadFromRenderedText(&modeTextTexture, currentSubstance->name, currentSubstance->color);
                    lastMode = mode; 
                }


                // Clear screen with grey background color
                SDL_SetRenderDrawColor(gRenderer, 110, 110, 110, 255);
                SDL_RenderClear(gRenderer);


                sprintf(modePresented, "Dropper Size: %d", sizeOfDropping); 
                loadFromRenderedText(&SizeOfDropperTexture, modePresented, textColor);

                // Update physics
                updatePhysics();

                // Render
                render();
                
                //this is for text
                renderTexture(&modeTextTexture, 0,0, NULL, 0, NULL, SDL_FLIP_NONE); 
                renderTexture(&SizeOfDropperTexture, 0,20, NULL, 0, NULL, SDL_FLIP_NONE); 
                SDL_RenderPresent(gRenderer); // Update screen

                // Optional: Add a small delay to control simulation speed
                SDL_Delay(16);

            }
        }
    }
    close();
    return 0;
}
