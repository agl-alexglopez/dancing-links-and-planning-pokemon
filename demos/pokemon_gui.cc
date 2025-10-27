///////////////////   System headers   ////////////////////////////////////////
#include <algorithm>
#include <exception>
#include <iostream>
#include <limits>

///////////////////   External dependencies   /////////////////////////////////
#include "raylib.h"

///////////////////   Project based internal modules   ////////////////////////
import dancing_links;

namespace Dx = Dancing_links;

////////////////////////    Prototypes     ////////////////////////////////////
namespace {
int run();

namespace Minimap {
void draw(Dx::Pokemon_test const &pt, int window_width, int window_height);
} // namespace Minimap

} // namespace

///////////////////////     Main           ////////////////////////////////////

int
main()
{
    return run();
}

///////////////////////////   Implementation   ////////////////////////////////

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

        SetTargetFPS(60);

        // Main game loop
        while (!WindowShouldClose()) // Detect window close button or ESC key
        {
            BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("Congrats! You created your first window!", 190, 200, 20,
                     LIGHTGRAY);
            Minimap::draw(interactions, screen_width, screen_height);

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

namespace Minimap {

constexpr float origin_x = 1.0;
constexpr float origin_y = 1.0;
constexpr float scale_factor = 0.40;
constexpr float node_radius = 7.0;
constexpr float text_label_font_size = 5.0;
constexpr float map_pad = 3.0;
constexpr float file_coordinate_pad = 1.0;

struct Min_max
{
    float min;
    float max;
};

struct Map_bounds
{
    struct Min_max x_data;
    struct Min_max y_data;
    struct Min_max x_draw;
    struct Min_max y_draw;
};

Dx::Point
scale_point(Dx::Point const &p, Map_bounds const &bounds)
{
    return Dx::Point{
        (((p.x - bounds.x_data.min) / (bounds.x_data.max - bounds.x_data.min))
         * (bounds.x_draw.max - bounds.x_draw.min))
            + bounds.x_draw.min + origin_x,
        (((p.y - bounds.y_data.min) / (bounds.y_data.max - bounds.y_data.min))
         * (bounds.y_draw.max - bounds.y_draw.min))
            + bounds.y_draw.min + origin_y,
    };
}

/// Draws the Pokemon generation minimap to the top left corner of the
/// screen. The minimap is scaled appropriately based on the dimensions of
/// the overall window so that the shape of a Pokemon region is visible,
/// roads connect gyms, and gyms act as selectable buttons to create subset
/// Dx solver problems.
void
draw(Dx::Pokemon_test const &pt, int const window_width,
     int const window_height)
{
    Map_bounds bounds = {
        .x_data= {
            .min = std::numeric_limits<float>::infinity(),
            .max = -std::numeric_limits<float>::infinity(),
        },
        .y_data= {
            .min = std::numeric_limits<float>::infinity(),
            .max = -std::numeric_limits<float>::infinity(),
        },
        .x_draw = {},
        .y_draw = {},
    };
    for (auto const &node : pt.gen_map.city_locations)
    {
        bounds.x_data.min = std::min(bounds.x_data.min, node.second.x);
        bounds.y_data.min = std::min(bounds.y_data.min, node.second.y);
        bounds.x_data.max = std::max(bounds.x_data.max, node.second.x);
        bounds.y_data.max = std::max(bounds.y_data.max, node.second.y);
    }
    bounds.x_data.min -= file_coordinate_pad;
    bounds.y_data.min -= file_coordinate_pad;
    bounds.x_data.max += file_coordinate_pad;
    bounds.y_data.max += file_coordinate_pad;

    float const minimap_width = static_cast<float>(window_width) * scale_factor;
    float const minimap_height
        = static_cast<float>(window_height) * scale_factor;
    float const minimap_aspect_ratio = minimap_width / minimap_height;
    float const file_specified_aspect_ratio
        = (bounds.x_data.max - bounds.x_data.min)
          / (bounds.y_data.max - bounds.y_data.min);
    float file_specified_width{};
    float file_specified_height{};
    if (file_specified_aspect_ratio >= minimap_aspect_ratio)
    {
        file_specified_width = minimap_width;
        file_specified_height
            = file_specified_width / file_specified_aspect_ratio;
    }
    else
    {
        file_specified_height = minimap_height;
        file_specified_width
            = file_specified_aspect_ratio * file_specified_height;
    }
    bounds.x_draw.min
        = ((minimap_width - file_specified_width) / 2.0F) + map_pad;
    bounds.y_draw.min
        = ((minimap_height - file_specified_height) / 2.0F) + map_pad;
    bounds.x_draw.max = bounds.x_draw.min + file_specified_width;
    bounds.y_draw.max = bounds.y_draw.min + file_specified_height;

    DrawRectangleV(
        Vector2{
            .x = origin_x,
            .y = origin_y,
        },
        Vector2{
            .x = minimap_width,
            .y = minimap_height,
        },
        WHITE);
    for (auto const &node : pt.gen_map.network)
    {
        Dx::Point const src_file_coordinates
            = pt.gen_map.city_locations.at(node.first);
        Dx::Point const src = scale_point(src_file_coordinates, bounds);
        for (auto const &edge : node.second)
        {
            Dx::Point const dst_file_coordinates
                = pt.gen_map.city_locations.at(edge);
            Dx::Point const dst = scale_point(dst_file_coordinates, bounds);
            DrawLineV(
                Vector2{
                    .x = src.x - node_radius,
                    .y = src.y - node_radius,
                },
                Vector2{
                    .x = dst.x - node_radius,
                    .y = dst.y - node_radius,
                },
                BLACK);
        }
    }
    for (auto const &node : pt.gen_map.city_locations)
    {
        Dx::Point const file_coordinates = node.second;
        Dx::Point const scaled_coordinates
            = scale_point(file_coordinates, bounds);
        DrawCircleV(
            Vector2{
                .x = scaled_coordinates.x - node_radius,
                .y = scaled_coordinates.y - node_radius,
            },
            node_radius, BLUE);
        DrawText(node.first.c_str(), static_cast<int>(scaled_coordinates.x),
                 static_cast<int>(scaled_coordinates.y), text_label_font_size,
                 BLACK);
    }
}

} // namespace Minimap

} // namespace
