#include <raylib.h>

int main(void)
{
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "square game");
    SetTargetFPS(60);

    const int SQUARE_SIZE = 100;
    const int SQUARE_X = SCREEN_WIDTH / 2 - SQUARE_SIZE / 2;
    const int SQUARE_Y = SCREEN_HEIGHT / 2 - SQUARE_SIZE / 2;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawRectangle(SQUARE_X, SQUARE_Y, SQUARE_SIZE, SQUARE_SIZE, DARKBLUE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
