#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// Screen dimension constants
const int SCREEN_WIDTH = 1392;
const int SCREEN_HEIGHT = 744;
const int PIXEL_SIZE = 4; 

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
    WATER = 2,
    RAINBOW = 3,
    WOOD = 4,
    FIRE = 5
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
void setPixel(Pixel rect, int x, int y); 
void update_water(Pixel GRID[SCREEN_WIDTH][SCREEN_HEIGHT], const Pixel emptyPixel);
void updateSand(Pixel GRID[SCREEN_WIDTH][SCREEN_HEIGHT], const Pixel emptyPixel, const Pixel waterPixel);
void dropperSize(const Pixel pixelType, int mouseX, int mouseY, int sizeOfDropping); 



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
    gFont = TTF_OpenFont("/Users/ghassanmuradagha/Documents/pro/SDL_GENERAL/fonts/open-sans/OpenSans-Bold.ttf", 15); //font size
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


// Structure to hold RGB values
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RGB;

// Global variables to maintain state between cycles
RGB current_color = {0, 0, 0};
int stage = 0;

RGB get_next_color(void) {
    // Increment based on current stage
    switch (stage) {
        case 0: // R
            if (current_color.r < 255) current_color.r++;
            else stage++;
            break;
        case 1: // G
            if (current_color.g < 255) current_color.g++;
            else stage++;
            break;
        case 2: // R
            if (current_color.r > 0) current_color.r--;
            else stage++;
            break;
        case 3: // B
            if (current_color.b < 255) current_color.b++;
            else stage++;
            break;
        case 4: // G
            if (current_color.g > 0) current_color.g--;
            else stage++;
            break;
        case 5: // R
            if (current_color.r < 255) current_color.r++;
            else stage++;
            break;
        case 6: // B
            if (current_color.b > 0) current_color.b--;
            else stage = 0; // Reset to beginning
            break;
    }
    
    return current_color;
}


// Assuming these are your existing structs/types
// struct Pixel { int type; /* other properties */ };
// enum { EMPTY, WATER, SAND, RAINBOW };

void update_water(Pixel GRID[SCREEN_WIDTH][SCREEN_HEIGHT], const Pixel emptyPixel) {
    // First pass: move particles down

    for (int y = GRID_HEIGHT - 1; y >= 0; --y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            if (GRID[x][y].type == WATER) {
                // Try to move down
                if (GRID[x][y + 1].type == EMPTY) {
                    GRID[x][y + 1] = GRID[x][y];
                    GRID[x][y] = emptyPixel;
                }
            }
        }
    }
    
    // Second pass: handle horizontal spreading
    for (int y = GRID_HEIGHT - 1; y >= 0; --y) {
        // Alternate scanning direction each row for more natural spreading
        if (y % 2 == 0) {
            // Scan left to right
            for (int x = 0; x < GRID_WIDTH; ++x) {
                if (GRID[x][y].type == WATER) {
                    if (y + 1 >= GRID_HEIGHT || GRID[x][y + 1].type != EMPTY) {
                        // Try to spread randomly left or right first
                        
                        int direction = (rand() % 2) * 2 - 1; // -1 or 1
                        
                        // First direction
                        int newX = x + direction;
                        if (newX >= 0 && newX < GRID_WIDTH && GRID[newX][y].type == EMPTY) {
                            GRID[newX][y] = GRID[x][y];
                            GRID[x][y] = emptyPixel;
                            continue;
                        }
                        
                        // Try opposite direction
                        newX = x - direction;
                        if (newX >= 0 && newX < GRID_WIDTH && GRID[newX][y].type == EMPTY) {
                            GRID[newX][y] = GRID[x][y];
                            GRID[x][y] = emptyPixel;
                        }
                        
                        // Try diagonal movement if horizontal movement wasn't possible
                        if (y + 1 < GRID_HEIGHT) {
                            if (x + 1 < GRID_WIDTH && GRID[x + 1][y + 1].type == EMPTY) {
                                GRID[x + 1][y + 1] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                            } 
                            else if (x - 1 >= 0 && GRID[x - 1][y + 1].type == EMPTY) {
                                GRID[x - 1][y + 1] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                            }
                        }
                    }
                }
            }
        } 
        else {
            // Scan right to left
            for (int x = GRID_WIDTH - 1; x >= 0; --x) {
                if (GRID[x][y].type == WATER) {
                    if (y + 1 >= GRID_HEIGHT || GRID[x][y + 1].type != EMPTY) {
                        
                        int direction = (rand() % 2) * 2 - 1;

                        int newX = x + direction;
                        if (newX >= 0 && newX < GRID_WIDTH && GRID[newX][y].type == EMPTY) {
                            GRID[newX][y] = GRID[x][y];
                            GRID[x][y] = emptyPixel;
                            continue;
                        }
                        
                        newX = x - direction;
                        if (newX >= 0 && newX < GRID_WIDTH && GRID[newX][y].type == EMPTY) {
                            GRID[newX][y] = GRID[x][y];
                            GRID[x][y] = emptyPixel;
                        }
                        
                        if (y + 1 < GRID_HEIGHT) {
                            if (x + 1 < GRID_WIDTH && GRID[x + 1][y + 1].type == EMPTY) {
                                GRID[x + 1][y + 1] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                            } 
                            else if (x - 1 >= 0 && GRID[x - 1][y + 1].type == EMPTY) {
                                GRID[x - 1][y + 1] = GRID[x][y];
                                GRID[x][y] = emptyPixel;
                            }
                        }
                    }
                }
            }
        }
    }
}

