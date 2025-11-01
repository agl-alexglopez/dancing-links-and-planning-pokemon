///////////////////   System headers   ////////////////////////////////////////
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <set>
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

constexpr std::string_view font_path = "data/font/PokemonGb-RAeo.ttf";

int run();

/// Helper lambda for building a static table of rgb colors for types at compile
/// time in the Generation colors table. Intended for use at compile time.
auto const from_hex = [](uint32_t const hex_code) -> Color {
    return Color{
        .r = static_cast<unsigned char>((hex_code & 0xFF0000U) >> 16),
        .g = static_cast<unsigned char>((hex_code & 0xFF00U) >> 8),
        .b = static_cast<unsigned char>(hex_code & 0xFF),
        .a = 0xFF,
    };
};

class Generation {
  public:
    Generation();
    void draw(int window_width, int window_height);

  private:
    //////////////////////   Helper Types    //////////////////////////////////

    struct Dropdown
    {
        Rectangle dimensions;
        int active;
        bool editmode;
    };

    enum class Solution_request : int // NOLINT
    {
        attack_exact_cover,
        attack_overlapping_cover,
        defense_exact_cover,
        defense_overlapping_cover,
    };

    struct Type_display_info
    {
        std::string_view type;
        Color rgb;
    };

    //////////////////////   Constants       //////////////////////////////////

    enum : uint8_t
    {
        pokemon_party_size = 6,
        attack_slots = pokemon_party_size * 4,
    };

    static constexpr float minimap_origin_x = 1.0;
    static constexpr float minimap_origin_y = 1.0;
    static constexpr float scale_minimap_y_factor = 0.25;
    static constexpr float minimap_button_size = 30;
    static constexpr float text_label_font_size = 5.0;
    static constexpr float map_pad = 3.0;
    static constexpr char const *const dst_relative_path = "data/dst/";
    /// The string provided to the solver drop down selection by Raylib.
    static constexpr char const *const dlx_solver_options
        = "Defense Exact Cover;Defense Overlapping Cover;Attack "
          "Exact Cover;Attack Overlapping Cover";
    static constexpr std::string_view graph_display_message
        = "Solution not yet requested. Select the Pokemon map and cover "
          "problem from the dropdown menus. If you wish to solve for a subset "
          "of gyms, select them on the minimap. Press the 'Solve!' button to "
          "initiate a solution.";
    static constexpr std::string_view no_solution_message
        = "No solution could be found for this Pokemon generation and cover "
          "problem. Try a different cover problem, gym configuration, or "
          "generation, then press the 'Solve!' button.";
    /// Nodes will use a backing color corresponding to a type as well as an
    /// abbreviation for the type name.
    static constexpr std::array<Type_display_info, 18> type_colors{
        Type_display_info{.type = "Bug", .rgb = from_hex(0xA6B91A)},
        Type_display_info{.type = "Drk", .rgb = from_hex(0x705746)},
        Type_display_info{.type = "Drg", .rgb = from_hex(0x6F35FC)},
        Type_display_info{.type = "Elc", .rgb = from_hex(0xF7D02C)},
        Type_display_info{.type = "Fry", .rgb = from_hex(0xD685AD)},
        Type_display_info{.type = "Fgt", .rgb = from_hex(0xC22E28)},
        Type_display_info{.type = "Fir", .rgb = from_hex(0xEE8130)},
        Type_display_info{.type = "Fly", .rgb = from_hex(0xA98FF3)},
        Type_display_info{.type = "Ghs", .rgb = from_hex(0x735797)},
        Type_display_info{.type = "Grs", .rgb = from_hex(0x7AC74C)},
        Type_display_info{.type = "Gnd", .rgb = from_hex(0xE2BF65)},
        Type_display_info{.type = "Ice", .rgb = from_hex(0x96D9D6)},
        Type_display_info{.type = "Nml", .rgb = from_hex(0xA8A77A)},
        Type_display_info{.type = "Psn", .rgb = from_hex(0xA33EA1)},
        Type_display_info{.type = "Psy", .rgb = from_hex(0xF95587)},
        Type_display_info{.type = "Rck", .rgb = from_hex(0xB6A136)},
        Type_display_info{.type = "Stl", .rgb = from_hex(0xB7B7CE)},
        Type_display_info{.type = "Wtr", .rgb = from_hex(0x6390F0)},
    };

