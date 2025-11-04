///////////////////////////////////////////////////////////////////////////////
/// Author: Alexander Lopez
///
/// This program implements an interactive Dancing Links graph cover visualizer.
/// The user is able to load in any of 9 generation Pokemon maps and solve
/// various graph cover questions. These questions basically boil down to the
/// following:
///
///   - What team of at most 6 Pokemon can I choose to be resilient to any
///   attack type I will encounter in the game?
///   - Of my 24 attack slots for my 6 Pokemon, which attack types can I choose
///   to give them so that I am super effective against any defensive types I
///   will encounter in the game?
///
/// These questions can be answered for an entire game or a subset of maps to
/// choose from on the mini map. If solutions exist, the best will be shown as
/// the inner ring of attack or defensive types that covers all the requested
/// attack or defensive types that surround this inner ring. The inner ring has
/// edges that lead to the types that are covered. These edges are color coded
/// to indicate the quality of the solution.
///
/// A line is colored based on the multiplier of the type interaction. If the
/// inner ring is defensive types than the edges indicate what resistance
/// multiplier the defensive type receives against the type it covers. The
/// multiplier could be 0.5, 0.25, or 0.0. These are all good but obviously a
/// 0.0 multiplier is best because it means immunity from damage. If the inner
/// ring is attack types the edges indicate the damage multiplier these types do
/// against the defensive types they cover. The multiplier could be 2 or 4. Both
/// are good but a 4x multiplier is best.
///
/// The user can cycle through the top ranked solutions provided. All solutions
/// shown tied for the best rank among all solutions. Solutions are ranked based
/// on the quality of their beneficial multipliers against other types. When
/// ranks are tied, the smaller set wins. As the user cycles through the
/// solutions that tied with the highest rank, smaller solutions sets will be
/// shown first. The reasoning is that while building a Pokemon team or
/// selecting types for attack slots, using as few slots as possible frees up
/// space in the team for other purposes.
///
/// Hovering over nodes will show their full type names. If hovering over
/// covered nodes in the surrounding circle, the multiplier is indicated and the
/// text matches the edge color.
///////////////////////////////////////////////////////////////////////////////
#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
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
#include <tuple>
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

constexpr std::string_view const font_path = "data/font/PokemonGb-RAeo.ttf";
constexpr std::string_view const graph_idle_message
    = "Solution not yet requested. Configure dropdown menus then press the "
      "'Solve!' button.";
constexpr std::string_view const no_solution_message
    = "No solution exists for this cover problem. Try  different "
      "configurations, then press the 'Solve!' button.";

int run();

/// Helper lambda for building a static table of rgb colors for types at compile
/// time in the Generation colors table.
auto const from_hex = [](uint32_t const hex_code) -> Color {
    return Color{
        .r = static_cast<unsigned char>((hex_code & 0xFF0000U) >> 16),
        .g = static_cast<unsigned char>((hex_code & 0xFF00U) >> 8),
        .b = static_cast<unsigned char>(hex_code & 0xFF),
        .a = 0xFF,
    };
};

///////////////////////////////////////////////////////////////////////////////
///
/// Generation Map and UI Management
///
/// The Generation class manages UI elements, map loading, and solution
/// management when the user requests cover problems.
///
///////////////////////////////////////////////////////////////////////////////
class Generation {
  public:
    Generation();
    [[nodiscard]] bool is_solution_requested() const;
    [[nodiscard]] static Rectangle get_ui_canvas(int window_width,
                                                 int window_height);
    [[nodiscard]] std::optional<
        std::tuple<Dx::Pokemon_links const &,
                   std::vector<Ranked_set<Dx::Type_encoding>> const &, size_t>>
    get_current_solution() const;
    void set_current_solution(size_t cur);

    void draw_minimap(int window_width, int window_height);

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

    //////////////////////   Constants       //////////////////////////////////

    enum : uint8_t // NOLINT
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
    static constexpr std::string_view too_many_solutions_message
        = "Too many solutions! The leading solution is shown. The best "
          "solution is unknown. Click to close.";

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
    bool requesting_solution{false};

    /// If a cover problem generated too many solutions we can make a pop up
    /// to warn the user of this fact.
    bool rendering_too_many_solutions{false};

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
    std::vector<Ranked_set<Dx::Type_encoding>> best_defense_solutions;

    /// The attack dancing links solver. Loaded along with each new generation.
    Dx::Pokemon_links attack_dlx;

    /// The most recent solution to the attack dlx problem rendered every
    /// frame if non-empty and defense solutions was selected.
    std::vector<Ranked_set<Dx::Type_encoding>> best_attack_solutions;

    /// We will allow the user to cycle through equivalent solutions with the
    /// best rank, from smallest to largest. This state must be remembered
    /// somewhere when passing to the graph drawing code.
    size_t cur_solution{0};

    //////////////////////    Functions ///////////////////////////////////

    void draw_ui_controls(Rectangle const &minimap_canvas);
    void draw_too_many_solutions_message(Rectangle const &popup_canvas);
    void reload_generation();
    static Dx::Point scale_map_point(Dx::Point const &p,
                                     Dx::Min_max const &x_data_bounds,
                                     Dx::Min_max const &x_draw_bounds,
                                     Dx::Min_max const &y_data_bounds,
                                     Dx::Min_max const &y_draw_bounds);
    static Vector2 get_menu_button_size(float minimap_width,
                                        float minimap_height);
    static void
    move_best_solutions(std::vector<Ranked_set<Dx::Type_encoding>> &fill,
                        std::set<Ranked_set<Dx::Type_encoding>> &&move_from);
}; // class Generation

