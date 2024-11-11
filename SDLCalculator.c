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
const int BUTTON_MID_X = 35;
const int BUTTON_MID_Y = 20;

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
char whichButtonWasPressed(int buttonX, int buttonY); 


// Global variables for the SDL window, renderer, font, and text texture
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;
LTexture gTextTexture; // Texture to display text
LTexture inputLine; // Texture to display text
SDL_Color textColor = {0, 0, 0}; // Black text color

// LTexture buttonText[NUM_BUTTONS]; // Texture to display text

LTexture allOfTheButtonsTextTextures[NUM_BUTTONS];  // Array to store text textures
const char* eachText[NUM_BUTTONS] = {
    "X", "0", ".", "1", "2", 
    "3", "4", "5", "6", "7",
    "8", "9", "+", "-", "*", 
    "%", "CE", "/"
};



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
    gFont = TTF_OpenFont("/Users/ghassanmuradagha/Documents/pro/fonts/open-sans/OpenSans-Bold.ttf", 20); //font size
    if (gFont == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        success = false;
    } else {

        // Render the text to create a texture
        if (!loadFromRenderedText(&gTextTexture, " ", textColor)) {
            printf("Failed to render text texture!\n");
            success = false;
        }

        if (!loadFromRenderedText(&inputLine, " ", textColor)) {
            printf("Failed to render text texture!\n");
            success = false;
        }

        for (int i = 0; i < NUM_BUTTONS; i++) {
            if (!loadFromRenderedText(&allOfTheButtonsTextTextures[i], eachText[i], textColor)) {
                printf("Failed to render text texture!\n");
                success = false;
            }
        }


    }
    return success;
}

