module;
#include "raylib.h"
export module gui;

export namespace Gui {

bool
run()
{
    int const screen_width = 800;
    int const screen_height = 450;

    InitWindow(screen_width, screen_height,
               "raylib [core] example - basic window");

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("Congrats! You created your first window!", 190, 200, 20,
                 LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow(); // Close window and OpenGL context
    return true;
}

} // namespace Gui