///////////////////////////////////////////////////////////////////////////////
///
/// Cover Solution Graph Drawing
///
/// The graph drawing class aims to be a stateless drawing method relying
/// heavily on mathematical concepts to consistently draw the graph cover
/// solutions in well-scaled visually pleasing ways.
///
/// Any internal tables or state should be constant and static so that only
/// one instance exists for the program of that data.
///
/// Try to maintain the constraint that all member functions are static and
/// there are not state data member variables.
///
///////////////////////////////////////////////////////////////////////////////
class Graph_draw {
  public:
    static size_t draw_graph_cover(
        Rectangle const &canvas, Dx::Pokemon_links const &dlx_solver,
        std::vector<Ranked_set<Dx::Type_encoding>> const &solutions,
        size_t cur_solution);

  private:
    static constexpr float default_line_thickness = 2.0;
    /// Nodes will use a backing color corresponding to a type as well as an
    /// abbreviation for the type name.
    static constexpr std::array<Color, 18> type_colors{
        from_hex(0xA6B91A), // Bug
        from_hex(0x705746), // Dark
        from_hex(0x6F35FC), // Dragon
        from_hex(0xF7D02C), // Electric
        from_hex(0xD685AD), // Fairy
        from_hex(0xC22E28), // Fighting
        from_hex(0xEE8130), // Fire
        from_hex(0xA98FF3), // Flying
        from_hex(0x735797), // Ghost
        from_hex(0x7AC74C), // Grass
        from_hex(0xE2BF65), // Ground
        from_hex(0x96D9D6), // Ice
        from_hex(0xA8A77A), // Normal
        from_hex(0xA33EA1), // Poison
        from_hex(0xF95587), // Psychic
        from_hex(0xB6A136), // Rock
        from_hex(0xB7B7CE), // Steel
        from_hex(0x6390F0), // Water
    };

    static constexpr std::array<std::string_view, 18> type_string_abbrev{
        "Bug", "Drk", "Drg", "Elc", "Fry", "Fgt", "Fir", "Fly", "Ghs",
        "Grs", "Gnd", "Ice", "Nml", "Psn", "Psy", "Rck", "Stl", "Wtr",
    };

    static constexpr std::array<Color, 7> multiplier_colors{
        Color{},                                       // empty
        Color{.r = 0, .g = 0, .b = 0, .a = 255},       // Immune
        Color{.r = 0, .g = 255, .b = 0, .a = 255},     // x0.25
        Color{.r = 0, .g = 0, .b = 255, .a = 255},     // x0.5
        Color{.r = 128, .g = 128, .b = 128, .a = 255}, // x1.0
        Color{.r = 212, .g = 175, .b = 55, .a = 255},  // x2.0
        Color{.r = 255, .g = 0, .b = 0, .a = 255},     // x4.0
    };

    static constexpr std::array<std::string_view, 7> multiplier_strings{
        "", "0.00x", "0.25x", "0.50x", "1.00x", "2.00x", "4.00x",
    };

    static Color select_max_contrast_black_or_white(Color const &background);
    static void draw_type_node(
        std::pair<std::string_view, std::optional<std::string_view>> const
            &type_string,
        std::pair<Color, std::optional<Color>> const &type_colors, float radius,
        float center_x, float center_y);
    static void draw_type_popup(Rectangle const &canvas,
                                Dx::Type_encoding inner_type,
                                Vector2 const &inner_point, float inner_radius,
                                Dx::Resistance outer_type, float outer_radius,
                                Vector2 const &outer_point);
    static void draw_directed_line(Vector2 const &inner_point,
                                   Dx::Resistance outer_type,
                                   float outer_radius,
                                   Vector2 const &outer_point, float thickness);
    static size_t draw_solution_navigation(Rectangle const &canvas,
                                           size_t num_solutions,
                                           size_t cur_solution);
    static std::pair<Color, std::optional<Color>>
    get_colors(std::pair<uint64_t, std::optional<uint64_t>> const &indices);
    static std::pair<std::string_view, std::optional<std::string_view>>
    get_string_abbreviation(
        std::pair<uint64_t, std::optional<uint64_t>> const &indices);
}; // class Graph

///////////////////////////////////////////////////////////////////////////////
///
///  Free Functions
///
///////////////////////////////////////////////////////////////////////////////

void draw_wrapping_message(Rectangle const &canvas, std::string_view message,
                           Color const &tint);
std::string_view get_token_with_trailing_delims(std::string_view view,
                                                std::string_view delim_set);

} // namespace

//////////////////////////     Main           /////////////////////////////////

int
main()
{
    return run();
}

///////////////////////////   Implementation   ////////////////////////////////

