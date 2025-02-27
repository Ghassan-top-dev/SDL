// gcc -O3 -I src/include -L src/lib -o main rayCast.c -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define SCREEN_WIDTH 1392 // Screen dimension constants
#define SCREEN_HEIGHT 744 // the size of the screen
#define NUM_RAYS 360 // Number of points on the circle

#define MAX_BALLS 1000


typedef struct {
    float startX, startY, endX, endY;
} lines;

typedef struct {
    float x, y;
} Vector2;


typedef struct {
    Vector2 position;        // Position
    Vector2 velocity;      // Velocity
    float mass; // mass
    float radius;      // Radius
} Circle;

Circle circles[MAX_BALLS]; // Declare the array
int DYNAMIC_CIRCLES = 4;

// Texture wrapper structure to hold texture data and dimensions
typedef struct {
    SDL_Texture* texture;
    int width;
    int height;
} LTexture;

SDL_Color textColor = {255, 255, 255}; // text color

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
        gWindow = SDL_CreateWindow("Raycast", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

bool checkOverlap(Circle* b1, Circle* b2){
    float dx = b2->position.x - b1->position.x;
    float dy = b2->position.y - b1->position.y;
    float dist = sqrt(dx * dx + dy * dy);
    return dist <= (b1->radius + b2->radius);
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
        circles[i].radius = rand() % 81 + 10; // 10 to 50

        // Mass proportional to radius (scaling factor: 1.5 for example)
        circles[i].mass = circles[i].radius * 1.5;

        // Mass proportional to radius (scaling factor: 1.5 for example)

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

int rayIntersectsLine(float rayStartX, float rayStartY, float rayEndX, float rayEndY,
                      float lineStartX, float lineStartY, float lineEndX, float lineEndY,
                      float *intersectionX, float *intersectionY) {
    float dx = rayEndX - rayStartX;
    float dy = rayEndY - rayStartY;
    float sx = lineEndX - lineStartX;
    float sy = lineEndY - lineStartY;

    float denominator = dx * sy - dy * sx;

    // Check if lines are parallel
    if (fabs(denominator) < 1e-6) {
        return 0; // No intersection
    }

    float t = ((lineStartX - rayStartX) * sy - (lineStartY - rayStartY) * sx) / denominator;
    float u = ((lineStartX - rayStartX) * dy - (lineStartY - rayStartY) * dx) / denominator;

    // Check if the intersection is valid
    if (t >= 0 && u >= 0 && u <= 1) {
        *intersectionX = rayStartX + t * dx;
        *intersectionY = rayStartY + t * dy;
        return 1; // Intersection found
    }

    return 0; // No valid intersection
}

int checkPreBuilt(float rayStartX, float rayStartY, float *rayEndX, float *rayEndY){ // I will need to refactor this
    float dx = *rayEndX - rayStartX;
    float dy = *rayEndY - rayStartY;

    // boundaries
    lines line[4];
    line[0].startX = 0; line[0].startY = 0; line[0].endX = SCREEN_WIDTH; line[0].endY = 0; // top
    line[1].startX = 0; line[1].startY = SCREEN_HEIGHT; line[1].endX = SCREEN_WIDTH; line[1].endY = SCREEN_HEIGHT; // bottom
    line[2].startX = 0; line[2].startY = 0; line[2].endX = 0; line[2].endY = SCREEN_HEIGHT; // left
    line[3].startX = SCREEN_WIDTH; line[3].startY = 0; line[3].endX = SCREEN_WIDTH; line[3].endY = SCREEN_HEIGHT; // right

    for (int i = 0; i < 4; i++)
    {
        float sx = line[i].endX - line[i].startX;
        float sy = line[i].endY - line[i].startY;
        float denominator = dx * sy - dy * sx;

        // Check if lines are parallel
        if (fabs(denominator) < 1e-6) {
            continue;
        }

        float t = ((line[i].startX - rayStartX) * sy - (line[i].startY - rayStartY) * sx) / denominator;
        float u = ((line[i].startX - rayStartX) * dy - (line[i].startY - rayStartY) * dx) / denominator;

        // Check if the intersection is valid
        if (t >= 0 && u >= 0 && u <= 1) {
            *rayEndX = rayStartX + t * dx;
            *rayEndY = rayStartY + t * dy;
            return 1; // Intersection found
        } 
    }
    return 0; 

}

int RayIntersectsCircle(float rayStartX, float rayStartY, float rayEndX, float rayEndY,
                        float circleX, float circleY, float radius, 
                        float *intersectionX, float *intersectionY) {
    float dx = rayEndX - rayStartX;
    float dy = rayEndY - rayStartY;

    // Quadratic coefficients
    float A = dx * dx + dy * dy;
    float B = 2 * (dx * (rayStartX - circleX) + dy * (rayStartY - circleY));
    float C = (rayStartX - circleX) * (rayStartX - circleX) + 
              (rayStartY - circleY) * (rayStartY - circleY) - radius * radius;

    // Discriminant
    float discriminant = B * B - 4 * A * C;

    if (discriminant < 0) {
        return 0; // No intersection
    }

    // Compute the two possible values of t
    float sqrtDiscriminant = sqrt(discriminant);
    float t1 = (-B - sqrtDiscriminant) / (2 * A);
    float t2 = (-B + sqrtDiscriminant) / (2 * A);

    // Check for the smallest positive t (valid intersection)
    float t = (t1 >= 0) ? t1 : ((t2 >= 0) ? t2 : -1);
    if (t < 0) {
        return 0; // No valid intersection
    }

    // Calculate the intersection point
    *intersectionX = rayStartX + t * dx;
    *intersectionY = rayStartY + t * dy;

    return 1; // Intersection found
}


void DrawLightFromOutline(SDL_Renderer* renderer, SDL_Point rayEndpoints[], SDL_Point rayStarts[]) {
    SDL_Vertex vertices[NUM_RAYS * 2]; // Two vertices for each ray: start and end

    // Create vertices for the light effect
    for (int i = 0; i < NUM_RAYS; i++) {
        // Ray start point (on the circle outline)
        vertices[i * 2].position.x = rayStarts[i].x;
        vertices[i * 2].position.y = rayStarts[i].y;
        vertices[i * 2].color = (SDL_Color){255, 255, 100, 200}; // Light yellow

        // Ray end point
        vertices[i * 2 + 1].position.x = rayEndpoints[i].x;
        vertices[i * 2 + 1].position.y = rayEndpoints[i].y;
        vertices[i * 2 + 1].color = (SDL_Color){255, 255, 100, 0}; // Fading light
    }

    // Define indices for triangles
    int indices[(NUM_RAYS - 1) * 3]; // Each triangle needs 3 indices
    for (int i = 0; i < NUM_RAYS - 1; i++) {
        indices[i * 3] = i * 2;         // Start of ray i
        indices[i * 3 + 1] = i * 2 + 1; // End of ray i
        indices[i * 3 + 2] = i * 2 + 3; // End of ray i+1
    }

    // Draw geometry
    SDL_RenderGeometry(renderer, NULL, vertices, NUM_RAYS * 2, indices, (NUM_RAYS - 1) * 3);
}



// main function
int main(int argc, char* args[]) {
    
    if (!init()) {
        printf("Failed to initialize!\n");
    } else {
        if (!loadMedia()) {
            printf("Failed to load media!\n");
        } else {
            int quit = 0;
            SDL_Event event;
            
            Circle lightCircle;
            lightCircle.position.x = 350; lightCircle.position.y = 400; lightCircle.radius = 80; 
            
            SDL_Point rayStartPoints[NUM_RAYS]; 
            SDL_Point rayEndPoints[NUM_RAYS]; 


            InitializeCircles();

            while (!quit) {
                while (SDL_PollEvent(&event) != 0) {
                    // controls
                    if (event.type == SDL_QUIT) quit = 1;
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
                        if (event.key.keysym.sym == SDLK_c) {};

                        // mode for which substance will be dropped
                        if (event.key.keysym.sym == SDLK_RIGHT){};
                        if (event.key.keysym.sym == SDLK_LEFT){};

                        // this changes the size of the dropper
                        if (event.key.keysym.sym == SDLK_UP) {};
                        if (event.key.keysym.sym == SDLK_DOWN){};

                    }

                    if (event.type == SDL_MOUSEBUTTONDOWN) {
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            
                        }
                    }

                    if (event.type == SDL_MOUSEBUTTONUP) {
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            
                        }
                    }

                }

                // Clear screen with grey background color
                SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
                SDL_RenderClear(gRenderer);

                
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

                    
                    SDL_SetRenderDrawColor(gRenderer, 246, 196, 31, 255);
                    // Draw the circle
                    DrawFilledCircle(gRenderer, circles[i].position.x, circles[i].position.y, circles[i].radius);
                    
                    DrawFilledCircle(gRenderer, lightCircle.position.x, lightCircle.position.y, lightCircle.radius);


                }

                for (int j = 0; j < NUM_RAYS; j++) {
                    // Calculate the angle for this ray
                    float angle = j * (2 * M_PI / NUM_RAYS);

                    // Calculate the starting point of the ray
                    float startX = lightCircle.position.x + lightCircle.radius * cos(angle);
                    float startY = lightCircle.position.y + lightCircle.radius * sin(angle);

                    // Initialize the end point of the ray to be far away
                    float endX = startX + 5000 * cos(angle);
                    float endY = startY + 5000 * sin(angle);

                    float nearestX = endX, nearestY = endY; // Store nearest intersection point
                    int foundIntersection = 0; // Flag to indicate intersection occurred

                    for (int i = 0; i < DYNAMIC_CIRCLES; i++) {
                        float intersectionX, intersectionY;

                        if (RayIntersectsCircle(startX, startY, endX, endY,
                                                circles[i].position.x, circles[i].position.y, circles[i].radius,
                                                &intersectionX, &intersectionY)) {
                            
                            foundIntersection = 1;

                            // Calculate distance to this intersection
                            float distToIntersection = sqrt(pow(intersectionX - startX, 2) + pow(intersectionY - startY, 2));
                            float distToNearest = sqrt(pow(nearestX - startX, 2) + pow(nearestY - startY, 2));

                            // Update nearest intersection point if closer
                            if (distToIntersection < distToNearest) {
                                nearestX = intersectionX;
                                nearestY = intersectionY;
                            }

                        }
                    }

                    // Draw the ray to the nearest intersection point or its full length
                    // SDL_RenderDrawLine(gRenderer, (int)startX, (int)startY, (int)nearestX, (int)nearestY);

                    rayStartPoints[j].x = (int)startX; rayStartPoints[j].y = (int)startY;
                    rayEndPoints[j].x = (int)nearestX; rayEndPoints[j].y = (int)nearestY;


                        
                }

                DrawLightFromOutline(gRenderer, rayEndPoints, rayStartPoints);

                
                
                

                //this is for text
                renderTexture(&modeTextTexture, 0,0, NULL, 0, NULL, SDL_FLIP_NONE); 
                SDL_RenderPresent(gRenderer); // Update screen

                // Optional: Add a small delay to control simulation speed
                SDL_Delay(16);

            }
        }
    }
    close();
    return 0;
}
