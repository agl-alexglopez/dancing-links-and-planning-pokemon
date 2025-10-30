///////////////////   System headers   ////////////////////////////////////////
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <map>
#include <string_view>
#include <utility>
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

class Minimap {
  public:
    Minimap();
    void draw(int window_width, int window_height);

  private:
    //////////////////////   Constants       //////////////////////////////////

    static constexpr float origin_x = 1.0;
    static constexpr float origin_y = 1.0;
    static constexpr float scale_factor = 0.25;
    static constexpr float button_size = 25;
    static constexpr float text_label_font_size = 5.0;
    static constexpr float map_pad = 3.0;
    static constexpr char const *const dst_relative_path = "data/dst/";

    //////////////////////   Helper Types    //////////////////////////////////

    struct Dropdown
    {
        Rectangle dimensions;
        int active;
        bool editmode;
    };

    //////////////////////   Data Structures  /////////////////////////////////

    /// Raylib Dropdowns use an active integer to track which option of a drop
    /// down is selected. So, it is helpful to store them contiguously
    /// corresponding to each active index.
    std::vector<std::string> dropdown_active_list;

    /// Raylib requires a semicolon separated string to display all options in
    /// the dropdown ("option1;option2"). The final options should not have a
    /// trailing semicolon.
    std::string dropdown_options;

    /// The state we must track to successfully use a Raylib drop down.
    Dropdown dst_map_select;

    /// The data from the current Pokemon generation map we have loaded in.
    /// This data structure tells us where all the gyms are for this generation
    /// on the map. It also tells us how all the types interact with one
    /// another in the form of a type map where the key is the type and the
    /// value is the set of defense multipliers against the single attack types
    /// in the game.
    Dx::Pokemon_test generation;

    /// The user interacts with the mini map by clicking the gym buttons. By
    /// default the solution to the entire game is loaded. But users can select
    /// smaller sub-problems of gyms to solve for.
    ///
    /// This vector stores an iterator to the Pokemon test map city location
    /// for a gym so that we can easily iterate through all gyms and render
    /// the button placement correctly along with the toggle gym button state
    /// boolean.
    ///
    /// Therefore this data structure must be initialized AFTER the current
    /// generation has loaded in. The current generation city locations map
    /// never changes once loaded in for the first time.
    ///
    /// These data structures are tightly bound like this because we don't want
    /// to bother the Dx::Pokemon_test with adding state. We do it here.
    std::vector<
        std::pair<std::map<std::string, Dx::Map_node>::const_iterator, bool>>
        gym_toggles;

    /// The defensive dancing links solver. Loaded along with each new
    /// generation.
    Dx::Pokemon_links defense_dlx;

    /// The attack dancing links solver. Loaded along with each new generation.
    Dx::Pokemon_links attack_dlx;

    //////////////////////    Functions ///////////////////////////////////

    void draw_frame_and_dropdown(float minimap_width, float minimap_height);
    void reload_generation();
    static Dx::Point scale_point(Dx::Point const &p,
                                 Dx::Min_max const &x_data_bounds,
                                 Dx::Min_max const &x_draw_bounds,
                                 Dx::Min_max const &y_data_bounds,
                                 Dx::Min_max const &y_draw_bounds);
};

class Scene {
  public:
    void draw(int window_width, int window_height);