namespace {

/// Runs the map and ui manager while drawing solutions as they occur.
int
run()
{
    try
    {
        int screen_width = 800;
        int screen_height = 450;

        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(screen_width, screen_height,
                   "Dancing Links and Planning Pokemon");
        Font pokemon_gameboy_font
            = LoadFontEx(font_path.data(), 64, nullptr, 0);
        GenTextureMipmaps(&pokemon_gameboy_font.texture);
        GuiSetFont(pokemon_gameboy_font);
        SetTargetFPS(60);
        // The generation is a stateful object, unfortunately. But this keeps
        // all the necessary constants and tables in one class rather than
        // global state.
        Generation gen;
        while (!WindowShouldClose())
        {
            screen_width = GetScreenWidth();
            screen_height = GetScreenHeight();
            BeginDrawing();
            ClearBackground(WHITE);
            Rectangle const ui_canvas
                = Generation::get_ui_canvas(screen_width, screen_height);
            Rectangle const graph_canvas{

                .width = static_cast<float>(screen_width),
                .height = static_cast<float>(screen_height) - ui_canvas.height,
                .x = ui_canvas.x,
                .y = ui_canvas.y + ui_canvas.height,
            };
            if (gen.is_solution_requested())
            {
                std::optional<std::tuple<
                    Dx::Pokemon_links const &,
                    std::vector<Ranked_set<Dx::Type_encoding>> const &,
                    size_t>> const solution
                    = gen.get_current_solution();
                if (solution.has_value())
                {
                    auto const [dlx, solution_set, index] = solution.value();
                    // Drawing solutions requires no persistent state which is
                    // important for performance as this can be a hot path.
                    gen.set_current_solution(Graph_draw::draw_graph_cover(
                        graph_canvas, dlx, solution_set, index));
                }
                else
                {
                    draw_wrapping_message(graph_canvas, no_solution_message,
                                          RED);
                }
            }
            else
            {
                draw_wrapping_message(graph_canvas, graph_idle_message, BLACK);
            }
            // Even though the user interacts with the menu before solving
            // we want it drawn last so drop down menus cover graph solutions.
            gen.draw_minimap(screen_width, screen_height);
            EndDrawing();
        }
        CloseWindow();
        return 0;
    } catch (std::exception const &e)
    {
        std::cerr << "exception caught: " << e.what() << std::flush;
        return 1;
    }
}

///////////////////////////////////////////////////////////////////////////////
///
/// Generation Implementation
///
/// This section implements all the logic required to load maps and select
/// graph cover problems to solve. It holds these solutions and map settings
/// until they are ready to be handed to the graph visualizer.
///
/// All text is in the style of the original Pokemon games!
///
///////////////////////////////////////////////////////////////////////////////

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
    rendering_too_many_solutions = false;
    requesting_solution = false;
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
Generation::draw_minimap(int const window_width, int const window_height)
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
        Dx::Point const src = scale_map_point(
            node_info.coordinates, generation.gen_map.x_data_bounds,
            x_draw_bounds, generation.gen_map.y_data_bounds, y_draw_bounds);
        for (std::string const *edge : node_info.edges)
        {
            Dx::Map_node const dst_info = generation.gen_map.network.at(*edge);
            Dx::Point const dst = scale_map_point(
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
        Dx::Point const scaled_coordinates = scale_map_point(
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
            requesting_solution = false;
            rendering_too_many_solutions = false;
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
    draw_ui_controls(Rectangle{
        .width = minimap_width,
        .height = minimap_height,
        .x = minimap_origin_x,
        .y = minimap_origin_y,
    });
    draw_too_many_solutions_message(Rectangle{
        .width = minimap_width,
        .height = minimap_height,
        .x = minimap_origin_x,
        .y = minimap_origin_y,
    });
}

void
Generation::draw_ui_controls(Rectangle const &minimap_canvas)
{
    Vector2 const button_size
        = get_menu_button_size(minimap_canvas.width, minimap_canvas.height);
    dst_map_select.dimensions = Rectangle{
        .width = button_size.x,
        .height = button_size.y,
        .x = minimap_canvas.x,
        .y = minimap_canvas.y + minimap_canvas.height,
    };
    dlx_solver_select.dimensions = Rectangle{
        .width = button_size.x,
        .height = button_size.y,
        .x = minimap_canvas.x + dst_map_select.dimensions.width,
        .y = minimap_canvas.y + minimap_canvas.height,
    };
    int const prev_map_selection = dst_map_select.active;
    if (GuiDropdownBox(dst_map_select.dimensions, dst_map_options.c_str(),
                       &dst_map_select.active, dst_map_select.editmode))
    {
        if (dst_map_select.active != prev_map_selection)
        {
            requesting_solution = false;
            rendering_too_many_solutions = false;
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
            requesting_solution = false;
            rendering_too_many_solutions = false;
        }
        dlx_solver_select.editmode = !dlx_solver_select.editmode;
    }
    if (GuiButton(
            Rectangle{
                .width = button_size.x,
                .height = button_size.y,
                .x = minimap_canvas.x + dst_map_select.dimensions.width
                     + dlx_solver_select.dimensions.width,
                .y = minimap_canvas.y + minimap_canvas.height,
            },
            "Solve!"))
    {
        cur_solution = 0;
        requesting_solution = true;
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
        Dx::Pokemon_links const *solver{};
        switch (dlx_active_solver)
        {
            case Solution_request::attack_exact_cover:
                move_best_solutions(
                    best_attack_solutions,
                    Dx::exact_cover_stack(attack_dlx, attack_slots));
                solver = &attack_dlx;
                break;
            case Solution_request::defense_exact_cover:
                move_best_solutions(
                    best_defense_solutions,
                    Dx::exact_cover_stack(defense_dlx, pokemon_party_size));
                solver = &defense_dlx;
                break;
            case Solution_request::attack_overlapping_cover:
                move_best_solutions(
                    best_attack_solutions,
                    Dx::overlapping_cover_stack(attack_dlx, attack_slots));
                solver = &attack_dlx;
                break;
            case Solution_request::defense_overlapping_cover:
                move_best_solutions(best_defense_solutions,
                                    Dx::overlapping_cover_stack(
                                        defense_dlx, pokemon_party_size));
                solver = &defense_dlx;
                break;
            default:
                std::cerr << "unmatchable dlx active solver\n";
                std::abort();
                break;
        }
        if (Dx::has_max_solutions(*solver))
        {
            rendering_too_many_solutions = true;
        }
    }
}

/// Fills the vector with the solutions tied for the best rank.
/// Rank is determined first by the integer rank and then ties among rank are
/// broken by the smallest solution size winning.
void
Generation::move_best_solutions(
    std::vector<Ranked_set<Dx::Type_encoding>> &fill,
    std::set<Ranked_set<Dx::Type_encoding>> &&move_from)
{
    fill.clear();
    std::set<Ranked_set<Dx::Type_encoding>> moved = std::move(move_from);
    if (moved.empty())
    {
        return;
    }
    int const leading_rank = moved.begin()->rank();
    for (auto iter = moved.begin(); iter != moved.end();)
    {
        if (leading_rank != iter->rank())
        {
            return;
        }
        auto next = std::next(iter);
        auto extraced = moved.extract(iter);
        fill.emplace_back(std::move(extraced.value()));
        iter = next;
    }
}

/// Draws a warning to the user that the shown solution may not be the best
/// among candidates due to how many solutions were generated. The solver
/// may cut off solution generation due to the problem space of some solutions.
///
/// Overlapping cover especially can have so many solutions they are not
/// countable on a human time scale.
void
Generation::draw_too_many_solutions_message(Rectangle const &popup_canvas)
{
    if (!rendering_too_many_solutions)
    {
        return;
    }
    if (CheckCollisionPointRec(GetMousePosition(), popup_canvas)
        && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        rendering_too_many_solutions = false;
        return;
    }
    float const border
        = std::max(popup_canvas.width, popup_canvas.height) * 0.01F;
    GuiDrawRectangle(popup_canvas, static_cast<int>(border), RED, WHITE);
    draw_wrapping_message(
        Rectangle{
            .width = popup_canvas.width - border,
            .height = popup_canvas.height - border,
            .x = popup_canvas.x + border,
            .y = popup_canvas.y + border,
        },
        too_many_solutions_message, RED);
}

/// There are many possible combinations of solver request and solution so
/// we will handle that complexity here so drawing code does not care.
///
/// None is returned if no solution is ready.
std::optional<
    std::tuple<Dx::Pokemon_links const &,
               std::vector<Ranked_set<Dx::Type_encoding>> const &, size_t>>
Generation::get_current_solution() const
{
    auto const dlx_active_solver
        = static_cast<Solution_request>(dlx_solver_select.active);
    if (!requesting_solution)
    {
        return {};
    }
    std::vector<Ranked_set<Dx::Type_encoding>> const *solution{};
    Dx::Pokemon_links const *dlx{};
    switch (dlx_active_solver)
    {
        case Solution_request::attack_exact_cover:
        case Solution_request::attack_overlapping_cover:
            solution = &best_attack_solutions;
            dlx = &attack_dlx;
            break;
        case Solution_request::defense_exact_cover:
        case Solution_request::defense_overlapping_cover:
            solution = &best_defense_solutions;
            dlx = &defense_dlx;
            break;
        default:
            return {};
            break;
    }
    if (solution->empty())
    {
        return {};
    }
    return {{*dlx, *solution, cur_solution}};
}

void
Generation::set_current_solution(size_t const cur)
{
    if (get_current_solution().has_value())
    {
        cur_solution = cur;
    }
}

/// Returns how much space is taken by the mini map and menu UI button
/// elements.
///
/// It is safe to draw anywhere outside of this canvas without interfering with
/// the use of the UI.
Rectangle
Generation::get_ui_canvas(int const window_width, int const window_height)
{
    Rectangle res{
        .width = static_cast<float>(window_width),
        .height = (static_cast<float>(window_height) * scale_minimap_y_factor),
        .x = minimap_origin_x,
        .y = minimap_origin_y,
    };
    res.height += get_menu_button_size(res.width, res.height).y;
    return res;
}

/// Given how large the mini map is returns the intended button size.
Vector2
Generation::get_menu_button_size(float minimap_width, float minimap_height)
{
    return {
        .x = minimap_width / 3.0F,
        .y = minimap_height * 0.08F,
    };
}

/// Scales a gym on the map appropriately so it takes as much space as possible
/// while maintaining the original graph shape construction.
Dx::Point
Generation::scale_map_point(Dx::Point const &p,
                            Dx::Min_max const &x_data_bounds,
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

/// Returns true if the solution should be rendered.
bool
Generation::is_solution_requested() const
{
    return requesting_solution;
}

///////////////////////////////////////////////////////////////////////////////
///
///  Graph Drawing
///
///  Graph drawing attempts to visually lay out the solution as an annulus or
///  two concentric rings. Imagine a circle surrounding these lines that form
///  segments.
///                           \  |  /
///                            \ | /
///                             \|/
///                        ---------------
///                             /|\
///                            / | \
///                           /  |  \
///
///  To make things simple the goal of this class is to place the solution
///  set on the inner ring of the annulus and then make the nodes that the
///  solution covers fill the area of the circle segment of the outer ring.
///
///  This part of the program probably has much more advanced graph drawing
///  algorithms we could try but the one I came up with is very simple, fast,
///  and looks pretty good. Check back for updates or put in a PR if you know
///  a better one.
///
///////////////////////////////////////////////////////////////////////////////

/// Draws a graph cover illustration placing the types the are in the highest
/// ranked set in an inner ring. Each solution node is given an equivalent
/// angle slice of a hypothetical circle. They will then fill their dedicated
/// segment with the nodes that they cover.
///
/// The nodes that are covered try their best to fill the available space they
/// have so that they are readable. This means nodes will be different sizes
/// which I find fun. Might be confusing if people think it means importance
/// but I think it provides good visual balance. Research alternatives.
size_t
Graph_draw::draw_graph_cover(
    Rectangle const &canvas, Dx::Pokemon_links const &dlx_solver,
    std::vector<Ranked_set<Dx::Type_encoding>> const &solutions,
    size_t cur_solution)
{
    float const node_radius = std::min(canvas.width, canvas.height) * 0.06F;
    size_t const solution_size = solutions.at(cur_solution).size();
    float const center_x = canvas.x + (canvas.width / 2);
    float const center_y = canvas.y + (canvas.height / 2);
    // Every type in the solution is given a segment of a circle and will be
    // placed on intervals around a ring by steps of this segment in radians.
    // If there is only one node, give it half the circle for its coverage. This
    // covers display and drawing edge cases.
    float const theta_segment_angle
        = (2.0F * std::numbers::pi_v<float>)
          / static_cast<float>(solution_size == 1 ? 2 : solution_size);
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
    float const inner_ring_node_center_radius
        = solution_size == 1
              ? 0.0F
              : node_radius / std::sin(theta_segment_angle * 0.5F);
    // While the inner nodes sit directly on the previously calculated radius,
    // we need an exclusive boundary for the circle segment that will hold the
    // types each inner ring type covers.
    float const inner_ring_annulus_radius
        = inner_ring_node_center_radius + node_radius;
    // We also have an imaginary outer circle that bounds the nodes that each
    // type in our solution covers. Each type is given a segment of this circle
    // to fill in with the other types that it covers. We will make this look
    // like a directed graph with lines extending from the inner ring nodes
    // to cover nodes distributed in this circle segment.
    //
    // The size of the nodes we cover will be adjusted to maximally fill the
    // provided segment. Together these circles form an annulus and we are
    // going to fill the area of the annulus segment wedge with the covered
    // nodes so that they do not overlap with the inner ring. The inner ring of
    // the annulus will encompass the inner ring of solution nodes.
    float const outer_ring_annulus_radius
        = std::min(canvas.width, canvas.height) * 0.45F;
    float const annulus_radius_squared_difference
        = (outer_ring_annulus_radius * outer_ring_annulus_radius)
          - (inner_ring_annulus_radius * inner_ring_annulus_radius);
    // We can fill the circle clockwise from the North tip of circle.
    float const start_theta = std::numbers::pi_v<float> * 0.5F;
    float cur_theta = start_theta;
    // There are at least three passes over the same data that must occur and
    // it is important not to allocate and free any memory in such a hot loop
    // if we can help it. So we use a c-macro like approach but with C++
    // lambdas and functions.
    //
    // Lines between nodes must be drawn first, then nodes, than any hover pop
    // up windows. We use the same exact math for placing these elements on
    // the screen in each iteration and only differ in what we draw and how
    // we use the coordinates. So accept a draw function and use the same
    // iteration and calculation pattern every time. A few cycles for the math
    // used to calculate the coordinates on our circles is better than
    // going to memory to record and retrieve all the point data and interacting
    // with the heap at construction and deconstruction.
    //
    // I don't like the current approach but we have to calculate so many
    // local variables and use them slightly differently on every pass so
    // it is hard to unify the logic.
    auto const for_each_annulus_point = [&](Dx::Type_encoding const &inner_type,
                                            auto &&draw_fn) {
        Vector2 const inner_ring_node{
            .x
            = (inner_ring_node_center_radius * std::cos(cur_theta)) + center_x,
            .y
            = (inner_ring_node_center_radius * std::sin(cur_theta)) + center_y,
        };
        auto const n
            = static_cast<float>(Dx::items_count_for(dlx_solver, inner_type));
        // This is based on circle packing. However, I don't want to mess with
        // circle packing. It's hard. So we use the estimated density ratio of
        // a hexagonal grid (pi / 2sqrt(3)) to determine our circle radius.
        //
        //                     pi      theta(R² - r²)
        //   n * pi * r²  <=  ----- * --------------
        //                     2√3         2
        //
        //   r <= sqrt( (theta(R² - r²)) / (4n√3) )
        //
        // Where theta is our angle allotment for this inner node to cover the
        // outer nodes. Then, instead of 4n√3 in the denominator we just make
        // it a constant we can bump up manually to make the nodes smaller if
        // needed. I think determining window size can be a little buggy on
        // raylib so we have to adjust manually.
        float const covered_node_radius
            = sqrt((theta_segment_angle * annulus_radius_squared_difference)
                   / (14.0F * n));
        float const theta_end = cur_theta + theta_segment_angle;
        // Conceptually the radius is our row and the theta angle is our col
        // that helps determine where the next node should be drawn.
        float radius_row
            = outer_ring_annulus_radius - (covered_node_radius * 0.6F);
        // This starting column is nudged inside our angle allotment by one
        // covered node circle radius using right triangle identity
        //
        // theta_col = 2 * asin(opposite / hypotenuse)
        //
        // where the right triangle has formed at 1/2 the length of a cord
        // equal to the radius of our covered node.
        float theta_col
            = cur_theta
              + (2.0F * std::asin((covered_node_radius * 0.5F) / radius_row));
        for (Dx::Pokemon_links::Poke_link const *iter
             = Dx::items_for_begin(dlx_solver, inner_type);
             iter != Dx::items_for_end();
             iter = Dx::items_for_next(dlx_solver, iter))
        {
            Dx::Resistance const cur_covered
                = Dx::item_resistance_from(dlx_solver, iter);
            draw_fn(inner_ring_node,
                    Vector2{
                        .x = (std::cos(theta_col) * radius_row) + center_x,
                        .y = (std::sin(theta_col) * radius_row) + center_y,
                    },
                    covered_node_radius, inner_type, cur_covered);
            // Here we step by 2 covered node radii so that nodes don't overlap
            // with the right triangle identity
            //
            // theta_step = 2 * asin(opposite / hypotenuse)
            //
            // where the right triangle has formed at 1/2 the length of the
            // desired cord, in this case one radius of a covered node. Then
            // we step by two of those angle increments so no overlap.
            float const angle_of_node_radius
                = std::asin(covered_node_radius / radius_row);
            theta_col += 2.0F * angle_of_node_radius;
            // Double check that drawing a new circle at this position would
            // not encroach on the next segments allotted space.
            if (theta_col + angle_of_node_radius > theta_end)
            {
                radius_row -= (2.0F * covered_node_radius);
                // Reset to the starting angle of the pie slice.
                theta_col = cur_theta
                            + (2.0F
                               * std::asin((covered_node_radius * 0.5F)
                                           / radius_row));
            }
        }
    };

    // Drawing lines just connects two points at their true center locations.
    auto const draw_line
        = [](Vector2 const &inner_point, Vector2 const &outer_point,
             float const outer_radius,
             [[maybe_unused]] Dx::Type_encoding const inner_type,
             Dx::Resistance const &outer_type) {
              draw_directed_line(inner_point, outer_type, outer_radius,
                                 outer_point, default_line_thickness);
          };
    // A node has much more complex drawing logic, text, and possibly two
    // colors so it will deal with its own function.
    auto const draw_outer_node
        = []([[maybe_unused]] Vector2 const &inner_point,
             Vector2 const &outer_point, float const outer_radius,
             [[maybe_unused]] Dx::Type_encoding const inner_type,
             Dx::Resistance const &outer_type) {
              std::pair<uint64_t, std::optional<uint64_t>> const
                  outer_type_indices
                  = outer_type.type().decode_indices();
              draw_type_node(get_string_abbreviation(outer_type_indices),
                             get_colors(outer_type_indices), outer_radius,
                             outer_point.x, outer_point.y);
          };
    // Final layer is a pop up zoom-like feature that gives the full name of
    // a type node if we hover over with the mouse.
    auto const draw_popup
        = [&canvas, &node_radius]([[maybe_unused]] Vector2 const &inner_point,
                                  Vector2 const &outer_point,
                                  float const outer_radius,
                                  Dx::Type_encoding const inner_type,
                                  Dx::Resistance const &outer_type) {
              draw_type_popup(canvas, inner_type, inner_point, node_radius,
                              outer_type, outer_radius, outer_point);
          };

    // Use our macro like for each loop.
    for (Dx::Type_encoding const &t : solutions.at(cur_solution))
    {
        for_each_annulus_point(t, draw_line);
        cur_theta += theta_segment_angle;
    }
    // It would not be a problem for theta to continue incrementing but reset
    // just to be safe as I'm not sure how Raylib handles multiple rotations
    // around the unit circle.
    cur_theta = start_theta;
    for (Dx::Type_encoding const &t : solutions.at(cur_solution))
    {
        for_each_annulus_point(t, draw_outer_node);
        // Draw the inner ring last just in case. It is the most important
        // visual element.
        std::pair<uint64_t, std::optional<uint64_t>> const inner_node_indices
            = t.decode_indices();
        draw_type_node(
            get_string_abbreviation(inner_node_indices),
            get_colors(inner_node_indices), node_radius,
            (inner_ring_node_center_radius * std::cos(cur_theta)) + center_x,
            (inner_ring_node_center_radius * std::sin(cur_theta)) + center_y);
        cur_theta += theta_segment_angle;
    }
    cur_theta = start_theta;
    // It can be nice for large solution sets to be able to hover over small
    // covered nodes and see the full type name.
    for (Dx::Type_encoding const &t : solutions.at(cur_solution))
    {
        for_each_annulus_point(t, draw_popup);
        Vector2 const point{
            .x
            = (inner_ring_node_center_radius * std::cos(cur_theta)) + center_x,
            .y
            = (inner_ring_node_center_radius * std::sin(cur_theta)) + center_y,
        };
        draw_type_popup(canvas, t, point, node_radius,
                        Dx::Resistance(t, Dx::Multiplier::emp), node_radius,
                        point);
        cur_theta += theta_segment_angle;
    }
    return draw_solution_navigation(canvas, solutions.size(), cur_solution);
}

/// Draws a Bezier curve between nodes with an arrow indicating the directed
/// graph relationship.
void
Graph_draw::draw_directed_line(Vector2 const &inner_point,
                               Dx::Resistance const outer_type,
                               float const outer_radius,
                               Vector2 const &outer_point,
                               float const thickness)
{
    float const angle = std::atan2(outer_point.y - inner_point.y,
                                   outer_point.x - inner_point.x);
    float const cos_angle = std::cos(angle);
    float const sin_angle = std::sin(angle);
    float const arrow_length = thickness * 3;
    Vector2 const arrow_tip = {
        outer_point.x - (cos_angle * (outer_radius + arrow_length)),
        outer_point.y - (sin_angle * (outer_radius + arrow_length)),
    };
    Color const &color
        = multiplier_colors.at(static_cast<size_t>(outer_type.multiplier()));
    DrawLineEx(inner_point, arrow_tip, thickness, color);
    DrawPoly(arrow_tip, 3, arrow_length, angle * RAD2DEG, color);
}

/// Draws a large readable version of a type node with the full type name
/// rather than an abbreviation. The pop up also detects if it will run off
/// screen and adjusts accordingly.
///
/// Pass in the same point twice if it is an inner node with no line or
/// multiplier that needs to be drawn.
void
Graph_draw::draw_type_popup(Rectangle const &canvas,
                            Dx::Type_encoding const inner_type,
                            Vector2 const &inner_point,
                            float const inner_radius,
                            Dx::Resistance const outer_type,
                            float const outer_radius,
                            Vector2 const &outer_point)
{
    Vector2 const mouse = GetMousePosition();
    if (!CheckCollisionPointCircle(mouse,
                                   Vector2{
                                       .x = outer_point.x,
                                       .y = outer_point.y,
                                   },
                                   outer_radius))
    {
        return;
    }
    bool const is_outer_popup
        = inner_point.x != outer_point.x || inner_point.y != outer_point.y;
    if (is_outer_popup)
    {
        draw_directed_line(inner_point, outer_type, outer_radius, outer_point,
                           default_line_thickness * 3);
        auto const inner_indices = inner_type.decode_indices();
        auto const outer_indices = outer_type.type().decode_indices();
        draw_type_node(get_string_abbreviation(outer_indices),
                       get_colors(outer_indices), outer_radius, outer_point.x,
                       outer_point.y);
        draw_type_node(get_string_abbreviation(inner_indices),
                       get_colors(inner_indices), inner_radius, inner_point.x,
                       inner_point.y);
    }
    Vector2 const popup_bounding_square{
        .x = std::min(canvas.width, canvas.height) / 4.0F,
        .y = std::min(canvas.width, canvas.height) / 4.0F,
    };
    // We will flip the pop up so it doesn't run off screen. Only a concern
    // as we increase toward higher x and y because at the top of the screen
    // the text box always starts from mouse and goes down.
    Vector2 point_origin = mouse;
    if (((mouse.x - canvas.x) + popup_bounding_square.x > canvas.width)
        || ((mouse.y - canvas.y) + popup_bounding_square.y > canvas.height))
    {
        point_origin.x = mouse.x - popup_bounding_square.x;
        point_origin.y = mouse.y - popup_bounding_square.y;
    }
    Vector2 const popup_circle_center{
        point_origin.x + (popup_bounding_square.x / 2.0F),
        point_origin.y + (popup_bounding_square.y / 2.0F),
    };
    float const popup_node_radius = popup_bounding_square.x / 2.0F;
    draw_type_node(outer_type.type().decode_type(),
                   get_colors(outer_type.type().decode_indices()),
                   popup_node_radius, popup_circle_center.x,
                   popup_circle_center.y);
    if (is_outer_popup)
    {
        auto const multiplier_index
            = static_cast<size_t>(outer_type.multiplier());
        std::string_view const multiplier_string
            = multiplier_strings.at(multiplier_index);
        Font const font = GuiGetFont();
        auto const base_size = static_cast<float>(font.baseSize);
        float font_spacing = base_size * 0.2F;
        Vector2 const measured_dimensions = MeasureTextEx(
            font, multiplier_string.data(), base_size, font_spacing);
        float const font_scaling
            = (popup_bounding_square.x * 0.5F)
              / std::max(measured_dimensions.x, measured_dimensions.y);
        float const font_size = base_size * font_scaling;
        font_spacing = font_size * 0.2F;
        DrawTextEx(
            font, multiplier_strings.at(multiplier_index).data(),
            Vector2{
                .x = (popup_node_radius
                      * std::cos(5.0F * (std::numbers::pi_v<float>) / 4.0F))
                     + popup_circle_center.x,
                .y = (popup_node_radius
                      * std::sin(5.0F * (std::numbers::pi_v<float>) / 4.0F))
                     + popup_circle_center.y,
            },
            font_size, font_spacing, multiplier_colors.at(multiplier_index));
    }
}

/// Draws a node for a Pokemon type. The provided text is the type name which
/// may be a dual type. The colors represent the RGB color of the background,
/// possibly split in two for dual types. This function will try to make the
/// text as large as possible within the node so that it still fits.
void
Graph_draw::draw_type_node(
    std::pair<std::string_view, std::optional<std::string_view>> const
        &type_string,
    std::pair<Color, std::optional<Color>> const &type_colors,
    float const radius, float const center_x, float const center_y)
{
    Font const font = GuiGetFont();
    auto const base_size = static_cast<float>(font.baseSize);
    float font_spacing = base_size * 0.2F;
    std::string_view max_string = type_string.first;
    if (type_string.second.has_value()
        && max_string.length() < type_string.second.value().length())
    {
        max_string = type_string.second.value();
    }
    Vector2 const measured_dimensions
        = MeasureTextEx(font, max_string.data(), base_size, font_spacing);
    // We find the imaginary largest bounding square that fits inside of a
    // circle which has the diagonal of the circle diameter and side length
    // of radius * sqrt(2). Then ensure what we measured fits in square.
    float const font_scaling
        = (radius * std::numbers::sqrt2_v<float>)
          / std::max(measured_dimensions.x, measured_dimensions.y);
    float const font_size = base_size * font_scaling;
    font_spacing = font_size * 0.2F;
    Color const type1_text_color
        = select_max_contrast_black_or_white(type_colors.first);
    if (type_string.second.has_value() && type_colors.second.has_value())
    {
        Color const type2_text_color
            = select_max_contrast_black_or_white(type_colors.second.value());
        DrawCircleSector(
            Vector2{
                center_x,
                center_y,
            },
            radius, 360, 180, 100, type_colors.first);
        DrawCircleSector(
            Vector2{
                center_x,
                center_y,
            },
            radius, 0, 180, 0, type_colors.second.value());
        DrawTextEx(font, type_string.first.data(),
                   Vector2{
                       .x = center_x - (radius * 0.8F),
                       .y = center_y - font_size,
                   },
                   font_size, font_spacing, type1_text_color);
        DrawTextEx(font, type_string.second.value().data(),
                   Vector2{
                       .x = center_x - (radius * 0.8F),
                       .y = center_y + (font_size / 2.0F),
                   },
                   font_size, font_spacing, type2_text_color);
    }
    else
    {
        DrawCircleV(
            Vector2{
                center_x,
                center_y,
            },
            radius, type_colors.first);
        DrawTextEx(font, type_string.first.data(),
                   Vector2{
                       .x = center_x - (radius * 0.8F),
                       .y = center_y - (font_size / 2.0F),
                   },
                   font_size, font_spacing, type1_text_color);
    }
}

size_t
Graph_draw::draw_solution_navigation(Rectangle const &graph_canvas,
                                     size_t const num_solutions,
                                     size_t cur_solution)
{
    assert(num_solutions);
    float const button_size
        = std::min(graph_canvas.width, graph_canvas.height) * 0.05F;
    Rectangle const prev_button{
        .width = button_size,
        .height = button_size,
        .x = graph_canvas.x,
        .y = graph_canvas.y,
    };
    Rectangle const next_button{
        .width = button_size,
        .height = button_size,
        .x = graph_canvas.x + button_size,
        .y = graph_canvas.y,
    };
    if (GuiButton(prev_button, GuiIconText(ICON_PLAYER_PLAY_BACK, "")))
    {
        cur_solution = cur_solution ? cur_solution - 1 : num_solutions - 1;
    }
    else if (GuiButton(next_button, GuiIconText(ICON_PLAYER_PLAY, "")))
    {
        ++cur_solution %= num_solutions;
    }
    // Maximum digits possible for 64 bit integer + null term.
    std::array<char, 20 + 1> cur_solution_string{};
    size_t digits = cur_solution + 1;
    size_t placed = 0;
    while (digits && placed < cur_solution_string.size())
    {
        cur_solution_string.at(placed)
            = static_cast<char>((digits % 10U) + '0');
        digits /= 10;
        ++placed;
    }
    std::reverse(cur_solution_string.begin(),
                 cur_solution_string.begin() + placed);
    if (placed == cur_solution_string.size())
    {
        cur_solution_string.back() = '\0';
    }
    else
    {
        cur_solution_string.at(placed) = '\0';
    }
    Font const font = GuiGetFont();
    auto const base_size = static_cast<float>(font.baseSize);
    float font_spacing = base_size * 0.2F;
    Vector2 const measured_dimensions = MeasureTextEx(
        font, cur_solution_string.data(), base_size, font_spacing);
    float const font_scaling
        = (button_size * 2)
          / std::max(measured_dimensions.x, measured_dimensions.y);
    float const font_size = base_size * font_scaling;
    font_spacing = font_size * 0.2F;
    DrawTextEx(font, cur_solution_string.data(),
               Vector2{
                   .x = graph_canvas.x,
                   .y = graph_canvas.y + button_size,
               },
               font_size, font_spacing, BLACK);
    return cur_solution;
}

/// Find the appropriate colors in the color table for a single or dual type.
std::pair<Color, std::optional<Color>>
Graph_draw::get_colors(
    std::pair<uint64_t, std::optional<uint64_t>> const &indices)
{
    return {
        type_colors.at(indices.first),
        indices.second.has_value() ? type_colors.at(indices.second.value())
                                   : std::optional<Color>{},
    };
}

/// Find the appropriate string abbreviation for display node type names.
std::pair<std::string_view, std::optional<std::string_view>>
Graph_draw::get_string_abbreviation(
    std::pair<uint64_t, std::optional<uint64_t>> const &indices)
{
    return {
        type_string_abbrev.at(indices.first),
        indices.second.has_value()
            ? type_string_abbrev.at(indices.second.value())
            : std::optional<std::string_view>{},
    };
}

/// Given a color returns black or white given which will have better contrast
/// against the provided background color. This is to help display the text
/// over Pokemon type nodes, but the concept could be applied anywhere.
Color
Graph_draw::select_max_contrast_black_or_white(Color const &background)
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

///////////////////////////////////////////////////////////////////////////////
///
/// Free Function Implementations
///
///////////////////////////////////////////////////////////////////////////////

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
draw_wrapping_message(Rectangle const &canvas, std::string_view message,
                      Color const &tint)
{
    Font const font = GuiGetFont();
    // Leave these constants here because you should consider scaling the font
    // based on the canvas size, not hard coded values.
    float const start_x = canvas.x + (canvas.width * 0.01F);
    float const end_x = canvas.x + canvas.width;
    float const end_y = canvas.y + canvas.height;
    float const font_scaling = std::min(canvas.width, canvas.height) / 1700.0F;
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
        std::string_view const word
            = get_token_with_trailing_delims(message, delim_set);
        float const word_width = get_word_width(word);
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
            auto const [codepoint, glyph_width] = get_glyph_info(&c);
            DrawTextCodepoint(font, codepoint,
                              Vector2{
                                  .x = cur_pos_x,
                                  .y = cur_pos_y,
                              },
                              font_size, tint);
            cur_pos_x += glyph_width + font_x_spacing;
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
get_token_with_trailing_delims(std::string_view view,
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

} // namespace
