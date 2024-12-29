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
#define SCREEN_WIDTH 1392
#define SCREEN_HEIGHT 744

#define MAX_BALLS 1500

// Struct for storing circle data
typedef struct {
    float x, y;
} Vector2;

typedef struct {
    int r; // Red component
    int g; // Green component
    int b; // Blue component
    int a; // Alpha component
} Color;

typedef struct {
    Vector2 position;        // Position
    Vector2 velocity;      // Velocity
    float mass; // mass
    float radius;      // Radius
    Color colour;
} Circle;

Circle circles[MAX_BALLS]; // Declare the array
int DYNAMIC_CIRCLES = 1000;

const Color colors[] = {
    // Earthy Browns and Greens
    {139, 69, 19, 255},    // Saddle brown
    {160, 82, 45, 255},    // Sienna
    {85, 107, 47, 255},    // Dark olive green
    {107, 142, 35, 255},   // Olive drab
    {34, 139, 34, 255},    // Forest green

    // Water and Sky Blues
    {70, 130, 180, 255},   // Steel blue
    {30, 144, 255, 255},   // Dodger blue
    {0, 191, 255, 255},    // Deep sky blue
    {176, 224, 230, 255},  // Powder blue
    {100, 149, 237, 255},  // Cornflower blue

    // Natural Rock and Soil Shades
    {112, 128, 144, 255},  // Slate gray
    {119, 136, 153, 255},  // Light slate gray
    {210, 180, 140, 255},  // Tan
    {244, 164, 96, 255},   // Sand brown
    {205, 133, 63, 255}    // Peru
};

float mouseVX, mouseVY;
float deltaTime = 1.0f / 60.0f; // Assume a frame rate of 60 FPS (adjust if variable)

// Global or persistent variables to store mouse positions
int previousMouseX = 0, previousMouseY = 0;



SDL_Color textColor = {0, 0, 0}; // text color

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

        // Render the text to create a texture
        if (!loadFromRenderedText(&gTextTexture, " ", textColor)) {
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

// circle functions:
void DrawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY, float radius) {
    // Loop through the vertical positions (y-coordinates) of the circle from the top to the bottom
    for (int y = -radius; y <= radius; y++) {
        // Calculate the horizontal distance (half-width) at this y-level using the circle equation
        int dx = (int)sqrt(radius * radius - y * y);  // Could use a faster square root approximation

        // Draw a horizontal line across the diameter of the circle at the current y-level
        SDL_RenderDrawLine(renderer, centerX - dx, centerY + y, centerX + dx, centerY + y);
    }
}

// Add two vectors
Vector2 addVectors(Vector2 v1, Vector2 v2) {
    Vector2 result = {
        .x = v1.x + v2.x,
        .y = v1.y + v2.y
    };
    return result;
}


// Vector subtraction (v1 - v2)
Vector2 subtractVectors(Vector2 v1, Vector2 v2) {
    Vector2 result = {
        .x = v1.x - v2.x,
        .y = v1.y - v2.y
    };
    return result;
}