    //////////////////////   Data Structures  /////////////////////////////////

    /// The state we must track to successfully use a Raylib drop down.
    Dropdown dst_map_select;

    /// Raylib Dropdowns use an active integer to track which option of a drop
    /// down is selected. So, it is helpful to store them contiguously
    /// corresponding to each active index.
    std::vector<std::string> dst_map_list;

    /// Raylib requires a semicolon separated string to display all options in
    /// the dropdown ("option1;option2"). The final options should not have a
    /// trailing semicolon.
    std::string dst_map_options;

    /// The choice of attack or defense and exact or overlapping coverage.
    Dropdown dlx_solver_select;

    /// We should only be rendering a solution while one has been requested.
    /// Otherwise, we might hitch the app every time the user changes the
    /// selection and we start solving right away. Some solutions take a long
    /// time.
    bool rendering_solution{false};

    /// The data from the current Pokemon generation map we have loaded in.
    /// This data structure tells us where all the gyms are for this
    /// generation on the map. It also tells us how all the types interact
    /// with one another in the form of a type map where the key is the type
    /// and the value is the set of defense multipliers against the single
    /// attack types in the game.
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

    /// We will keep an active set of the user's selections of gyms to solve the
    /// cover problem. We hide all items that need covering except for those
    /// corresponding to the types at the active gyms.
    ///
    /// If the set is empty we solve for the entire generation and all types.
    std::set<std::string_view> selected_gyms;

    /// The defensive dancing links solver. Loaded along with each new
    /// generation.
    Dx::Pokemon_links defense_dlx;

    /// The most recent solution to the defense dlx problem rendered every
    /// frame if non-empty and defense solutions was selected.
    std::set<Ranked_set<Dx::Type_encoding>> defense_solutions;

    /// The attack dancing links solver. Loaded along with each new generation.
    Dx::Pokemon_links attack_dlx;

    /// The most recent solution to the attack dlx problem rendered every
    /// frame if non-empty and defense solutions was selected.
    std::set<Ranked_set<Dx::Type_encoding>> attack_solutions;

    //////////////////////    Functions ///////////////////////////////////

    void draw_graph_cover(Rectangle canvas);
    void draw_controls(float minimap_width, float minimap_height);
    void reload_generation();
    static Dx::Point scale_point(Dx::Point const &p,
                                 Dx::Min_max const &x_data_bounds,
                                 Dx::Min_max const &x_draw_bounds,
                                 Dx::Min_max const &y_data_bounds,
                                 Dx::Min_max const &y_draw_bounds);
    static void draw_wrapping_message(Rectangle canvas,
                                      std::string_view message);
    static std::string_view
    get_token_with_trailing_delims(std::string_view view,
                                   std::string_view delim_set);
    static Color select_max_contrast_black_or_white(Color const &background);
    static void draw_type_node(Dx::Type_encoding type, float radius,
                               float center_x, float center_y);
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
        Font gb_font = LoadFontEx(font_path.data(), 64, nullptr, 0);
        GenTextureMipmaps(&gb_font.texture);
        GuiSetFont(gb_font);
        SetTargetFPS(60);
        Generation gen;

