// gcc -O3 -I src/include -L src/lib -o main allTogether.c -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer


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
#define NUM_RAYS 2 // Number of points on the circle

#define MAX_BALLS 1000


typedef struct {
    float startX, startY, endX, endY;
} lines;

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
int DYNAMIC_CIRCLES = 10;


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
    // Center the mouse circle properly
    mouseCircle.position.x = mouseX;
    mouseCircle.position.y = mouseY;
    
    // Adjust velocity scaling based on testing
    const float velocityScale = 0.05f;  // Adjusted from 0.01
    mouseCircle.velocity.x = mouseVX * velocityScale;
    mouseCircle.velocity.y = mouseVY * velocityScale;
    
    // Make radius and mass more reasonable
    mouseCircle.radius = 5.0f;  // Smaller radius for more precise collisions
    mouseCircle.mass = 20.0f;   // Adjusted mass for better collision response
    
    // Add some minimum velocity threshold to prevent tiny movements
    const float minVelocity = 0.1f;
    if (fabsf(mouseCircle.velocity.x) < minVelocity && fabsf(mouseCircle.velocity.y) < minVelocity) {
        return;
    }

    for (int i = 0; i < DYNAMIC_CIRCLES; i++) {
        if (checkOverlap(&mouseCircle, &circles[i])) {
            resolveCollision(&mouseCircle, &circles[i]);
            
            // Add minimum velocity after collision to prevent sticking
            const float minPostCollisionVel = 0.5f;
            if (fabsf(circles[i].velocity.x) < minPostCollisionVel) {
                circles[i].velocity.x *= minPostCollisionVel / fabsf(circles[i].velocity.x);
            }
            if (fabsf(circles[i].velocity.y) < minPostCollisionVel) {
                circles[i].velocity.y *= minPostCollisionVel / fabsf(circles[i].velocity.y);
            }
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
    float t = (t2 >= 0) ? t2 : ((t1 >= 0) ? t1 : -1);
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

// Helper function to convert degrees to radians
double deg_to_rad(double degrees) {
    return degrees * M_PI / 180.0;
}

// Helper function to calculate angle between two points
double get_angle(int x1, int y1, int x2, int y2) {
    return atan2(y2 - y1, x2 - x1) * 180.0 / M_PI;
}

void drawThickerLine(SDL_Renderer* renderer, 
                     int x1, int y1, int x2, int y2, int thickness) {
    // Calculate the vector of the original line
    float dx = x2 - x1;
    float dy = y2 - y1;

    // Normalize the perpendicular vector
    float length = sqrtf(dx * dx + dy * dy);
    float perp_dx = -dy / length;
    float perp_dy = dx / length;

    // Scale the perpendicular vector by half the desired thickness
    float offset = thickness / 2.0;

    // Compute offset points
    int offset_x1 = x1 + (int)(perp_dx * offset);
    int offset_y1 = y1 + (int)(perp_dy * offset);
    int offset_x2 = x2 + (int)(perp_dx * offset);
    int offset_y2 = y2 + (int)(perp_dy * offset);

    int offset_neg_x1 = x1 - (int)(perp_dx * offset);
    int offset_neg_y1 = y1 - (int)(perp_dy * offset);
    int offset_neg_x2 = x2 - (int)(perp_dx * offset);
    int offset_neg_y2 = y2 - (int)(perp_dy * offset);

    // Draw the original line and the offset lines
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    SDL_RenderDrawLine(renderer, offset_x1, offset_y1, offset_x2, offset_y2);
    SDL_RenderDrawLine(renderer, offset_neg_x1, offset_neg_y1, offset_neg_x2, offset_neg_y2);
}


void drawShadow(SDL_Renderer* renderer, int center_x, int center_y, int end_x, int end_y, int start_x, int start_y, int radius, int intersectionOfLineRayX1, int intersectionOfLineRayY1, int intersectionOfLineRayX2, int intersectionOfLineRayY2, int endXRay1, int endYRay1, int endXRay2, int endYRay2) {
    
    
    // Calculate start and end angles
    double start_angle = get_angle(center_x, center_y, start_x, start_y);
    double end_angle = get_angle(center_x, center_y, end_x, end_y);
    
    // Ensure end angle is greater than start angle
    if (end_angle < start_angle) end_angle += 360; 


    // Number of segments to draw (more segments = smoother arc)
    #define segments 100
    double angle_step = (end_angle - start_angle) / segments;

    Vector2 arcPoints[segments + 1]; 

    // Variables to store the previous point
    int prev_x = 0;
    int prev_y = 0;

    float adjustedRadius = radius + 1; 

    for (int i = 0; i <= segments; i++) {
        double angle = deg_to_rad(start_angle + (i * angle_step));
        int x = center_x + (int)(adjustedRadius * cos(angle));
        int y = center_y + (int)(adjustedRadius * sin(angle));


        arcPoints[i].x = x;
        arcPoints[i].y = y;

        // Store current point as previous
        prev_x = x;
        prev_y = y;
    }


  // Calculate other points to draw the lines from (0.90 length)
    int dx_TangentPointToLine = intersectionOfLineRayX1 - start_x;
    int dy_TangentPointToLine = intersectionOfLineRayY1 - start_y;

    float newEndTangentPointToLine_x = start_x + dx_TangentPointToLine * 0.90;
    float newEndTangentPointToLine_Y = start_y + dy_TangentPointToLine * 0.90;

    // Calculate other points to draw the lines from (0.95 length)
    float newEndTangentPointToLine_95_x = start_x + dx_TangentPointToLine * 0.95;
    float newEndTangentPointToLine_95_Y = start_y + dy_TangentPointToLine * 0.95;

    // Calculate other points to draw the lines from (0.90 length)
    int dx_CenterOfCircleToLine = intersectionOfLineRayX1 - end_x;
    int dy_CenterOfCircleToLine = intersectionOfLineRayY1 - end_y;

    float CenterOfCircleToLine_x = end_x + dx_CenterOfCircleToLine * 0.90;
    float CenterOfCircleToLine_y = end_y + dy_CenterOfCircleToLine * 0.90;

    // Calculate other points to draw the lines from (0.95 length)
    float CenterOfCircleToLine_95_x = end_x + dx_CenterOfCircleToLine * 0.95;
    float CenterOfCircleToLine_95_y = end_y + dy_CenterOfCircleToLine * 0.95;

    for (int i = 0; i < segments; i++) {
        // Original line
        SDL_RenderDrawLine(renderer, intersectionOfLineRayX1, intersectionOfLineRayY1, arcPoints[i].x, arcPoints[i].y); 

        // 0.90 scaled tangent point
        SDL_RenderDrawLine(renderer, newEndTangentPointToLine_x, newEndTangentPointToLine_Y, arcPoints[i].x, arcPoints[i].y); 

        // 0.90 scaled center perpendicular line point
        SDL_RenderDrawLine(renderer, CenterOfCircleToLine_x, CenterOfCircleToLine_y, arcPoints[i].x, arcPoints[i].y); 

        // 0.95 scaled tangent point
        SDL_RenderDrawLine(renderer, newEndTangentPointToLine_95_x, newEndTangentPointToLine_95_Y, arcPoints[i].x, arcPoints[i].y); 

        // 0.95 scaled center perpendicular line point
        SDL_RenderDrawLine(renderer, CenterOfCircleToLine_95_x, CenterOfCircleToLine_95_y, arcPoints[i].x, arcPoints[i].y); 
    }


    drawThickerLine(renderer, 
                    intersectionOfLineRayX1, intersectionOfLineRayY1, 
                    intersectionOfLineRayX2, intersectionOfLineRayY2, 
                    5);


    // Vertices for a square
    SDL_Vertex vertices[4] = {
        {{intersectionOfLineRayX1, intersectionOfLineRayY1},     {0, 0, 0, 255}},    // Top left
        {{intersectionOfLineRayX2, intersectionOfLineRayY2},   {0, 0, 0, 255}},    // Top right
        {{endXRay1, endYRay1}, {0, 0, 0, 255}},    // Bottom right
        {{endXRay2, endYRay2},   {0, 0, 0, 255}}     // Bottom left
    };



    // Indices to make two triangles
    int indices[6] = {
        0, 2, 1,  // First triangle: left-high to right-high to left-low
        1, 2, 3   // Second triangle: left-low to right-high to right-low
    };

    // Draw the square (made of two triangles)
    SDL_RenderGeometry(renderer, NULL, vertices, 4, indices, 6);


}


// Function to draw a perpendicular line
void DrawPerpendicularLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, int px, int py, int length, int *perp_x1, int *perp_y1, int *perp_x2, int *perp_y2) {
    // Calculate slope of the original line
    double dx = x2 - x1;
    double dy = y2 - y1;
    double original_slope = dy / dx;

    // Calculate perpendicular slope
    double perpendicular_slope = -1 / original_slope;

    // Determine the endpoints of the perpendicular line
    double angle = atan(perpendicular_slope);
    int x_offset = length * cos(angle);
    int y_offset = length * sin(angle);

    // Perpendicular line endpoints
    *perp_x1 = px - x_offset;
    *perp_y1 = py - y_offset;
    *perp_x2 = px + x_offset;
    *perp_y2 = py + y_offset;

    // Draw the perpendicular line
    // SDL_RenderDrawLine(renderer, *perp_x1, *perp_y1, *perp_x2, *perp_y2);
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

            // which mode
            int mode = 0; char modePresented[32], lastMode = -1; 
            const lookUpOfModes[] = {
                "Elastic Collision", 
                "Ray Cast", 
                "Tangent Rays",    
                "Lighting System"
            };

            
            Circle lightCircle;
            lightCircle.position.x = 600; lightCircle.position.y = 400; lightCircle.radius = 40; 

            Circle testCircle;
            testCircle.position.x = 800; testCircle.position.y = 400; testCircle.radius = 40; 

            Vector2 tangentPoint1, tangentPoint2;

            
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

                        if (event.key.keysym.sym == SDLK_RIGHT && mode+1 <= 4) mode+=1;
                        if (event.key.keysym.sym == SDLK_LEFT && mode-1 >= 0) mode-=1;

                        // this changes the size of the dropper
                        if (event.key.keysym.sym == SDLK_UP) {};
                        if (event.key.keysym.sym == SDLK_DOWN){};

                    }

                    if (event.type == SDL_MOUSEBUTTONDOWN) {
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            int mouseX, mouseY;
                            SDL_GetMouseState(&mouseX, &mouseY); 
                            printf("Mouse Pressed: (%d, %d)\n", mouseX, mouseY); 
                            
                        }
                    }

                    if (event.type == SDL_MOUSEBUTTONUP) {
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            
                        }
                    }

                    if (event.type == SDL_MOUSEMOTION){
                        int mouseX, mouseY;
                        SDL_GetMouseState(&mouseX, &mouseY);
                        lightCircle.position.x = mouseX;
                        lightCircle.position.y = mouseY;


                    }

                }

                // this chooses the mode and presents it
                if (mode != lastMode)
                {
                    int CurrentMode = (mode >= 1 && mode <= 4) ? &lookUpOfModes[mode] : &lookUpOfModes[0];
                    loadFromRenderedText(&modeTextTexture, CurrentMode, textColor);
                    lastMode = mode; 
                }

                // Clear screen with grey background color
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 100, 255);
                SDL_RenderClear(gRenderer);

                
               



 
                // step 1
                Vector2 lightToObstacleVector;
                lightToObstacleVector.x = testCircle.position.x - lightCircle.position.x;
                lightToObstacleVector.y = testCircle.position.y - lightCircle.position.y;
                
                
                float distLightToObstacle = mag(lightToObstacleVector) - lightCircle.radius;    
                 
                // step 3
                float theta = asin(testCircle.radius / distLightToObstacle); 

                float phi = atan2(lightToObstacleVector.y, lightToObstacleVector.x); // Correct usage

                // step 5

                float directionOfRay1 = phi - theta;
                float directionOfRay2 = phi + theta;


                // step 6

                // this calculates the rays
                float startX = lightCircle.position.x + lightCircle.radius * cos(phi);
                float startY = lightCircle.position.y + lightCircle.radius * sin(phi);
                
                // step 7

                float endXRay1 = startX + 10000 * cos(directionOfRay1);
                float endYRay1 = startY + 10000 * sin(directionOfRay1);

                float endXRay2 = startX + 10000 * cos(directionOfRay2);
                float endYRay2 = startY + 10000 * sin(directionOfRay2);
                

                // SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
                // // Draw the ray to the nearest intersection point or its full length
                // SDL_RenderDrawLine(gRenderer, (int)startX, (int)startY, (int)endXRay1, (int)endYRay1); // ray1

                // SDL_RenderDrawLine(gRenderer, (int)startX, (int)startY, (int)endXRay2, (int)endYRay2); // ray2

                // this calculates the dots of the tangents of the test circle
                // using this tutorial
                // https://stackoverflow.com/questions/49968720/find-tangent-points-in-a-circle-from-a-point

                float vectorFromLightToObstacle = sqrt(pow(startX - testCircle.position.x, 2) + pow(startY - testCircle.position.y, 2)); // this is 'b'

                float thetaV2 = acos(testCircle.radius/ vectorFromLightToObstacle);  // angle theta

                float directionAngle = atan2(startY - testCircle.position.y, startX - testCircle.position.x); // this is 'd'

                float d1 = directionAngle + thetaV2;
                float d2 = directionAngle - thetaV2;
                
                // first tangent point
                tangentPoint1.x = testCircle.position.x + testCircle.radius * cos(d1);
                tangentPoint1.y = testCircle.position.y + testCircle.radius * sin(d1);
                
                // Second tangent point
                tangentPoint2.x = testCircle.position.x + testCircle.radius * cos(d2);
                tangentPoint2.y = testCircle.position.y + testCircle.radius * sin(d2);

                // // Render the tangent points
                // SDL_SetRenderDrawColor(gRenderer, 100, 100, 100, 255);
                // DrawFilledCircle(gRenderer, tangentPoint1.x, tangentPoint1.y, 2);
                // DrawFilledCircle(gRenderer, tangentPoint2.x, tangentPoint2.y, 2);

                float intersectionOfCircleX, intersectionOfCircleY; 
                float intersectionOfLineRay1X, intersectionOfLineRay1Y; // intersection between the perpendicular line and ray 1
                float intersectionOfLineRay2X, intersectionOfLineRay2Y; // intersection between the perpendicular line and ray 2
                int perp_x1, perp_y1, perp_x2, perp_y2;

    
                RayIntersectsCircle(startX, startY, testCircle.position.x, testCircle.position.y, testCircle.position.x, testCircle.position.y, testCircle.radius, &intersectionOfCircleX, &intersectionOfCircleY); 
                

                DrawPerpendicularLine(gRenderer, startX, startY, intersectionOfCircleX, intersectionOfCircleY, intersectionOfCircleX, intersectionOfCircleY, 1000, &perp_x1, &perp_y1, &perp_x2, &perp_y2); 

                rayIntersectsLine(startX, startY, tangentPoint1.x, tangentPoint1.y, perp_x1, perp_y1, perp_x2, perp_y2, &intersectionOfLineRay1X, &intersectionOfLineRay1Y);
                rayIntersectsLine(startX, startY, tangentPoint2.x, tangentPoint2.y, perp_x1, perp_y1, perp_x2, perp_y2, &intersectionOfLineRay2X, &intersectionOfLineRay2Y);

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
                    int randColor = rand() % 15;  // Random index (0 to 14)
                    SDL_SetRenderDrawColor(gRenderer, circles[i].colour.r, circles[i].colour.g, circles[i].colour.b, circles[i].colour.a);
                    
                    // Draw the circle
                    DrawFilledCircle(gRenderer, circles[i].position.x, circles[i].position.y, circles[i].radius);
                }


                // Set the draw color
                SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
                drawShadow(gRenderer, testCircle.position.x, testCircle.position.y, intersectionOfCircleX, intersectionOfCircleY,tangentPoint1.x, tangentPoint1.y, testCircle.radius, intersectionOfLineRay1X, intersectionOfLineRay1Y, intersectionOfLineRay2X, intersectionOfLineRay2Y, endXRay1, endYRay1, endXRay2, endYRay2); // using intersectionOfLineRay1
                drawShadow(gRenderer, testCircle.position.x, testCircle.position.y, tangentPoint2.x, tangentPoint2.y, intersectionOfCircleX, intersectionOfCircleY, testCircle.radius, intersectionOfLineRay2X, intersectionOfLineRay2Y, intersectionOfLineRay1X, intersectionOfLineRay1Y, endXRay1, endYRay1, endXRay2, endYRay2); // using intersectionOfLineRay2

                SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 255);
                DrawFilledCircle(gRenderer, lightCircle.position.x, lightCircle.position.y, lightCircle.radius);
                DrawFilledCircle(gRenderer, testCircle.position.x, testCircle.position.y, testCircle.radius);









                // for (int j = 0; j < NUM_RAYS; j++) {
                //     // Calculate the angle for this ray
                //     float angle = j * (2 * M_PI / NUM_RAYS);

                //     // Calculate the starting point of the ray
                //     float startX = lightCircle.position.x + lightCircle.radius * cos(angle);
                //     float startY = lightCircle.position.y + lightCircle.radius * sin(angle);

                //     // Initialize the end point of the ray to be far away
                //     float endX = startX + 5000 * cos(angle);
                //     float endY = startY + 5000 * sin(angle);
                    

                //     float nearestX = endX, nearestY = endY; // Store nearest intersection point
                //     int foundIntersection = 0; // Flag to indicate intersection occurred

                    
                //     SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
                //     // Draw the ray to the nearest intersection point or its full length
                //     SDL_RenderDrawLine(gRenderer, (int)startX, (int)startY, (int)nearestX, (int)nearestY);

                //     rayStartPoints[j].x = (int)startX; rayStartPoints[j].y = (int)startY;
                //     rayEndPoints[j].x = (int)nearestX; rayEndPoints[j].y = (int)nearestY;


                        
                // }


                
                
                

                //this is for text
                renderTexture(&modeTextTexture, 0,0, NULL, 0, NULL, SDL_FLIP_NONE); 
                SDL_RenderPresent(gRenderer); // Update screen

            }
        }
    }
    close();
    return 0;
}
