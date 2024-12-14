#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

#define WIDTH 600
#define HEIGHT 500
#define SQUARE_SIZE 5
#define HUE_MAX 360

int cols, rows;
float gravity = 0.1;
float hueValue = 200;

float **make2DArray(int cols, int rows) {
    float **arr = malloc(cols * sizeof(float *));
    for (int i = 0; i < cols; i++) {
        arr[i] = malloc(rows * sizeof(float));
        for (int j = 0; j < rows; j++) {
            arr[i][j] = 0;
        }
    }
    return arr;
}

bool withinCols(int i) {
    return i >= 0 && i < cols;
}

bool withinRows(int j) {
    return j >= 0 && j < rows;
}

void drawSquare(SDL_Renderer *renderer, int x, int y, int size, float hue) {
    int r = (int)(255 * fabs(sin(hue * M_PI / 180.0)));
    int g = (int)(255 * fabs(sin((hue + 120) * M_PI / 180.0)));
    int b = (int)(255 * fabs(sin((hue + 240) * M_PI / 180.0)));
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_Rect rect = {x, y, size, size};
    SDL_RenderFillRect(renderer, &rect);
}

void free2DArray(float **arr, int cols) {
    for (int i = 0; i < cols; i++) {
        free(arr[i]);
    }
    free(arr);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Falling Sand Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    cols = WIDTH / SQUARE_SIZE;
    rows = HEIGHT / SQUARE_SIZE;

    float **grid = make2DArray(cols, rows);
    float **velocityGrid = make2DArray(cols, rows);

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEMOTION) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int mouseX = event.motion.x;
                    int mouseY = event.motion.y;
                    int mouseCol = mouseX / SQUARE_SIZE;
                    int mouseRow = mouseY / SQUARE_SIZE;

                    int matrix = 5;
                    int extent = matrix / 2;
                    for (int i = -extent; i <= extent; i++) {
                        for (int j = -extent; j <= extent; j++) {
                            if ((float)rand() / RAND_MAX < 0.75) {
                                int col = mouseCol + i;
                                int row = mouseRow + j;
                                if (withinCols(col) && withinRows(row)) {
                                    grid[col][row] = hueValue;
                                    velocityGrid[col][row] = 1;
                                }
                            }
                        }
                    }
                    hueValue += 0.5;
                    if (hueValue > HUE_MAX) {
                        hueValue = 1;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < cols; i++) {
            for (int j = 0; j < rows; j++) {
                if (grid[i][j] > 0) {
                    int x = i * SQUARE_SIZE;
                    int y = j * SQUARE_SIZE;
                    drawSquare(renderer, x, y, SQUARE_SIZE, grid[i][j]);
                }
            }
        }

        float **nextGrid = make2DArray(cols, rows);
        float **nextVelocityGrid = make2DArray(cols, rows);

        for (int i = 0; i < cols; i++) {
            for (int j = 0; j < rows; j++) {
                float state = grid[i][j];
                float velocity = velocityGrid[i][j];
                bool moved = false;
                if (state > 0) {
                    int newPos = (int)(j + velocity);
                    for (int y = newPos; y > j; y--) {
                        float below = (withinRows(y)) ? grid[i][y] : -1;
                        int dir = (rand() % 2 == 0) ? 1 : -1;
                        float belowA = (withinCols(i + dir) && withinRows(y)) ? grid[i + dir][y] : -1;
                        float belowB = (withinCols(i - dir) && withinRows(y)) ? grid[i - dir][y] : -1;

                        if (below == 0) {
                            nextGrid[i][y] = state;
                            nextVelocityGrid[i][y] = velocity + gravity;
                            moved = true;
                            break;
                        } else if (belowA == 0) {
                            nextGrid[i + dir][y] = state;
                            nextVelocityGrid[i + dir][y] = velocity + gravity;
                            moved = true;
                            break;
                        } else if (belowB == 0) {
                            nextGrid[i - dir][y] = state;
                            nextVelocityGrid[i - dir][y] = velocity + gravity;
                            moved = true;
                            break;
                        }
                    }
                }
                if (state > 0 && !moved) {
                    nextGrid[i][j] = grid[i][j];
                    nextVelocityGrid[i][j] = velocityGrid[i][j] + gravity;
                }
            }
        }

        free2DArray(grid, cols);
        free2DArray(velocityGrid, cols);
        grid = nextGrid;
        velocityGrid = nextVelocityGrid;

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    free2DArray(grid, cols);
    free2DArray(velocityGrid, cols);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