  private:
    Minimap minimap;
};

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
        int screen_width = 800;
        int screen_height = 450;

        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(screen_width, screen_height,
                   "raylib [core] example - basic window");

        SetTargetFPS(60);
        Scene scene;

        while (!WindowShouldClose())
        {
            screen_width = GetScreenWidth();
            screen_height = GetScreenHeight();
            BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("Congrats! You created your first window!", 190, 200, 20,
                     LIGHTGRAY);
            scene.draw(screen_width, screen_height);

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

void
Scene::draw(int const window_width, int const window_height)
{
    minimap.draw(window_width, window_height);
}

Minimap::Minimap()
    : dst_map_select({
          .active = 0,
          .editmode = false,
      })
{
    for (auto const &entry : std::filesystem::directory_iterator("data/dst"))
    {
        if (entry.is_directory())
        {
            continue;
        }
        std::filesystem::path const &p = entry.path();
        std::string_view file_name(p.c_str());
        file_name = file_name.substr(file_name.find_last_of('/') + 1);
        dropdown_active_list.emplace_back(file_name);
    }
    if (dropdown_active_list.empty())
    {
        return;
    }
    std::ranges::sort(dropdown_active_list,
                      [](std::string const &a, std::string const &b) {
                          return a.compare(b) < 0;
                      });
    for (size_t i = 0; i < dropdown_active_list.size() - 1; ++i)
    {
        dropdown_options.append(dropdown_active_list[i]).append(";");
    }
    dropdown_options.append(dropdown_active_list.back());
    reload_generation();
}

void
Minimap::reload_generation()
{
    if (dst_map_select.active >= dropdown_active_list.size())
    {
        std::cerr << "Active Pokemon Generation selector out of range.\n";
        std::abort();
    }
    std::string const path
        = std::string(dst_relative_path)
              .append(dropdown_active_list[dst_map_select.active]);
    std::ifstream gen(path);
    if (gen.fail())
    {
        std::cerr << "Cannot load Pokemon Generation .dst file " << path
                  << '.\n';
        std::abort();
    }
    // Clear the old gym toggles before loading new generation so we are not
    // left with references to map data that does not exist.
    gym_toggles.clear();
    // Remember to load the Pokemon generation first.
    generation = Dx::load_pokemon_generation(gen);
    // Next, the gym toggles can now finally be loaded and locked to the map.
    for (auto gym = std::ranges::cbegin(generation.gen_map.network);
         gym != std::ranges::cend(generation.gen_map.network);
         gym = std::ranges::next(gym))
    {
        gym_toggles.emplace_back(gym, false);
    }
    defense_dlx = Dx::Pokemon_links(generation.interactions,
                                    Dx::Pokemon_links::Coverage_type::defense);
    attack_dlx = Dx::Pokemon_links(generation.interactions,
                                   Dx::Pokemon_links::Coverage_type::attack);
}

void
Minimap::draw_frame_and_dropdown(float const minimap_width,
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
        if (!dst_map_select.editmode)
        {
            reload_generation();
        }
    }
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
Minimap::draw(int const window_width, int const window_height)
{

    float const minimap_width = static_cast<float>(window_width) * scale_factor;
    float const minimap_height
        = static_cast<float>(window_height) * scale_factor;

    draw_frame_and_dropdown(minimap_width, minimap_height);

    float const minimap_aspect_ratio = minimap_width / minimap_height;
    float const file_specified_aspect_ratio
        = (generation.gen_map.x_data_bounds.max
           - generation.gen_map.x_data_bounds.min)
          / (generation.gen_map.y_data_bounds.max
             - generation.gen_map.y_data_bounds.min);
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

    for (auto const &[city_string, node_info] : generation.gen_map.network)
    {
        Dx::Point const src = scale_point(
            node_info.coordinates, generation.gen_map.x_data_bounds,
            x_draw_bounds, generation.gen_map.y_data_bounds, y_draw_bounds);
        for (std::string const *edge : node_info.edges)
        {
            Dx::Map_node const dst_info = generation.gen_map.network.at(*edge);
            Dx::Point const dst = scale_point(
                dst_info.coordinates, generation.gen_map.x_data_bounds,
                x_draw_bounds, generation.gen_map.y_data_bounds, y_draw_bounds);
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
    // We may be mutating a buttons toggle state we mutably iterate.
    for (auto &[city_location_map_iterator, toggle_state] : gym_toggles)
    {
        Dx::Map_node const &file_coordinates
            = city_location_map_iterator->second;
        Dx::Point const scaled_coordinates = scale_point(
            file_coordinates.coordinates, generation.gen_map.x_data_bounds,
            x_draw_bounds, generation.gen_map.y_data_bounds, y_draw_bounds);
        // We want the lines that connect the button nodes to run through the
        // center of the button. Buttons are drawn as squares with the top
        // left corner at the x and y point so move that corner so that the
        // center of the button is the center of node and line connections.
        if (GuiToggle(
                Rectangle{
                    .height = button_size,
                    .width = button_size,
                    .x = scaled_coordinates.x - (button_size / 2),
                    .y = scaled_coordinates.y - (button_size / 2),
                },
                city_location_map_iterator->first.c_str(), &toggle_state))
        {}
    }
}

Dx::Point
Minimap::scale_point(Dx::Point const &p, Dx::Min_max const &x_data_bounds,
                     Dx::Min_max const &x_draw_bounds,
                     Dx::Min_max const &y_data_bounds,
                     Dx::Min_max const &y_draw_bounds)
{
    return Dx::Point{
        .x
        = (((p.x - x_data_bounds.min) / (x_data_bounds.max - x_data_bounds.min))
           * (x_draw_bounds.max - x_draw_bounds.min))
          + x_draw_bounds.min + origin_x,
        .y
        = (((p.y - y_data_bounds.min) / (y_data_bounds.max - y_data_bounds.min))
           * (y_draw_bounds.max - y_draw_bounds.min))
          + y_draw_bounds.min + origin_y,
    };
}

} // namespace