void updateSand(Pixel GRID[SCREEN_WIDTH][SCREEN_HEIGHT], const Pixel emptyPixel, const Pixel waterPixel){

    for (int y = GRID_HEIGHT - 1; y >= 0; --y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            if (GRID[x][y].type == SAND) {
                // Try to move down
                if (GRID[x][y + 1].type == EMPTY) {
                    GRID[x][y + 1] = GRID[x][y];
                    GRID[x][y] = emptyPixel;
                }
                else if(GRID[x][y + 1].type == WATER){
                    GRID[x][y + 1] = GRID[x][y];
                    GRID[x][y] = waterPixel;
                }
            }
        }
    }

    for (int y = GRID_HEIGHT - 1; y >= 0; --y) {
        if (y % 2 == 0){
            for (int x = GRID_WIDTH -1; x >= 0; --x) { //right to left
                // Check if the current cell is not EMPTY
                if (GRID[x][y].type == SAND) {
                    if (y + 1 >= GRID_HEIGHT || GRID[x][y + 1].type != EMPTY) {
                        
                        if(x + 1 < GRID_WIDTH && x - 1 >= 0 && GRID[x-1][y+1].type == EMPTY && y != 61){ //move the block left
                            GRID[x-1][y+1] = GRID[x][y]; // Move the block down
                            GRID[x][y] = emptyPixel;    // Set current cell to EMPTY
                        }
                        else if(x + 1 < GRID_WIDTH && x + 1 >= 0 && GRID[x+1][y+1].type == EMPTY && y != 61){ //move the block right
                            GRID[x+1][y+1] = GRID[x][y]; // Move the block down
                            GRID[x][y] = emptyPixel;    // Set current cell to EMPTY
                        }
                    } 
                }
            }
        }
        else{
            for (int x = 0; x < GRID_WIDTH; ++x) { //right to left
                if (GRID[x][y].type == SAND) {

                    if (y + 1 >= GRID_HEIGHT || GRID[x][y + 1].type != EMPTY) {

                        if(x + 1 < GRID_WIDTH && x + 1 >= 0 && GRID[x+1][y+1].type == EMPTY && y != 61){ //move the block right
                            GRID[x+1][y+1] = GRID[x][y]; // Move the block down
                            GRID[x][y] = emptyPixel;    // Set current cell to EMPTY
                        }
                        else if(x + 1 < GRID_WIDTH && x - 1 >= 0 && GRID[x-1][y+1].type == EMPTY && y != 61){ //move the block left
                            GRID[x-1][y+1] = GRID[x][y]; // Move the block down
                            GRID[x][y] = emptyPixel;    // Set current cell to EMPTY
                        }

                    } 

                }
            }
        }
    }
}


