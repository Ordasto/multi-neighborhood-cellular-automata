#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




typedef struct {
    float value;
} GridCell;

typedef struct {
    int width;
    int height;
} Size2D;


void update(Size2D* size, GridCell grid_front[size->height][size->width], GridCell grid_back[size->height][size->width]);
void draw(Size2D* size, GridCell grid[size->height][size->width], Camera2D* camera);


Vector2 get_center() {
    Vector2 center = {
        (GetScreenWidth() / 2),
        (GetScreenHeight() / 2),
    };
    return center;
}


int main(int argc, char *argv[]) {
    int screen_width = 900;
    int screen_height = 900;
    InitWindow(screen_width, screen_height, "Langton's Ant");

    Vector2 center = get_center();

    Vector2 offset = center;
    offset.x += 0;
    offset.y += 0;

    Camera2D camera;
    camera.offset = offset;
    camera.rotation = 0;
    camera.target = center;
    camera.zoom = 1.0f;


    int grid_w = 200;
    int grid_h = 200;
    Size2D grid_size = {grid_w, grid_h};

    GridCell (*grid_front)[grid_h] = malloc(grid_w * grid_h * sizeof(GridCell));
    GridCell (*grid_back)[grid_h] = malloc(grid_w * grid_h * sizeof(GridCell));
    // Check if grid allocated correctly
    if (!grid_front || !grid_back) {

        printf("Failed to allocate memory for grid.");
        return -1;
    }
    // instantiate grid
    for (int y = 0; y < grid_h; y++) {
        for (int x = 0; x < grid_w; x++) {
            grid_front[y][x].value = ((float)(rand() % 1000)) / 1000;
            
            // Shouldn't make a difference but better safe than sorry.
            grid_back[y][x].value = grid_front[y][x].value;
        }
    }

    // big center thing
    //grid_front[grid_h / 2][grid_w / 2].value = 1000000;

    SetTargetFPS(60);
    bool sim = true;
    while (!WindowShouldClose()) {

        // MOVE TO INPUT MANAGER
        // Add keybind to pause simulation and speedup/slowdown simulation (effect iterations)
        // camera zoom
        if (IsKeyReleased(KEY_UP)) {
            camera.zoom += 0.1;
        }
        if (IsKeyReleased(KEY_DOWN)) {
            camera.zoom -= 0.1;
        }
        // Camera move
        if (IsKeyDown(KEY_W)) {
            camera.offset.y += 1;
        }
        if (IsKeyDown(KEY_S)) {
            camera.offset.y -= 1;
        }
        if (IsKeyDown(KEY_A)) {
            camera.offset.x += 1;
        }
        if (IsKeyDown(KEY_D)) {
            camera.offset.x -= 1;
        }
        // toggle sim
        if (IsKeyReleased(KEY_SPACE)) {
            sim = !sim;
        }

        // alternate back and front grids
        if (sim) {
            void* temp = grid_front;
            grid_front = grid_back;
            grid_back = temp;
            update(&grid_size, grid_front, grid_back);
        }
        draw(&grid_size, grid_front, &camera);
    }
    CloseWindow();

    return 0;
}

#ifdef _WIN32
int WinMain() {
    return main(0, NULL);
}
#endif // _WIN32


void update(Size2D* size, GridCell grid_front[size->height][size->width], GridCell grid_back[size->height][size->width]) {
    // For each pixel
    float decay = 0.005;
    int nsize = 11;
    int radius = nsize / 2;
    for (int y = 0; y < size->height; y++) {
        for (int x = 0; x < size->width; x++) {
            // For each neighbor
            float neighbourhood_total = 0.0;
            // Change this to just be caluclated once outside the loop
            // make neighbourhood size customisable
            for (int y_mod = -radius; y_mod <= radius; y_mod++) {
                for (int x_mod = -radius; x_mod <= radius; x_mod++) {
    
                    // Neighbor x
                    int n_x = x + x_mod;
                    // Neighbor y
                    int n_y = y + y_mod;

                    if (n_x < 0) {
                        n_x += size->width;
                    }
                    else if (n_x >= size->width) {
                        n_x -= size->width;
                    }

                    if (n_y < 0) {
                        n_y += size->height;
                    }
                    else if (n_y >= size->height) {
                        n_y -= size->height;
                    }

                    neighbourhood_total += grid_back[n_y][n_x].value;

                }
            }
            //float out = 0;
            float out = neighbourhood_total / (nsize*nsize);

            //if (neighbourhood_total >= 0.0 && neighbourhood_total <= 33.0) {
            //    out = 0.0;
            //}
            //if (neighbourhood_total >= 34.0 && neighbourhood_total <= 45.0) {
            //    out = 1.0;
            //}
            //if (neighbourhood_total >= 58.0 && neighbourhood_total <= 121.0) {
            //    out = 0.0;
            //}
            grid_front[y][x].value = out;
            //grid_front[y][x].value = grid_back[y][x].value;
        }
    }
}

void draw(Size2D* size, GridCell grid[size->height][size->width], Camera2D* camera) {
    BeginDrawing();
    BeginMode2D(*camera);

    ClearBackground(BLACK);

    Vector2 rect_size = {
        ((float)GetScreenWidth() / (float)size->width),
        ((float)GetScreenHeight() / (float)size->height),
    };

    // This should probably be put in 'update' or a seperate input handling function.
    int x_cord = GetMouseX() / rect_size.x;
    int y_cord = GetMouseY() / rect_size.y;
    float mouse_value = grid[y_cord][x_cord].value;
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        // Need to translate screen coords to game coords
        // add ability to change size and color for mouse
        grid[y_cord][x_cord].value = 1000.0f;
    }

    for (int y = 0; y < size->height; y++) {
        for (int x = 0; x < size->width; x++) {
            GridCell* cell = &grid[y][x];
            float value = cell->value;
            if (value > 1.0) {
                value = 1.0;
            }
            if (value > 0.0) {
                Vector2 pos = {
                    (rect_size.x * x),
                    (rect_size.y * y),
                };

                Color col = {
                    255,
                    255,
                    255,
                    (int)(255.0 * value),
                };
                
                // Want to represent a linear value with more than just one color component
                // e.g. lowest values complete blue that then becomes rec?
                //short val = 510 * cell->value;
                //Color col = {
                //    val & 0x0000FFFF,
                //    0,
                //    val & 0xFFFF0000,
                //    255,
                //};

                DrawRectangleV(pos, rect_size, col);

            }
        }
    }
    // Draws fps
    EndMode2D();

    DrawRectangle(0,0,50,25,WHITE);
    DrawText(TextFormat("FPS:%d", GetFPS()), 5, 5, 10, BLACK);
    //DrawText(TextFormat("ZOOM:%f", zoom), 5, 20, 10, BLACK);
    // 
    // Draws mouse X and Y next to mouse
    DrawText(TextFormat("x:%d. y:%d", GetMouseX(), GetMouseY()), GetMouseX(), GetMouseY()-15, 20, RED);
    DrawText(TextFormat("val:%f", mouse_value), GetMouseX(), GetMouseY()-30, 20, RED);
    EndDrawing();
}
