/// Author: Alexander G. Lopez
/// File: pokemon_cli.cc
/// ---------------------
/// This program is a minimal demo to get you up and running with dancing links
/// solutions to "real world," or in this case "video game world" data. Progress
/// on The graph cover visualizer with OpenGL is slow and I needed to test some
/// changes I made to dancing links algorithms so here is the program to do it.
/// It is not bad as it now has some color output which is nice. Use it as
/// follows.
///
/// Request an exact defensive cover of the entire Pokemon generation on a
/// certain map. This is the default.
///
/// ./build/bin/pokemon_cli data/dist/Gen-9-Paldea.dst
///
/// The above command answers the question: given at most 6 Pokemon what is the
/// minimum number I can use to have better than normal defense from every type
/// in the game? Request the Attack coverage instead in this form.
///
/// ./build/bin/pokemon_cli data/dist/Gen-9-Paldea.dst A
///
/// This answers the following question: given 24 attack slots across a team of
/// 6 pokemon which attack types can I choose to be super effective against
/// every type in the game? Ask for any subset of gyms for a given generation as
/// follows.
///
/// ./build/bin/pokemon_cli data/dist/Gen-9-Paldea.dst G1 G2 G4
///
/// The above format asks the following question: given a subset of gyms how do
/// I defensively cover myself from the attack types present in those gyms? You
/// can ask the attack version by adding an A to your arguments. Finally, you
/// can specify an exact or overlapping cover. By default this program tries to
/// find exact covers to these pokemon typing questions meaning that at every
/// option we use to cover all the items covers the items such that each item is
/// covered exactly once. This is hard to do in many maps. Instead, if you want
/// a blanket approach that simply seeks to cover all the items even if multiple
/// options cover the same items add the "O" flag as follows.
///
/// ./build/bin/pokemon_cli data/dist/Gen-9-Paldea.dst G1 G2 G4 O
///
/// This will generate many solutions because this is a much looser constraint
/// to apply to cover problems. Solutions will be cut off at 200,000 and your
/// terminal will likely not display anywhere near the full set. Run this
/// program from the root of the code base where the CMakePresets.json file is.
/// Enjoy!
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

import dancing_links;

namespace Dx = Dancing_links;
namespace {

constexpr int max_name_width = 17;
constexpr int digit_width = 3;

constexpr std::string_view nil = "\033[0m";
constexpr std::string_view ansi_yel = "\033[38;5;11m";
constexpr std::string_view ansi_red = "\033[38;5;9m";
constexpr std::string_view ansi_grn = "\033[38;5;10m";
// Obtained from hex color gist:
// https://gist.github.com/apaleslimghost/0d25ec801ca4fc43317bcff298af43c3
constexpr std::array<std::string_view, 18> type_colors = {
    // "Bug",
    "\033[38;2;166;185;26m",
    // "Dark",
    "\033[38;2;112;87;70m",
    // "Dragon",
    "\033[38;2;111;53;252m",
    // "Electric",
    "\033[38;2;247;208;44m",
    // "Fairy",
    "\033[38;2;214;133;173m",
    // "Fighting",
    "\033[38;2;194;46;40m",
    // "Fire",
    "\033[38;2;238;129;48m",
    // "Flying",
    "\033[38;2;169;143;243m",
    // "Ghost",
    "\033[38;2;115;87;151m",
    // "Grass",
    "\033[38;2;122;199;76m",
    // "Ground",
    "\033[38;2;226;191;101m",
    // "Ice",
    "\033[38;2;150;217;214m",
    // "Normal",
    "\033[38;2;168;167;122m",
    // "Poison",
    "\033[38;2;163;62;161m",
    // "Psychic",
    "\033[38;2;249;85;135m",
    // "Rock",
    "\033[38;2;182;161;54m",
    // "Steel",
    "\033[38;2;183;183;206m",
    // "Water",
    "\033[38;2;99;144;240m",
};

constexpr auto help_msg =
    R"(Pokemon CLI Usage:
    h                - Read this help message.
    plain            - Print without colors. Useful for piping or redirecting to file.
    color            - The default color output to the terminal using ANSI escape sequences.
    data/dst/map.dst - Path from the root of the repository to the generation map to solve.
    G[GYM NUMBER]    - Add as many gyms to your argument to solve cover problems only for those gyms.
    E4               - Add the "Elite Four" or equivalent stand-in final boss for a generation to the subset.
    A                - The Attack flag to solve the attack type cover problem.
    D                - The Defense flag to solve the defensive type cover problem. This is the default.
    E                - Solve an Exact cover problem. This the default.
    O                - Solve the overlapping cover problem
Example Command:
    ./build/bin/pokemon_cli G1 G2 G3 G4 data/dst/Gen-5-Unova2.dst)";