// Frees up resources and shuts down SDL libraries
void close() {
    freeTexture(&gTextTexture); // Free text texture
    freeTexture(&inputLine); // Free text texture


    for (int i = 0; i < NUM_BUTTONS; i++) {
        freeTexture(&allOfTheButtonsTextTextures[i]); // Free text texture
    }

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

void grid(){ //line grid
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


char whichButtonWasPressed(int buttonX, int buttonY) {
    // Row 1 (Y = 390, Characters: "X", "0", ".")
    if (buttonY >= 390 && buttonY < 450) {
        if (buttonX >= 0 && buttonX < 83) {
            return 'X';
        } else if (buttonX >= 83 && buttonX < 166) {
            return '0';
        } else if (buttonX >= 166 && buttonX < 249) {
            return '.';
        }
    }
    
    // Row 2 (Y = 330, Characters: "1", "2", "3")
    else if (buttonY >= 330 && buttonY < 390) {
        if (buttonX >= 0 && buttonX < 83) {
            return '1';
        } else if (buttonX >= 83 && buttonX < 166) {
            return '2';
        } else if (buttonX >= 166 && buttonX < 249) {
            return '3';
        }
    }
    
    // Row 3 (Y = 270, Characters: "4", "5", "6")
    else if (buttonY >= 270 && buttonY < 330) {
        if (buttonX >= 0 && buttonX < 83) {
            return '4';
        } else if (buttonX >= 83 && buttonX < 166) {
            return '5';
        } else if (buttonX >= 166 && buttonX < 249) {
            return '6';
        }
    }
    
    // Row 4 (Y = 210, Characters: "7", "8", "9")
    else if (buttonY >= 210 && buttonY < 270) {
        if (buttonX >= 0 && buttonX < 83) {
            return '7';
        } else if (buttonX >= 83 && buttonX < 166) {
            return '8';
        } else if (buttonX >= 166 && buttonX < 249) {
            return '9';
        }
    }
    
    // Row 5 (Y = 150, Characters: "+", "-", "*")
    else if (buttonY >= 150 && buttonY < 210) {
        if (buttonX >= 0 && buttonX < 83) {
            return '+';
        } else if (buttonX >= 83 && buttonX < 166) {
            return '-';
        } else if (buttonX >= 166 && buttonX < 249) {
            return '*';
        }
    }
    
    // Row 6 (Y = 90, Characters: "%", "CE", "/")
    else if (buttonY >= 90 && buttonY < 150) {
        if (buttonX >= 0 && buttonX < 83) {
            return '%';
        } else if (buttonX >= 83 && buttonX < 166) {
            return 'C'; // Assuming 'CE' can be returned as 'C'
        } else if (buttonX >= 166 && buttonX < 249) {
            return '/';
        }
    }

    // If no button was pressed
    return -1;
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

            SDL_Rect buttons[NUM_BUTTONS]; 
            int spacerHor = 0; //testing for now
            int spacerVer = 60; 
            int exactButton = 0; //this is stupid but this gives memory to the inner for loop
            int holder[2][18], counterForHolderRow = 0, counterForHolderCol = 0; //This will hold the data for all buttons

            for (int c = 0; c < 6; c++)
            {
                for (int i = 0; i < 3; i++)
                {
                    buttons[i+exactButton].w = BUTTON_WIDTH; 
                    buttons[i+exactButton].h = BUTTON_HEIGHT; 
                
                    buttons[i+exactButton].y = SCREEN_HEIGHT - spacerVer; 
                    holder[counterForHolderRow][counterForHolderCol] = buttons[i+exactButton].y; 
                    counterForHolderRow++; 
                    
                    buttons[i+exactButton].x = spacerHor; 
                    holder[counterForHolderRow][counterForHolderCol] = buttons[i+exactButton].x; 
                    
                    counterForHolderRow = 0; 
                    counterForHolderCol++; 

                    spacerHor += 83; 
                }
                spacerHor = 0;
                spacerVer+=60;
                exactButton+=3;
                
            }



            


            
            int mouseX = 0, mouseY = 0; //used for mouse hovering
            while (!quit) {
                char positionText[50];
                sprintf(positionText, "(%d, %d)", mouseX, mouseY); 


                while (SDL_PollEvent(&event) != 0) { // Handle events

                    if (event.type == SDL_QUIT) quit = 1; // User requests quit
                    

                    if (event.type == SDL_KEYDOWN) {
                      if (event.key.keysym.sym == SDLK_ESCAPE) quit = true; // Exit on pressing the escape key
                    }

                    //mouse stuff
                    switch (event.type) {
                        case SDL_MOUSEBUTTONDOWN:
                            if (event.button.button == SDL_BUTTON_LEFT) {
                                printf("Left button pressed at (%d, %d)\n", event.button.x, event.button.y);
                                mouseX = event.button.x;
                                mouseY = event.button.y;

                                char button = whichButtonWasPressed(event.button.x, event.button.y);

                                if (button != -1) { // Check if a valid button was pressed
                                    char text[2] = { button, '\0' }; // Convert char to string format
                                    loadFromRenderedText(&inputLine, text, textColor);
                                }





                            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                                printf("Right button pressed at (%d, %d)\n", event.button.x, event.button.y);
                            }
                            break;
                        case SDL_MOUSEBUTTONUP:
                            // mouseColorR = mouseColorG = mouseColorB = 0; // Reset color on release
                            // printf("Mouse button released at (%d, %d)\n", event.button.x, event.button.y);
                            break;
                        case SDL_MOUSEMOTION:
                            mouseX = event.motion.x;
                            mouseY = event.motion.y;
                            printf("Mouse moved to (%d, %d)\n", mouseX, mouseY);
                            loadFromRenderedText(&gTextTexture, positionText, textColor);
                            break;
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
                renderTexture(&inputLine, 0,20, NULL, 0, NULL, SDL_FLIP_NONE); //this is for text (dk, posx, posy, dw, dw, dw,dw); 

                // renderTexture(&buttonText[0], (buttons[0].x) + BUTTON_MID_X,(buttons[0].y) + BUTTON_MID_Y, NULL, 0, NULL, SDL_FLIP_NONE); //this is for text (dk, posx, posy, dw, dw, dw,dw); 

                for (int i = 0; i < NUM_BUTTONS; i++) {
                    SDL_Rect quack = {buttons[i].x + BUTTON_MID_X, buttons[i].y + BUTTON_MID_Y, allOfTheButtonsTextTextures[i].width, allOfTheButtonsTextTextures[i].height};
                    renderTexture(&allOfTheButtonsTextTextures[i], quack.x, quack.y, NULL, 0, NULL, SDL_FLIP_NONE);
                }


                SDL_RenderPresent(gRenderer); // Update screen
            }
        }
    }
    close(); // Free resources and close SDL

    return 0;
}