        while (!WindowShouldClose())
        {
            screen_width = GetScreenWidth();
            screen_height = GetScreenHeight();
            BeginDrawing();

            ClearBackground(RAYWHITE);

            gen.draw(screen_width, screen_height);

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

Generation::Generation()
    : dst_map_select(Dropdown{
          .dimensions = {},
          .active = 0,
          .editmode = false,
      }),
      dlx_solver_select(Dropdown{
          .dimensions = {},
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
        dst_map_list.emplace_back(file_name);
    }
    if (dst_map_list.empty())
    {
        return;
    }
    std::ranges::sort(dst_map_list,
                      [](std::string const &a, std::string const &b) {
                          return a.compare(b) < 0;
                      });
    for (size_t i = 0; i < dst_map_list.size() - 1; ++i)
    {
        dst_map_options.append(dst_map_list[i]).append(";");
    }
    dst_map_options.append(dst_map_list.back());
    reload_generation();
}

void
Generation::reload_generation()
{
    if (dst_map_select.active >= dst_map_list.size())
    {
        std::cerr << "Active Pokemon Generation selector out of range.\n";
        std::abort();
    }
    std::string const path = std::string(dst_relative_path)
                                 .append(dst_map_list[dst_map_select.active]);
    std::ifstream gen(path);
    if (gen.fail())
    {
        std::cerr << "Cannot load Pokemon Generation .dst file " << path
                  << '.\n';
        std::abort();
    }
    rendering_solution = false;
    // Clear out any subsets of problems we wanted to solve.
    selected_gyms.clear();
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

/// Draws the Pokemon generation minimap to the top left corner of the
/// screen. The minimap is scaled appropriately based on the dimensions of
/// the overall window so that the shape of a Pokemon region is visible,
/// roads connect gyms, and gyms act as selectable buttons to create subset
/// Dx solver problems.
///
/// The window has been allowed to be resizable so this draw call should occur
/// on every loop in case the window size is updated.
void
Generation::draw(int const window_width, int const window_height)
{

    auto const minimap_width = static_cast<float>(window_width);
    float const minimap_height
        = static_cast<float>(window_height) * scale_minimap_y_factor;
    // Layout the map and the drop down menu below it.
    DrawRectangleV(
        Vector2{
            .x = minimap_origin_x,
            .y = minimap_origin_y,
        },
        Vector2{
            .x = minimap_width,
            .y = minimap_height,
        },
        WHITE);

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
    for (auto &[city_location_map_iterator, active_toggle_state] : gym_toggles)
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
        bool const prev_state = active_toggle_state;
        GuiToggle(
            Rectangle{
                .height = minimap_button_size,
                .width = minimap_button_size,
                .x = scaled_coordinates.x - (minimap_button_size / 2),
                .y = scaled_coordinates.y - (minimap_button_size / 2),
            },
            city_location_map_iterator->first.c_str(), &active_toggle_state);
        if (active_toggle_state != prev_state)
        {
            rendering_solution = false;
            if (active_toggle_state)
            {
                selected_gyms.insert(city_location_map_iterator->first);
            }
            else
            {
                selected_gyms.erase(city_location_map_iterator->first);
            }
        }
    }
    draw_controls(minimap_width, minimap_height);
    draw_graph_cover(Rectangle{
        .width = static_cast<float>(window_width),
        .height = static_cast<float>(window_height) - minimap_height,
        .x = minimap_origin_x,
        .y = minimap_origin_y + minimap_height + minimap_button_size,
    });
}

void
Generation::draw_controls(float const minimap_width, float const minimap_height)
{
    float const button_width = minimap_width / 3;
    float const button_height = minimap_height * 0.08F;
    dst_map_select.dimensions = Rectangle{
        .width = button_width,
        .height = button_height,
        .x = minimap_origin_x,
        .y = minimap_origin_y + minimap_height,
    };
    dlx_solver_select.dimensions = Rectangle{
        .width = button_width,
        .height = button_height,
        .x = minimap_origin_x + dst_map_select.dimensions.width,
        .y = minimap_origin_y + minimap_height,
    };
    int const prev_map_selection = dst_map_select.active;
    if (GuiDropdownBox(dst_map_select.dimensions, dst_map_options.c_str(),
                       &dst_map_select.active, dst_map_select.editmode))
    {
        if (dst_map_select.active != prev_map_selection)
        {
            rendering_solution = false;
        }
        dst_map_select.editmode = !dst_map_select.editmode;
        if (!dst_map_select.editmode)
        {
            reload_generation();
        }
    }
    int const prev_solution_selction = dlx_solver_select.active;
    if (GuiDropdownBox(dlx_solver_select.dimensions, dlx_solver_options,
                       &dlx_solver_select.active, dlx_solver_select.editmode))
    {
        if (dlx_solver_select.active != prev_solution_selction)
        {
            rendering_solution = false;
        }
        dlx_solver_select.editmode = !dlx_solver_select.editmode;
    }
    if (GuiButton(
            Rectangle{
                .width = button_width,
                .height = button_height,
                .x = minimap_origin_x + dst_map_select.dimensions.width
                     + dlx_solver_select.dimensions.width,
                .y = minimap_origin_y + minimap_height,
            },
            "Solve!"))
    {
        rendering_solution = true;
        auto const dlx_active_solver
            = static_cast<Solution_request>(dlx_solver_select.active);
        Dx::reset_items(defense_dlx);
        Dx::reset_items(attack_dlx);
        if (!selected_gyms.empty())
        {
            std::set<Dx::Type_encoding> const subset_attack
                = Dx::load_selected_gyms_attacks(
                    dst_map_list[dst_map_select.active], selected_gyms);
            Dx::hide_items_except(defense_dlx, subset_attack);
            std::set<Dx::Type_encoding> const subset_defense
                = Dx::load_selected_gyms_defenses(
                    dst_map_list[dst_map_select.active], selected_gyms);
            Dx::hide_items_except(attack_dlx, subset_defense);
        }
        switch (dlx_active_solver)
        {
            case Solution_request::attack_exact_cover:
            case Solution_request::defense_exact_cover:
                attack_solutions
                    = Dx::exact_cover_stack(attack_dlx, attack_slots);
                defense_solutions
                    = Dx::exact_cover_stack(defense_dlx, pokemon_party_size);
                break;
            case Solution_request::attack_overlapping_cover:
            case Solution_request::defense_overlapping_cover:
                attack_solutions
                    = Dx::overlapping_cover_stack(attack_dlx, attack_slots);
                defense_solutions = Dx::overlapping_cover_stack(
                    defense_dlx, pokemon_party_size);
                break;
            default:
                std::cerr << "unmatchable dlx active solver\n";
                std::abort();
                break;
        }
    }
}

void
Generation::draw_graph_cover(Rectangle const canvas)
{
    auto const dlx_active_solver
        = static_cast<Solution_request>(dlx_solver_select.active);
    if (!rendering_solution)
    {
        draw_wrapping_message(canvas, graph_display_message);
        return;
    }
    std::set<Ranked_set<Dx::Type_encoding>> const *solution{};
    Dx::Pokemon_links const *dlx{};
    switch (dlx_active_solver)
    {
        case Solution_request::attack_exact_cover:
        case Solution_request::attack_overlapping_cover:
            solution = &attack_solutions;
            dlx = &attack_dlx;
            break;
        case Solution_request::defense_exact_cover:
        case Solution_request::defense_overlapping_cover:
            solution = &defense_solutions;
            dlx = &defense_dlx;
            break;
        default:
            return;
            break;
    }
    if (solution->empty())
    {
        draw_wrapping_message(canvas, no_solution_message);
        return;
    }
    float const node_radius = std::min(canvas.width, canvas.height) / 30.0F;
    size_t const solution_size = (*solution->begin()).size();
    // Every type in the solution is given a segment of a circle and will be
    // placed on intervals around a ring by steps of this segment in radians.
    float const theta_segment_angle
        = static_cast<float>(2.0F * std::numbers::pi)
          / static_cast<float>(solution_size);
    // We want all of our solution types to gather at the center of the screen
    // in the smallest ring possible without overlapping. The circle cord
    // segment that passes through two center points of type nodes on this
    // imaginary circle must be at least (2 * (radius of a node)).
    //
    // We can find the chord length between two points on a circle with the
    // chord length formula (chord = 2Rsin(theta / 2)). We know theta is our
    // angle we chose to evenly divide the circle across the number of nodes
    // in our solution (chord = 2Rsin((2PI / solution size) / 2)), which
    // simplifies to chord = 2Rsin(theta / 2). We know the chord length
    // we want is (r = 2 * radius of a node) so we get
    //
    // 2r = 2Rsin(theta / 2)
    // R = r / sin(theta / 2)
    //
    // where r is known, giving the perfect radius for the inner ring.
    float const inner_ring_radius
        = solution_size == 1
              ? 0
              : node_radius / std::sin(theta_segment_angle / 2.0F);
    float const center_x = canvas.x + (canvas.width / 2);
    float const center_y = canvas.y + (canvas.height / 2);
    // We can fill the circle clockwise from the North tip of circle.
    float cur_theta = std::numbers::pi / 2.0F;
    for (Dx::Type_encoding t : *solution->begin())
    {
        float i = 1;
        for (auto const covered : Dx::items_for(*dlx, t))
        {
            draw_type_node(
                covered, node_radius,
                ((inner_ring_radius + (i * node_radius)) * std::cos(cur_theta))
                    + center_x,
                ((inner_ring_radius + (i * node_radius)) * std::sin(cur_theta))
                    + center_y);
            ++i;
        }
        draw_type_node(t, node_radius,
                       (inner_ring_radius * std::cos(cur_theta)) + center_x,
                       (inner_ring_radius * std::sin(cur_theta)) + center_y);
        cur_theta += theta_segment_angle;
    }
}

void
Generation::draw_type_node(Dx::Type_encoding const type, float const radius,
                           float const center_x, float const center_y)
{
    Font const font = GuiGetFont();
    float const font_scaling
        = (2.0F * static_cast<float>(std::numbers::pi) * radius) / 1200.0F;
    auto const font_size = static_cast<float>(font.baseSize) * font_scaling;
    float const font_spacing = font_size * 0.2F;
    std::pair<uint64_t, std::optional<uint64_t>> const type_indices
        = type.decode_indices();
    Type_display_info const &type1 = type_colors.at(type_indices.first);
    Color const type1_color = select_max_contrast_black_or_white(type1.rgb);
    if (type_indices.second.has_value())
    {
        Type_display_info const &type2
            = type_colors.at(type_indices.second.value());
        Color const type2_color = select_max_contrast_black_or_white(type2.rgb);
        DrawCircleSector(
            Vector2{
                center_x,
                center_y,
            },
            radius, 360, 180, 100, type1.rgb);
        DrawCircleSector(
            Vector2{
                center_x,
                center_y,
            },
            radius, 0, 180, 100, type2.rgb);
        DrawTextEx(font, type1.type.data(),
                   Vector2{
                       .x = center_x - (radius * 0.5F),
                       .y = center_y - font_size,
                   },
                   font_size, font_spacing, type1_color);
        DrawTextEx(font, type2.type.data(),
                   Vector2{
                       .x = center_x - (radius * 0.5F),
                       .y = center_y + font_size,
                   },
                   font_size, font_spacing, type2_color);
    }
    else
    {
        DrawCircleV(
            Vector2{
                center_x,
                center_y,
            },
            radius, type1.rgb);
        DrawTextEx(font, type1.type.data(),
                   Vector2{
                       .x = center_x - (radius * 0.5F),
                       .y = center_y - (font_size / 2.0F),
                   },
                   font_size, font_spacing, type1_color);
    }
}

/// Attempts to display a helpful directions with word wrapping near the center
/// of the screen. Because solving cover problems can be CPU intensive and take
/// some time, we don't solve by default. The user must request a solution with
/// the solve button so a message helps facilitate that.
///
/// This function will respect some escaped characters in a string like the
/// usual newline or return characters (\n or \r). However, some will not be
/// rendered correctly such as escaped tabs (\t). For spaces just add the
/// appropriate number of spaces to the string rather than adding an escaped
/// tab.
void
Generation::draw_wrapping_message(Rectangle const canvas,
                                  std::string_view message)
{
    Font const font = GuiGetFont();
    // Leave these constants here because you should consider scaling the font
    // based on the canvas size, not hard coded values.
    float const start_x = canvas.x + (canvas.width / 30.0F);
    float const end_x = canvas.x + canvas.width - (canvas.width / 30.0F);
    float const end_y = canvas.y + canvas.height;
    float const font_scaling = std::min(canvas.width, canvas.height) / 2000.0F;
    auto const font_size = static_cast<float>(font.baseSize) * font_scaling;
    float const font_x_spacing = font_size * 0.2F;
    float const font_y_spacing = font_size * 0.8F;
    float cur_pos_x = start_x;
    float cur_pos_y = canvas.y + (canvas.height / 3.5F);

    /// Returns the codepoint and the glyph width of the specified character.
    auto const get_glyph_info
        = [=](char const *const c) -> std::pair<int, float> {
        int codepoint_byte_count = 0;
        int const codepoint = GetCodepoint(c, &codepoint_byte_count);
        int const glyph_index = GetGlyphIndex(font, codepoint);
        float const glyph_width
            = font.glyphs[glyph_index].advanceX == 0
                  ? font.recs[glyph_index].width * font_scaling
                  : static_cast<float>(font.glyphs[glyph_index].advanceX)
                        * font_scaling;
        return {
            codepoint,
            glyph_width,
        };
    };

    /// Returns the width of a word according the glyph width of each character
    /// and the added spacing we have specified between letters.
    auto const get_word_width = [=](std::string_view word) -> float {
        float word_width = 0;
        for (char const &c : word)
        {
            float const glyph_width = get_glyph_info(&c).second;
            word_width += glyph_width + font_x_spacing;
        }
        return word_width;
    };

    /// Advances x and y positions and returns if there is space for a new
    /// line below the current. Returns true if there is more space for new
    /// lines or false if the window is exhausted for space.
    auto const advance_new_line
        = [=](float &cur_pos_x, float &cur_pos_y) -> bool {
        cur_pos_y += (static_cast<float>(font.baseSize) * font_scaling)
                     + font_y_spacing;
        // If we would write off the screen no point in continuing.
        if (cur_pos_y > end_y)
        {
            return false;
        }
        cur_pos_x = start_x;
        return true;
    };

    std::string_view const delim_set(" \r\t\n\v\f");
    while (!message.empty())
    {
        std::string_view word
            = get_token_with_trailing_delims(message, delim_set);
        float word_width = get_word_width(word);
        if (cur_pos_x + word_width > end_x
            && !advance_new_line(cur_pos_x, cur_pos_y))
        {
            return;
        }
        for (char const &c : word)
        {
            if (c == '\n' || c == '\r' || c == '\r\n')
            {
                if (!advance_new_line(cur_pos_x, cur_pos_y))
                {
                    return;
                }
                continue;
            }
            std::pair<int, float> const glyph_info = get_glyph_info(&c);
            DrawTextCodepoint(font, glyph_info.first,
                              Vector2{
                                  .x = cur_pos_x,
                                  .y = cur_pos_y,
                              },
                              font_size, BLACK);
            cur_pos_x += glyph_info.second + font_x_spacing;
        }
        // The word we got back as a token may have skipped leading delimiters
        // in the original message so be sure to cut those off by using the
        // true underlying pointer arithmetic, not just word size.
        message.remove_prefix((word.data() + word.size()) - message.data());
    }
}

/// Tokenize a view into the first occurrence of a word before any delimiter
/// specified in the delimiter set. The original word is returned if no
/// delimiters are found. Leading delimiters in the set are skipped.
/// However, all found trailing delimiters are included in the returned
/// string view to aid in left justifying the text and preserving extra
/// spaces, returns, or other formatting.
std::string_view
Generation::get_token_with_trailing_delims(std::string_view view,
                                           std::string_view delim_set)
{
    size_t const skip_leading = view.find_first_not_of(delim_set);
    if (skip_leading == std::string_view::npos)
    {
        return view;
    }
    view.remove_prefix(skip_leading);
    size_t const first_found = view.find_first_of(delim_set);
    if (first_found == std::string_view::npos)
    {
        return view;
    }
    size_t const end_of_delims
        = view.substr(first_found, view.length() - first_found)
              .find_first_not_of(delim_set);
    if (end_of_delims == std::string_view::npos)
    {
        return view;
    }
    return view.substr(0, first_found + end_of_delims);
}

/// Given a color returns black or white given which will have better contrast
/// against the provided background color. This is to help display the text
/// over Pokemon type nodes, but the concept could be applied anywhere.
Color
Generation::select_max_contrast_black_or_white(Color const &background)
{
    float const luminance = (0.2126F * static_cast<float>(background.r))
                            + (0.7152F * static_cast<float>(background.g))
                            + (0.0722F * static_cast<float>(background.b));
    float const saturation
        = static_cast<float>(
              (std::max({background.r, background.g, background.b})
               - std::min({background.r, background.g, background.b})))
          / static_cast<float>(
              std::max({background.r, background.g, background.b}));
    auto text_color = WHITE;
    if (saturation <= 0.5 && luminance > 0.5)
    {
        text_color = BLACK;
    }
    return text_color;
}

Dx::Point
Generation::scale_point(Dx::Point const &p, Dx::Min_max const &x_data_bounds,
                        Dx::Min_max const &x_draw_bounds,
                        Dx::Min_max const &y_data_bounds,
                        Dx::Min_max const &y_draw_bounds)
{
    return Dx::Point{
        .x
        = (((p.x - x_data_bounds.min) / (x_data_bounds.max - x_data_bounds.min))
           * (x_draw_bounds.max - x_draw_bounds.min))
          + x_draw_bounds.min + minimap_origin_x,
        .y
        = (((p.y - y_data_bounds.min) / (y_data_bounds.max - y_data_bounds.min))
           * (y_draw_bounds.max - y_draw_bounds.min))
          + y_draw_bounds.min + minimap_origin_y,
    };
}

} // namespace