enum class Solution_type : uint8_t
{
    exact,
    overlapping
};

enum class Table_type : uint8_t
{
    first,
    normal,
    last
};

enum class Print_style : uint8_t
{
    color,
    plain
};

struct Universe_sets
{
    std::vector<Dx::Type_encoding> items;
    std::vector<Dx::Type_encoding> options;
};

struct Runner
{
    std::string map;
    std::map<Dx::Type_encoding, std::set<Dx::Resistance>> interactions;
    std::set<std::string_view> selected_gyms;
    Dx::Pokemon_links::Coverage_type type{
        Dx::Pokemon_links::Coverage_type::defense};
    Solution_type sol_type{Solution_type::exact};
    Print_style style{Print_style::color};
};

int run(std::span<char const *const> args);
int solve(Runner const &runner);
void print_types(Ranked_set<Dx::Type_encoding> const &res, Print_style style);
std::string
generate_type_string(std::pair<std::string_view, std::string_view> name,
                     std::pair<uint64_t, std::optional<uint64_t>> indices,
                     Print_style style);
void print_prep_message(Universe_sets const &sets, Print_style style);
void break_line(size_t max_set_len, Table_type t);
void print_solution_msg(std::set<Ranked_set<Dx::Type_encoding>> const &result,
                        Runner const &runner);
void help();

} // namespace

/// Run this program from the root of the repository. That way your request for
/// any generation map will be handled correctly becuase the map you are
/// requesting will be in the /data/dst or /data/json relative to the root.
int
main(int argc, char **argv)
{
    auto const args
        = std::span<char const *const>{argv, static_cast<size_t>(argc)}.subspan(
            1);
    if (args.empty())
    {
        return 0;
    }
    return run(args);
}

