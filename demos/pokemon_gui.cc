///////////////////   System headers   ////////////////////////////////////////
#include <exception>
#include <iostream>

///////////////////   External dependencies   /////////////////////////////////
#include "raylib.h"

///////////////////   Project based internal modules   ////////////////////////
import dancing_links;

namespace Dx = Dancing_links;

namespace {

int
run()
{
    try
    {
        std::ifstream gen("data/dst/Gen-1-Kanto.dst");
        Dx::Type_encoding const tester("Fire");
        Dx::Pokemon_test const interactions = Dx::load_pokemon_generation(gen);
        std::cout << "Hello from the GUI.\n";
        std::cout << "Tester Type_encoding is: " << tester << "\n";
        std::cout << "Generation size is: " << interactions.interactions.size()
                  << "\n";
        std::cout << "Generation map city count is: "
                  << interactions.gen_map.network.size() << "\n";
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
        return 0;
    } catch (std::exception const &e)
    {
        std::cerr << "exception caught: " << e.what() << std::flush;
        return 1;
    }
}

} // namespace

int
main()
{
    return run();
}