// Dot product (returns a scalar/float)
float dotProduct(Vector2 v1, Vector2 v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

// Scale a vector by a number
Vector2 scaleVector(Vector2 v, float scale) {
    Vector2 result = {
        .x = v.x * scale,
        .y = v.y * scale
    };
    return result;
}

// calculates the magnitude of a vector
float mag(Vector2 vel) {

    return sqrt((vel.x * vel.x) + (vel.y * vel.y));
}

// Set the magnitude of a vector
Vector2 setMagnitude(Vector2 v, float newMag) {
    // Calculate the current magnitude of the vector
    float currentMag = mag(v);

    // If the current magnitude is zero, return the original vector to avoid division by zero
    if (currentMag == 0) {
        return v;
    }

    // Calculate the scaling factor
    float scale = newMag / currentMag;

    // Scale the vector to the desired magnitude
    return scaleVector(v, scale);
}



void resolveCollision(Circle* b1, Circle* b2) {

    // this is the distance between the center of the 2 balls
    float dx = b2->position.x - b1->position.x;
    float dy = b2->position.y - b1->position.y;
    float dist = sqrt(dx * dx + dy * dy);

    // Check if the circles are colliding (i.e., distance is less than sum of radii)
    if (dist <= b1->radius + b2->radius) {

        // Calculate mass terms
        float totalMass = b1->mass + b2->mass;

        // save the actual velocities
        Vector2 SavedvelocityB1 = b1->velocity;
        Vector2 SavedvelocityB2 = b2->velocity;

        // numerator ball1
        float numeratorB1 = (2.0f * b2->mass) * (  dotProduct(  subtractVectors(b2->velocity,b1->velocity)  , subtractVectors(b2->position,b1->position) )  );
        // numerator ball2
        float numeratorB2 = (2.0f * b1->mass) * (  dotProduct(  subtractVectors(b1->velocity,b2->velocity)  , subtractVectors(b1->position,b2->position) )  );



        // denomenator
        float denomenator = totalMass * dist * dist; 

        // ball1
        b1->velocity = addVectors(SavedvelocityB1 , scaleVector( subtractVectors(b2->position , b1->position) ,  numeratorB1 / denomenator));
        // ball2
        b2->velocity = addVectors(SavedvelocityB2 , scaleVector( subtractVectors(b1->position , b2->position) ,  numeratorB2 / denomenator));

    }
}

bool checkOverlap(Circle* b1, Circle* b2){
    float dx = b2->position.x - b1->position.x;
    float dy = b2->position.y - b1->position.y;
    float dist = sqrt(dx * dx + dy * dy);
    return dist <= (b1->radius + b2->radius);
}

void InitializeCircles() {
    srand(time(NULL)); // Seed random number generator

    for (int i = 0; i < DYNAMIC_CIRCLES; i++) {
        int randColor = rand() % 15;
        // Random position within screen bounds (adjust based on your screen size)
        circles[i].position.x = rand() % (SCREEN_WIDTH - 100) + 50; // Keep circles away from edges
        circles[i].position.y = rand() % (SCREEN_HEIGHT - 100) + 50;

        // Random velocity components (from -5 to 5, but non-zero)
        circles[i].velocity.x = (rand() % 5) - 2; // -5 to 5
        circles[i].velocity.y = (rand() % 5) - 2;

        // Random radius (10 to 50)
        circles[i].radius = rand() % 11 + 10; // 10 to 50

        // Mass proportional to radius (scaling factor: 1.5 for example)
        circles[i].mass = circles[i].radius * 1.5;

        // Mass proportional to radius (scaling factor: 1.5 for example)
        circles[i].colour = colors[randColor]; 

        for (int j = 0; j < i; j++)
        {
            if (checkOverlap(&circles[i], &circles[j]))
            {
                i--; 
                break; 
            }

            
        }
        
        
    }
}


void correctPositions() {
    for (int i = 0; i < DYNAMIC_CIRCLES; i++) {
        for (int j = i + 1; j < DYNAMIC_CIRCLES; j++) {
            if (checkOverlap(&circles[i], &circles[j])) {
                float dx = circles[j].position.x - circles[i].position.x;
                float dy = circles[j].position.y - circles[i].position.y;
                float dist = sqrt(dx * dx + dy * dy);
                float overlap = (circles[i].radius + circles[j].radius) - dist;

                if (dist > 0) { // Avoid divide by zero
                    float moveX = (dx / dist) * (overlap / 2);
                    float moveY = (dy / dist) * (overlap / 2);

                    circles[i].position.x -= moveX;
                    circles[i].position.y -= moveY;
                    circles[j].position.x += moveX;
                    circles[j].position.y += moveY;
                }
            }
        }
    }
}


void updateMouseVelocity(float* mouseVX, float* mouseVY, float deltaTime , int currentMouseX, int currentMouseY) {

    // Calculate the velocity
    *mouseVX = (currentMouseX - previousMouseX) / deltaTime;
    *mouseVY = (currentMouseY - previousMouseY) / deltaTime;

    // Update the previous position for the next frame
    previousMouseX = currentMouseX;
    previousMouseY = currentMouseY;
}


void handleMouseCollision(int mouseX, int mouseY, float mouseVX, float mouseVY) {
    Circle mouseCircle;
    mouseCircle.position.x = mouseX;
    mouseCircle.position.y = mouseY;
    mouseCircle.velocity.x = mouseVX * 0.01;
    mouseCircle.velocity.y = mouseVY * 0.01;
    mouseCircle.radius = 10.0f; // Arbitrary large size
    mouseCircle.mass = 50.0f; // Very large mass

    for (int i = 0; i < DYNAMIC_CIRCLES; i++) {
        if (checkOverlap(&mouseCircle, &circles[i])) {
            resolveCollision(&mouseCircle, &circles[i]);
        }
    }
}

void applyDampening(float *xVel, float *yVel){

    if (rand() % 100 < 30)
    {
        *xVel *= 0.99;
        *yVel *= 0.99;

    }
    


}


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
            InitializeCircles();
            char ballNum[32];

            bool pressed = false; 

            int mouseX, mouseY;


            while (!quit) {

                // Get mouse position
                SDL_GetMouseState(&mouseX, &mouseY);

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


                    if (e.type == SDL_MOUSEBUTTONDOWN) {
                        if (e.button.button == SDL_BUTTON_RIGHT) {

                            //int mousePosX, mousePosY;
                            // Get mouse position
                            //SDL_GetMouseState(&mousePosX, &mousePosY);

                            int randColor = rand() % 15;

                            // Create a new circle
                            Circle newCircle;
                            newCircle.position.x = mouseX;
                            newCircle.position.y = mouseY;
                            newCircle.velocity.x = (rand() % 5) - 2; // Random velocity
                            newCircle.velocity.y = (rand() % 5) - 2;
                            newCircle.radius = rand() % 11 + 10;     // Random radius
                            newCircle.mass = newCircle.radius * 1.5; // Mass proportional to radius
                            newCircle.colour = colors[randColor];

                            // Add the circle to the array
                            circles[DYNAMIC_CIRCLES++] = newCircle;                        
                        }

                        if (e.button.button == SDL_BUTTON_LEFT) pressed = true;
                        
                    }

                    if (e.type == SDL_MOUSEBUTTONUP){
                        if (e.button.button == SDL_BUTTON_LEFT)
                        {
                            pressed = false; 
                        }
                    }

                    
                    if (e.type == SDL_MOUSEMOTION && pressed == true){


                       

                        // Handle collisions with the mouse as a circle
                        handleMouseCollision(mouseX, mouseY, mouseVX, mouseVY);
                    }

                    
                }
                // Clear screen with white background
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                SDL_RenderClear(gRenderer);    



                for (int i = 0; i < DYNAMIC_CIRCLES; i++) {
                    for (int j = i + 1; j < DYNAMIC_CIRCLES; j++) {

                        resolveCollision(&circles[i], &circles[j]);
                    }
                } 

                correctPositions();



                updateMouseVelocity(&mouseVX, &mouseVY, deltaTime, mouseX, mouseY);


                // Update and draw each circle
                for (int i = 0; i < DYNAMIC_CIRCLES; i++) {
                    // Update position based on velocity
                    circles[i].position.x += circles[i].velocity.x;
                    circles[i].position.y += circles[i].velocity.y;

                    // Handle boundaries
                    // right
                    if (circles[i].position.x >= SCREEN_WIDTH - circles[i].radius) {
                        circles[i].velocity.x *= -1;
                        circles[i].position.x = SCREEN_WIDTH - circles[i].radius; 
 
                    }
                    // left
                    else if (circles[i].position.x <= circles[i].radius)
                    {
                        circles[i].velocity.x *= -1;
                        circles[i].position.x = circles[i].radius; 
                    }
                    // bottom
                    else if (circles[i].position.y >= SCREEN_HEIGHT - circles[i].radius) { 
                        circles[i].velocity.y *= -1;
                        circles[i].position.y = SCREEN_HEIGHT - circles[i].radius; 

                    }
                    // top
                    else if (circles[i].position.y <= circles[i].radius)
                    {
                        circles[i].velocity.y *= -1;
                        circles[i].position.y = circles[i].radius; 
 
                    }

                    applyDampening(&circles[i].velocity.x, &circles[i].velocity.y);
                    
                    int randColor = rand() % 15;  // Random index (0 to 14)
                    SDL_SetRenderDrawColor(gRenderer, circles[i].colour.r, circles[i].colour.g, circles[i].colour.b, circles[i].colour.a);
                    // Draw the circle
                    DrawFilledCircle(gRenderer, circles[i].position.x, circles[i].position.y, circles[i].radius);
                }

                sprintf(ballNum, "Number of balls: %d", DYNAMIC_CIRCLES);
                loadFromRenderedText(&gTextTexture,ballNum, textColor);
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
