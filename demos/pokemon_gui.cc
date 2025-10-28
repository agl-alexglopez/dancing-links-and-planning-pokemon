///////////////////   System headers   ////////////////////////////////////////
#include <algorithm>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <vector>

///////////////////   External dependencies   /////////////////////////////////
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

///////////////////   Project based internal modules   ////////////////////////
import dancing_links;

namespace Dx = Dancing_links;

////////////////////////    Prototypes     ////////////////////////////////////
namespace {
int run();

namespace Minimap {
class Map {
  public:
    Map();
    void draw(Dx::Pokemon_test &pt, int window_width, int window_height);

  private:
    struct Dropdown
    {
        Rectangle dimensions;
        int active;
        bool editmode;
    };
    std::string dropdown_options;
    Dropdown dst_map_select;
    void draw_frame_and_dropdown(float minimap_width, float minimap_height);
};
} // namespace Minimap

namespace Render {
class Draw {
  public:
    void draw(Dx::Pokemon_test &pt, int window_width, int window_height);

  private:
    Minimap::Map minimap;
};
} // namespace Render

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
        if (gen.fail())
        {
            return 1;
        }
        Dx::Pokemon_test interactions = Dx::load_pokemon_generation(gen);
        int screen_width = 800;
        int screen_height = 450;

        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(screen_width, screen_height,
                   "raylib [core] example - basic window");

        SetTargetFPS(60);
        Render::Draw scene;

        while (!WindowShouldClose())
        {
            screen_width = GetScreenWidth();
            screen_height = GetScreenHeight();
            BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("Congrats! You created your first window!", 190, 200, 20,
                     LIGHTGRAY);
            scene.draw(interactions, screen_width, screen_height);

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

namespace Render {

void
Draw::draw(Dx::Pokemon_test &pt, int const window_width,
           int const window_height)
{
    minimap.draw(pt, window_width, window_height);
}

} // namespace Render

namespace Minimap {

constexpr float origin_x = 1.0;
constexpr float origin_y = 1.0;
constexpr float scale_factor = 0.25;
constexpr float button_size = 25;
constexpr float text_label_font_size = 5.0;
constexpr float map_pad = 3.0;

Map::Map()
    : dst_map_select({
          .active = 0,
          .editmode = false,
      })
{
    std::vector<std::string> files{};
    for (auto const &entry : std::filesystem::directory_iterator("data/dst"))
    {
        if (entry.is_directory())
        {
            continue;
        }
        std::filesystem::path const &p = entry.path();
        std::string_view file_name(p.c_str());
        file_name = file_name.substr(file_name.find_last_of('/') + 1);
        files.emplace_back(file_name);
    }
    if (files.empty())
    {
        return;
    }
    std::ranges::sort(files, [](std::string const &a, std::string const &b) {
        return a.compare(b) < 0;
    });
    for (size_t i = 0; i < files.size() - 1; ++i)
    {
        dropdown_options.append(files[i]).append(";");
    }
    dropdown_options.append(files.back());
}

void
Map::draw_frame_and_dropdown(float const minimap_width,
                             float const minimap_height)
{
    // Layout the map and the drop down menu below it.
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
    dst_map_select.dimensions = Rectangle{
        .width = minimap_width * 0.33F,
        .height = minimap_height * 0.05F,
        .x = origin_x,
        .y = origin_y + minimap_height,
    };
    if (GuiDropdownBox(dst_map_select.dimensions, dropdown_options.c_str(),
                       &dst_map_select.active, dst_map_select.editmode))
    {
        dst_map_select.editmode = !dst_map_select.editmode;
    }
}

inline Dx::Point
scale_point(Dx::Point const &p, Dx::Min_max const &x_data_bounds,
            Dx::Min_max const &x_draw_bounds, Dx::Min_max const &y_data_bounds,
            Dx::Min_max const &y_draw_bounds)
{
    return Dx::Point{
        (((p.x - x_data_bounds.min) / (x_data_bounds.max - x_data_bounds.min))
         * (x_draw_bounds.max - x_draw_bounds.min))
            + x_draw_bounds.min + origin_x,
        (((p.y - y_data_bounds.min) / (y_data_bounds.max - y_data_bounds.min))
         * (y_draw_bounds.max - y_draw_bounds.min))
            + y_draw_bounds.min + origin_y,
    };
}

/// Draws the Pokemon generation minimap to the top left corner of the
/// screen. The minimap is scaled appropriately based on the dimensions of
/// the overall window so that the shape of a Pokemon region is visible,
/// roads connect gyms, and gyms act as selectable buttons to create subset
/// Dx solver problems.
///
/// The window has been allowed to be resizable so this draw call should occur
/// on every loop in case the window size is updated.
void
Map::draw(Dx::Pokemon_test &pt, int const window_width, int const window_height)
{

    float const minimap_width = static_cast<float>(window_width) * scale_factor;
    float const minimap_height
        = static_cast<float>(window_height) * scale_factor;

    draw_frame_and_dropdown(minimap_width, minimap_height);

    float const minimap_aspect_ratio = minimap_width / minimap_height;
    float const file_specified_aspect_ratio
        = (pt.gen_map.x_data_bounds.max - pt.gen_map.x_data_bounds.min)
          / (pt.gen_map.y_data_bounds.max - pt.gen_map.y_data_bounds.min);
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
    Dx::Min_max x_draw_bounds{};
    Dx::Min_max y_draw_bounds{};
    x_draw_bounds.min
        = ((minimap_width - file_specified_width) / 2.0F) + map_pad;
    x_draw_bounds.max = x_draw_bounds.min + file_specified_width;
    y_draw_bounds.min
        = ((minimap_height - file_specified_height) / 2.0F) + map_pad;
    y_draw_bounds.max = y_draw_bounds.min + file_specified_height;

    for (auto const &node : pt.gen_map.network)
    {
        Dx::Point const src_file_coordinates
            = pt.gen_map.city_locations.at(node.first);
        Dx::Point const src = scale_point(
            src_file_coordinates, pt.gen_map.x_data_bounds, x_draw_bounds,
            pt.gen_map.y_data_bounds, y_draw_bounds);
        for (auto const &edge : node.second)
        {
            Dx::Point const dst_file_coordinates
                = pt.gen_map.city_locations.at(edge);
            Dx::Point const dst = scale_point(
                dst_file_coordinates, pt.gen_map.x_data_bounds, x_draw_bounds,
                pt.gen_map.y_data_bounds, y_draw_bounds);
            DrawLineV(
                Vector2{
                    .x = src.x,
                    .y = src.y,
                },
                Vector2{
                    .x = dst.x,
                    .y = dst.y,
                },
                BLACK);
        }
    }
    for (auto const &node : pt.gen_map.city_locations)
    {
        Dx::Point const file_coordinates = node.second;
        Dx::Point const scaled_coordinates = scale_point(
            file_coordinates, pt.gen_map.x_data_bounds, x_draw_bounds,
            pt.gen_map.y_data_bounds, y_draw_bounds);
        // We want the lines that connect the button nodes to run through the
        // center of the button. Buttons are drawn as squares with the top
        // left corner at the x and y point so move that corner so that the
        // center of the button is the center of node and line connections.
        GuiButton(
            Rectangle{
                .height = button_size,
                .width = button_size,
                .x = scaled_coordinates.x - (button_size / 2),
                .y = scaled_coordinates.y - (button_size / 2),
            },
            node.first.c_str());
    }
}

} // namespace Minimap

} // namespace