namespace {

int
run(std::span<char const *const> const args)
{
    try
    {
        Runner runner;
        for (auto const &arg : args)
        {
            std::string_view const arg_str{arg};
            if (arg_str.find('/') != std::string::npos)
            {
                if (!runner.interactions.empty())
                {
                    std::cerr << "Cannot load multiple generations "
                                 "simultaneously. Specify one.\n";
                    return 1;
                }
                auto const owned = std::string(arg_str);
                std::ifstream f(owned);
                runner.interactions = Dancing_links::load_interaction_map(f);
                runner.map = owned.substr(owned.find_last_of('/') + 1);
            }
            else if (arg_str.starts_with('G') || arg_str == "E4")
            {
                runner.selected_gyms.emplace(arg_str);
            }
            else if (arg_str == "A")
            {
                runner.type = Dx::Pokemon_links::Coverage_type::attack;
            }
            else if (arg_str == "D")
            {
                runner.type = Dx::Pokemon_links::Coverage_type::defense;
            }
            else if (arg_str == "E")
            {
                runner.sol_type = Solution_type::exact;
            }
            else if (arg_str == "O")
            {
                runner.sol_type = Solution_type::overlapping;
            }
            else if (arg_str == "color")
            {
                runner.style = Print_style::color;
            }
            else if (arg_str == "plain")
            {
                runner.style = Print_style::plain;
            }
            else if (arg_str == "h")
            {
                help();
                return 0;
            }
            else
            {
                std::cerr << "Unknown argument: " << arg_str << "\n";
                help();
                return 1;
            }
        }
        return solve(runner);
    } catch (...)
    {
        std::cerr << "Pokemon CLI application encountered exception.\n";
        help();
        return 1;
    }
}

int
solve(Runner const &runner)
{
    if (runner.map.empty() || runner.map.empty())
    {
        std::cerr << "No data loaded from any map to solve.\n";
        return 1;
    }
    Dx::Pokemon_links links(runner.interactions, runner.type);
    if (!runner.selected_gyms.empty())
    {
        std::set<Dx::Type_encoding> subset{};
        // Load in the opposite of our coverage type so we know what we
        // attack/defend against in this subset of gyms.
        runner.type == Dx::Pokemon_links::Coverage_type::attack
            ? subset = Dancing_links::load_selected_gyms_defenses(
                  runner.map, runner.selected_gyms)
            : subset = Dancing_links::load_selected_gyms_attacks(
                  runner.map, runner.selected_gyms);
        if (subset.empty())
        {
            std::cerr << "The existence of one or more requested gyms could "
                         "not be confirmed.\n";
            return 1;
        }
        Dx::hide_items_except(links, subset);
    }
    Universe_sets const items_options = {
        .items = Dx::items(links),
        .options = Dx::options(links),
    };
    print_prep_message(items_options, runner.style);
    int const depth_limit
        = runner.type == Dx::Pokemon_links::Coverage_type::attack ? 24 : 6;

    // Core dancing links solver operates here, either exact or overlapping.
    std::set<Ranked_set<Dx::Type_encoding>> const result
        = runner.sol_type == Solution_type::exact
              ? Dx::exact_cover_stack(links, depth_limit)
              : Dx::overlapping_cover_stack(links, depth_limit);

    print_solution_msg(result, runner);
    if (result.empty())
    {
        return 0;
    }
    auto const &largest_ranked_set
        = std::ranges::max(result, [](Ranked_set<Dx::Type_encoding> const &a,
                                      Ranked_set<Dx::Type_encoding> const &b) {
              return a.size() < b.size();
          });
    size_t const max_set_len = largest_ranked_set.size();
    break_line(max_set_len, Table_type::first);
    size_t cur_set = 1;
    if (Dx::has_max_solutions(links))
    {
        std::cout << "Hit maximum solutions capacity, quitting early!\n";
    }
    for (auto const &res : result)
    {
        std::cout << std::left << std::setw(digit_width) << res.rank();
        size_t col = res.size();
        print_types(res, runner.style);
        while (col < max_set_len)
        {
            std::cout << "│" << std::left << std::setw(max_name_width) << "";
            ++col;
        }
        std::cout << "│\n";
        cur_set == result.size() ? break_line(max_set_len, Table_type::last)
                                 : break_line(max_set_len, Table_type::normal);
        ++cur_set;
    }
    if (Dx::has_max_solutions(links))
    {
        std::cout << "Hit maximum solutions capacity, quitting early!\n";
    }
    print_solution_msg(result, runner);
    print_prep_message(items_options, runner.style);
    return 0;
}

void
print_types(Ranked_set<Dx::Type_encoding> const &res, Print_style style)
{
    for (auto const &t : res)
    {
        std::pair<std::string_view, std::string_view> const type_pair
            = t.decode_type();
        std::pair<uint64_t, std::optional<uint64_t>> const type_indices
            = t.decode_indices();
        std::string const output
            = generate_type_string(type_pair, type_indices, style);
        int width = 0;
        if (type_indices.second)
        {
            width = max_name_width
                    - static_cast<int>(type_pair.first.size() + 1
                                       + type_pair.second.size());
        }
        else
        {
            width = max_name_width - static_cast<int>(type_pair.first.size());
        }
        std::cout << "│" << output << std::setw(width) << "";
    }
}

std::string
generate_type_string(std::pair<std::string_view, std::string_view> name,
                     std::pair<uint64_t, std::optional<uint64_t>> indices,
                     Print_style style)
{
    std::string output = {};
    if (indices.second)
    {
        if (style == Print_style::color)
        {
            output = std::string(type_colors.at(indices.first))
                         .append(name.first)
                         .append(nil)
                         .append("-")
                         .append(type_colors.at(indices.second.value()))
                         .append(name.second)
                         .append(nil);
        }
        else
        {
            output = std::string(name.first).append("-").append(name.second);
        }
    }
    else
    {
        if (style == Print_style::color)
        {
            output = std::string(type_colors.at(indices.first))
                         .append(name.first)
                         .append(nil);
        }
        else
        {
            output = std::string(name.first);
        }
    }
    return output;
}

void
print_prep_message(Universe_sets const &sets, Print_style style)
{
    std::string item_msg = {};
    if (style == Print_style::color)
    {
        item_msg.append("Trying to cover ")
            .append(ansi_yel)
            .append(std::to_string(sets.items.size()))
            .append(" items\n\n")
            .append(nil);
    }
    else
    {
        item_msg.append("Trying to cover ")
            .append(std::to_string(sets.items.size()))
            .append(" items\n\n");
    }
    std::cout << item_msg;
    for (auto const &type : sets.items)
    {
        std::pair<std::string_view, std::string_view> const type_pair
            = type.decode_type();
        std::pair<uint64_t, std::optional<uint64_t>> const type_indices
            = type.decode_indices();
        std::string const output
            = generate_type_string(type_pair, type_indices, style);
        std::cout << output << ", ";
    }
    std::string option_msg = {};
    if (style == Print_style::color)
    {
        option_msg.append("\n")
            .append(ansi_yel)
            .append(std::to_string(sets.options.size()))
            .append(" options")
            .append(nil)
            .append(" are available:\n\n");
    }
    else
    {
        option_msg.append("\n")
            .append(std::to_string(sets.options.size()))
            .append(" options")
            .append(" are available:\n\n");
    }
    std::cout << "\n" << option_msg;
    for (auto const &type : sets.options)
    {
        std::pair<std::string_view, std::string_view> const type_pair
            = type.decode_type();
        std::pair<uint64_t, std::optional<uint64_t>> const type_indices
            = type.decode_indices();
        std::string const output
            = generate_type_string(type_pair, type_indices, style);
        std::cout << output << ", ";
    }
    std::cout << "\n";
}

void
print_solution_msg(std::set<Ranked_set<Dx::Type_encoding>> const &result,
                   Runner const &runner)
{
    std::string msg = {};
    if (runner.style == Print_style::color)
    {
        msg.append(result.empty() ? ansi_red : ansi_grn)
            .append("\nFound ")
            .append(std::to_string(result.size()))
            .append(runner.sol_type == Solution_type::exact ? " exact"
                                                            : " overlapping")
            .append(" cover ranked sets of options for specified items.")
            .append(runner.type == Dx::Pokemon_links::Coverage_type::defense
                        ? " Lower rank is better."
                        : " Higher rank is better.")
            .append("\n\n")
            .append(nil);
    }
    else
    {
        msg.append("\nFound ")
            .append(std::to_string(result.size()))
            .append(runner.sol_type == Solution_type::exact ? " exact"
                                                            : " overlapping")
            .append(" cover ranked sets of options for specified items.")
            .append(runner.type == Dx::Pokemon_links::Coverage_type::defense
                        ? " Lower rank is better."
                        : " Higher rank is better.")
            .append("\n\n");
    }
    std::cout << msg;
}

void
break_line(size_t max_set_len, Table_type t)
{
    std::cout << std::left << std::setw(digit_width) << "";
    switch (t)
    {
        case Table_type::first:
            std::cout << "┌";
            break;
        case Table_type::normal:
            std::cout << "├";
            break;
        case Table_type::last:
            std::cout << "└";
            break;
        default:
            std::cerr << "Unknown table type\n";
    }
    for (size_t col = 0; col < max_set_len; ++col)
    {
        for (size_t line = 0; line < max_name_width; ++line)
        {
            std::cout << "─";
        }
        switch (t)
        {
            case Table_type::first:
                std::cout << (col == max_set_len - 1 ? "┐" : "┬");
                break;
            case Table_type::normal:
                std::cout << (col == max_set_len - 1 ? "┤" : "┼");
                break;
            case Table_type::last:
                std::cout << (col == max_set_len - 1 ? "┘" : "┴");
                break;
            default:
                std::cerr << "Unknown table type\n";
        }
    }
    std::cout << "\n";
}

void
help()
{
    std::cout << help_msg << "\n";
}

} // namespace
