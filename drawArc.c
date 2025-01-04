// gcc -O3 -I src/include -L src/lib -o main deep.c -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer

#include <SDL2/SDL.h>
#include <math.h>

#define PI 3.14159265358979323846

// Helper function to convert degrees to radians
double deg_to_rad(double degrees) {
    return degrees * M_PI / 180.0;
}

// Helper function to calculate angle between two points
double get_angle(int x1, int y1, int x2, int y2) {
    return atan2(y2 - y1, x2 - x1) * 180.0 / M_PI;
}

// Draw an arc from start point to end point with given radius
void draw_arc(SDL_Renderer* renderer, int center_x, int center_y, 
              int start_x, int start_y, int end_x, int end_y, 
              int radius, SDL_Color color) {
    
    // Set the draw color
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    // Calculate start and end angles
    double start_angle = get_angle(center_x, center_y, start_x, start_y);
    double end_angle = get_angle(center_x, center_y, end_x, end_y);
    
    // Ensure end angle is greater than start angle
    if (end_angle < start_angle) {
        end_angle += 360.0;
    }
    
    // Number of segments to draw (more segments = smoother arc)
    const int segments = 120;
    double angle_step = (end_angle - start_angle) / segments;
    
    // Variables to store the previous point
    int prev_x = 0;
    int prev_y = 0;
    
    // Draw the arc segment by segment
    for (int i = 0; i <= segments; i++) {
        double angle = deg_to_rad(start_angle + (i * angle_step));
        int x = center_x + (int)(radius * cos(angle));
        int y = center_y + (int)(radius * sin(angle));
        
        if (i > 0) {
            // Draw line from previous point to current point
            SDL_RenderDrawLine(renderer, 
                             prev_x, prev_y,
                             x, y);
        }
        
        // Store current point as previous for next iteration
        prev_x = x;
        prev_y = y;
    }
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Circle Arc", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1392, 744, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Set the draw color to white
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Color white = {255, 255, 255, 255};


    draw_arc(renderer, 800, 400,  720, 401, 785, 322, 80, white);       

    // Present the renderer
    SDL_RenderPresent(renderer);

    // Wait for a few seconds to see the result
    SDL_Delay(5000);

    // Clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