void dropperSize(const Pixel pixelType, int mouseX, int mouseY, int sizeOfDropping){

    if (mouseX >= 0 && mouseX < SCREEN_WIDTH && mouseY >= 0 && mouseY < SCREEN_HEIGHT){                  
        for (int dx = 0; dx < sizeOfDropping; dx++)
        {
            int commonality = (rand() % 2) * 2 - 1; // -1 or 1
            int changeInX = mouseX;
            changeInX+= commonality; 

            GRID[changeInX+dx][mouseY].type = pixelType.type; 
            GRID[changeInX+dx][mouseY].color = pixelType.color; 
            changeInX+= commonality; 
        } 
    }   
}

void randSand(int randSandNum, int *s1, int *s2, int *s3) {
    if (randSandNum == 1) {
        *s1 = 234; *s2 = 225; *s3 = 176;
    }
    else if (randSandNum == 2) {
        *s1 = 229; *s2 = 216; *s3 = 144;
    }
    else if (randSandNum == 3) {
        *s1 = 195; *s2 = 184; *s3 = 124;
    }
    else if (randSandNum == 4) {
        *s1 = 255; *s2 = 200; *s3 = 20;
    }
    else {
        *s1 = 207; *s2 = 224; *s3 = 227;
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
            SDL_Event event; // Event handler

            // sand color
            int s1, s2, s3; 


            srand(time(NULL));
            bool pressed = false;
            int mouseX = 0, mouseY = 0;  // Tracks the mouse's current position
            int mode = 0; char modePresented[32]; //which substance
            int sizeOfDropping = 1; 



            Pixel waterPixel = {WATER, 0, 25, false, {15, 94, 156, 255}};
            Pixel emptyPixel = {EMPTY, 0, 0, false, {0, 0, 0, 255}};

            // set all pixels to empty to begin with
            for (int y = 0; y < GRID_HEIGHT; y++) {
                for (int x = 0; x < GRID_WIDTH; x++) {
                    GRID[x][y].type = emptyPixel.type;
                }
            }

            
            while (!quit) {
                while (SDL_PollEvent(&event) != 0) { // Handle events

                    //place controls below...
                    if (event.type == SDL_QUIT) quit = 1; // User requests quit
                    if (event.type == SDL_KEYDOWN){
                        if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1; // Exit on pressing the escape key
                        // these control which substance
                        if (event.key.keysym.sym == SDLK_RIGHT) mode+=1;
                        if (event.key.keysym.sym == SDLK_LEFT) mode-=1;

                        if (event.key.keysym.sym == SDLK_UP) sizeOfDropping+=1;
                        if (event.key.keysym.sym == SDLK_DOWN && sizeOfDropping-1 > 0) sizeOfDropping-=1;
                        if (event.key.keysym.sym == SDLK_c){
                            for (int y = 0; y < GRID_HEIGHT; y++) {
                                for (int x = 0; x < GRID_WIDTH; x++) {
                                    GRID[x][y].type = emptyPixel.type;
                                }
                            }
                        }
                        
                    }


                    if (event.type == SDL_MOUSEBUTTONDOWN){
                        if (event.button.button == SDL_BUTTON_LEFT)
                        {
                            pressed = true; 

                        }
                        
                    }

                    if (event.type == SDL_MOUSEBUTTONUP){
                         if (event.button.button == SDL_BUTTON_LEFT)
                        {
                            pressed = false; 
                        }
                    }

                    if (event.type == SDL_MOUSEMOTION){
                        mouseX = event.motion.x / PIXEL_SIZE;
                        mouseY = event.motion.y / PIXEL_SIZE;
                        
                    }

                }

                RGB color = get_next_color();
                Pixel rainbowPixel = {RAINBOW, 0, 25, false, {color.r, color.g, color.b, 255}};
                
                int s1, s2, s3; 
                int randSandNum = rand() % 4 + 1;
                randSand(randSandNum, &s1, &s2, &s3); 

                Pixel sandPixel = {SAND, 0, 25, false, {s1, s2, s3, 255}};



                
                // Clear screen with grey background
                SDL_SetRenderDrawColor(gRenderer, 110, 110, 110, 255);
                SDL_RenderClear(gRenderer); 

                //add the different substances here (this is where the grid will be inichalized with each pixel)
                if (pressed)
                {
                    if (mode == 1) dropperSize(sandPixel, mouseX, mouseY, sizeOfDropping);                    
                    if (mode == 2) dropperSize(waterPixel, mouseX, mouseY, sizeOfDropping);       
                    if (mode == 3) dropperSize(rainbowPixel, mouseX, mouseY, sizeOfDropping); 
                 
                }

                sprintf(modePresented, "Mode: %d", mode); 
                loadFromRenderedText(&modeTextTexture, modePresented, textColor);
                sprintf(modePresented, "Dropper Size: %d", sizeOfDropping); 
                loadFromRenderedText(&SizeOfDropperTexture, modePresented, textColor);

                update_water(GRID, emptyPixel);  // Handle all water movement
                updateSand(GRID, emptyPixel, waterPixel);
                
                for (int y = GRID_HEIGHT - 1; y >= 0; --y) {
                    for (int x = GRID_WIDTH -1; x >= 0; --x) {
                        // Check if the current cell is not EMPTY

                        if (GRID[x][y].type == RAINBOW) {
                            // Ensure we're not at the bottom row
                            if (y + 1 < GRID_HEIGHT && GRID[x][y + 1].type == EMPTY) {
                                GRID[x][y + 1] = GRID[x][y]; // Move the block down
                                GRID[x][y] = emptyPixel;    // Set current cell to EMPTY
                            }
                            else if(x + 1 < GRID_WIDTH && x - 1 >= 0 && GRID[x-1][y+1].type == EMPTY && y != 61){ //move the block left
                                GRID[x-1][y+1] = GRID[x][y]; // Move the block down
                                GRID[x][y] = emptyPixel;    // Set current cell to EMPTY
                            }
                            else if(x + 1 < GRID_WIDTH && x + 1 >= 0 && GRID[x+1][y+1].type == EMPTY && y != 61){ //move the block right
                                GRID[x+1][y+1] = GRID[x][y]; // Move the block down
                                GRID[x][y] = emptyPixel;    // Set current cell to EMPTY
                            }
                        }
                    }
                }

                
                for (int y = 0; y < GRID_HEIGHT; y++){
                    for (int x = 0; x < GRID_WIDTH; x++){
                        Pixel pixelRect = GRID[x][y]; 
                        
                        if (GRID[x][y].type != EMPTY){
                        
                            pixelRect.type = sandPixel.type;
                            setPixel(pixelRect, x, y);
                        }                         
                    }
                } 

                //this is for text
                renderTexture(&modeTextTexture, 0,0, NULL, 0, NULL, SDL_FLIP_NONE); //this is for text (dk, posx, posy, dk, dk, dk,dk); 
                renderTexture(&SizeOfDropperTexture, 80,0, NULL, 0, NULL, SDL_FLIP_NONE); //this is for text (dk, posx, posy, dk, dk, dk,dk); 
                SDL_RenderPresent(gRenderer); // Update screen
            }
        }
    }
    close(); // Free resources and close SDL

    return 0;
}

