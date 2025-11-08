module;
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
export module dancing_links:pokemon_parser;
import :resistance;
import :type_encoding;

///////////////////////////////////   Exported Interface

export namespace Dancing_links {

struct Point
{
    float x;
    float y;
};

struct Min_max
{
    float min;
    float max;
};

// This type is critical to how the network is stored and traversed. Every
// entry in our network map will have a std::string key and this node as
// the value. Only one std::string name for every city will ever be allocated
// and it will be stored as the key for that city in the map. Then the edges
// are simply pointers back to the keys in the map that already exist as our
// string names.
//
// This way we do not waste tons of repetitive tiny string allocations on the
// heap for the same city name in well connected networks. Instead we have a
// set of trivially copyable pointers. These pointers will be searched and
// stored in the set by their pointer address but this is OK because every
// key in the map will be unique. Obviously, it's a map!
struct Map_node
{
    // Every city is given a 3 letter abbreviation code. Add null terminator.
    std::array<char, 4> code;
    // Where the user has specified the location of this city should be in
    // their dst file. This is true to what they wrote in the file. We can
    // adjust this to display as needed later in the GUI.
    Point coordinates;
    // The set of pointers to other string keys in the map that serve as our
    // city edge connections. Simple pointers, no wasted strings.
    std::set<Map_node *> edges;
    // The attack types across all the Pokemon one may face at this gym.
    std::vector<Type_encoding> attack;
    // The defense types across all the Pokemon one may face at this gym.
    std::vector<Type_encoding> defense;
};

/// Leave a comment at the first line of the Pokemon Generation .dst file you
/// want to construct with the pokemon generation in base 10 numbers. This will
/// determine the types availabe for the given map and generation cover problem.
/// Any of the 9 generations are acceptable. If the first line does not match
/// one of these strings I will just include all pokemon types in the cover
/// problem. Like this.
///
/// # 1
/// # 2
/// # 3
/// # 4
/// # 5
/// # 6
/// # 7
/// # 8
/// # 9
///
/// This also means you should start every file with some sort of comment
/// labelling it with identifying info, even if there is not generation
/// specification.
struct Pokemon_test
{
    std::map<std::string, Map_node, std::less<>> network;
    struct Min_max x_data_bounds{};
    struct Min_max y_data_bounds{};
    /// This map will hold all types--dual types included--and a map of defense
    /// multipliers ranging from x0.0,x0.25,x0.5,x1.0,x2.0,x4.0. In these maps
    /// will be the type as the key and all the single attack types that have
    /// that multiplier against the current type. There are 18 single attack
    /// types. The highest level map holds 162 type keys that exist in Pokemon
    /// as of December 2022. However, depending on which generation map you
    /// decide to draw some types might be missing. For example, generation one
    /// Pokemon did not have types like Fairy, Dark, or Steel.
    std::map<Type_encoding, std::set<Resistance>> interactions;
};

/// @brief load_pokemon_generation builds the PokemonTest needed to interact
/// with a generation's map in the Pokemon Planning GUI.
/// @param source the file with the map that gives us info on which gen
/// to build.
/// @return the completed pokemon test with map drawing and Pokemon info.
Pokemon_test load_pokemon_generation(std::string_view region_name);

/// @brief loads all the attack types at the selected gyms into one set.
/// @param generation the fully loaded generation.
/// @param selected_gyms the set of gyms attack types are loaded from.
/// @return the set of only those attack types present at the selected gyms.
/// @warning the names in the selected gyms set must match how they appeared
/// when the generation was loaded from a json file.
std::set<Type_encoding>
get_selected_gyms_attacks(Pokemon_test const &generation,
                          std::set<std::string_view> const &selected_gyms);

/// @brief loads all the defense types at the selected gyms into one set.
/// @param generation the fully loaded generation.
/// @param selected_gyms the set of gyms defense types are loaded from.
/// @return the set of only those defense types present at the selected gyms.
/// @warning the names in the selected gyms set must match how they appeared
/// when the generation was loaded from a json file.
std::set<Type_encoding>
get_selected_gyms_defenses(Pokemon_test const &generation,
                           std::set<std::string_view> const &selected_gyms);

} // namespace Dancing_links

///////////////////////////////////////   Implementation

namespace Dancing_links {

namespace {

enum class Gym_type : uint8_t
{
    attack,
    defense,
};

namespace nlo = nlohmann;

constexpr float file_coordinate_pad = 1.0;

// There is no 0th generation so we will make it easier to select the right file
// by leaving 0 "".
constexpr std::array<
    std::tuple<std::string_view, std::string_view, std::string_view>, 10>
    generation_type_rules_json = {{
        {"", "", ""},
        {"1", "Kanto", "data/types/gen-1-types.json"},
        {"2", "Johto", "data/types/gen-2-types.json"},
        {"3", "Hoenn", "data/types/gen-3-types.json"},
        {"4", "Sinnoh", "data/types/gen-4-types.json"},
        {"5", "Unova", "data/types/gen-5-types.json"},
        {"6", "Kalos", "data/types/gen-6-types.json"},
        {"7", "Alola", "data/types/gen-7-types.json"},
        {"8", "Galar", "data/types/gen-8-types.json"},
        {"9", "Paldea", "data/types/gen-9-types.json"},
    }};

constexpr std::array<
    std::tuple<std::string_view, std::string_view, std::string_view>, 10>
    generation_region_maps_json{{
        {"", "", ""},
        {"1", "Kanto", "data/maps/gen-1-kanto.json"},
        {"2", "Johto", "data/maps/gen-2-johto.json"},
        {"3", "Hoenn", "data/maps/gen-3-hoenn.json"},
        {"4", "Sinnoh", "data/maps/gen-4-sinnoh.json"},
        {"5", "Unova", "data/maps/gen-5-unova.json"},
        {"6", "Kalos", "data/maps/gen-6-kalos.json"},
        {"7", "Alola", "data/maps/gen-7-alola.json"},
        {"8", "Galar", "data/maps/gen-8-galar.json"},
        {"9", "Paldea", "data/maps/gen-9-paldea.json"},
    }};

std::array<std::pair<std::string_view, Multiplier>, 6> const damage_multipliers
    = {{
        {"immune", Multiplier::imm},
        {"quarter", Multiplier::f14},
        {"half", Multiplier::f12},
        {"normal", Multiplier::nrm},
        {"double", Multiplier::dbl},
        {"quad", Multiplier::qdr},
    }};

Multiplier const &
get_multiplier(std::string const &key)
{
    for (auto const &mult : damage_multipliers)
    {
        if (mult.first == key)
        {
            return mult.second;
        }
    }
    throw std::logic_error("Out of bounds. Key not found. ");
}

nlo::json
get_json_object(std::string_view path_to_json)
{
    std::ifstream json_file(path_to_json.data());
    if (!json_file.is_open())
    {
        std::cerr << "Could not open json file: ." << path_to_json << "\n";
        json_file.close();
        std::abort();
    }
    nlo::json data = nlo::json::parse(json_file);
    json_file.close();
    if (data.is_discarded())
    {
        std::cerr << "Error parsing all map data to json object.\n";
        std::abort();
    }
    return data;
}

void
set_resistances(std::map<Type_encoding, std::set<Resistance>> &result,
                Type_encoding const &new_type, nlo::json const &multipliers)
{
    for (auto const &[multiplier, types_in_multiplier] : multipliers.items())
    {
        Multiplier const multiplier_tag = get_multiplier(multiplier);
        for (auto const &t : types_in_multiplier)
        {
            auto const type = t.template get<std::string_view>();
            result[new_type].emplace(type, multiplier_tag);
        }
    }
}

std::map<std::string, Map_node, std::less<>>
load_map_from_json(std::string_view const region_name)
{
    auto const fill_type_table
        = [](std::vector<Type_encoding> &table, auto &&json_table) {
              table.reserve(json_table.size());
              for (auto const &type : json_table)
              {
                  auto const type_view = type.template get<std::string_view>();
                  table.emplace_back(type_view);
              }
          };
    auto const *const generation_map = std::ranges::find(
        generation_region_maps_json, region_name,
        [](std::tuple<std::string_view, std::string_view,
                      std::string_view> const &t) { return std::get<1>(t); });
    if (generation_map == generation_region_maps_json.end())
    {
        std::cerr << "could not find generation region map data for "
                  << region_name << '\n';
        std::abort();
    }
    std::map<std::string, Map_node, std::less<>> network{};
    nlo::json generation = get_json_object(std::get<2>(*generation_map));
    auto const &region = generation[std::get<0>(*generation_map).data()]
                                   [std::get<1>(*generation_map).data()];
    for (auto const &[city_name, city_data] : region.items())
    {
        auto inserted = network.try_emplace(city_name, Map_node{});
        Map_node &attributes = inserted.first->second;
        for (auto const &[key, val] : city_data.items())
        {
            if (key == "code")
            {
                auto const three_lettters = val.get<std::string_view>();
                assert(three_lettters.length() == 3);
                std::ranges::copy(three_lettters, attributes.code.begin());
                attributes.code.back() = '\0';
            }
            else if (key == "point")
            {
                assert(val.is_array());
                auto const &[x, y] = val.get<std::pair<float, float>>();
                attributes.coordinates = Point{.x = x, .y = y};
            }
            else if (key == "edges")
            {
                assert(val.is_array());
                for (auto const &edge : val)
                {
                    auto const edge_view = edge.get<std::string_view>();
                    auto const ensure_inserted = network.try_emplace(
                        std::string(edge_view), Map_node{});
                    attributes.edges.insert(&ensure_inserted.first->second);
                }
            }
            else if (key == "attack")
            {
                fill_type_table(attributes.attack, val);
            }
            else if (key == "defense")
            {
                fill_type_table(attributes.defense, val);
            }
            else
            {
                std::cerr << "unknown json field in region map: " << key;
                std::abort();
            }
        }
    }
    return network;
}

std::map<Type_encoding, std::set<Resistance>>
load_interaction_map(std::string_view region_name)
{
    auto const *const interaction_map = std::ranges::find(
        generation_type_rules_json, region_name,
        [](std::tuple<std::string_view, std::string_view,
                      std::string_view> const &t) { return std::get<1>(t); });
    if (interaction_map == generation_type_rules_json.end())
    {
        std::cerr << "could not find generation type interaction data for "
                  << region_name << '\n';
        std::abort();
    }
    nlo::json const json_types = get_json_object(std::get<2>(*interaction_map));
    std::map<Type_encoding, std::set<Resistance>> result = {};
    for (auto const &[type, resistances] : json_types.items())
    {
        Type_encoding const encoded(type);
        result.insert({encoded, {}});
        set_resistances(result, encoded, resistances);
    }
    return result;
}

std::set<Type_encoding>
get_selected_gyms_types(Pokemon_test const &generation,
                        std::set<std::string_view> const &selected_gyms,
                        Gym_type type)
{
    std::set<Type_encoding> result{};
    for (auto const &gym : selected_gyms)
    {
        auto const found = generation.network.find(gym);
        if (found == generation.network.end())
        {
            std::cerr << "selected gyms contains out of network name.\n";
            std::abort();
        }
        std::vector<Type_encoding> const &table = type == Gym_type::attack
                                                      ? found->second.attack
                                                      : found->second.defense;
        for (Type_encoding const &encoding : table)
        {
            result.insert(encoding);
        }
    }
    return result;
}

} // namespace

Pokemon_test
load_pokemon_generation(std::string_view region_name)
{
    Pokemon_test generation;
    generation.network = load_map_from_json(region_name);
    generation.interactions = load_interaction_map(region_name);
    generation.x_data_bounds.min = std::numeric_limits<float>::infinity();
    generation.y_data_bounds.min = std::numeric_limits<float>::infinity();
    generation.x_data_bounds.max = -std::numeric_limits<float>::infinity();
    generation.y_data_bounds.max = -std::numeric_limits<float>::infinity();
    for (auto const &[_, node] : generation.network)
    {
        generation.x_data_bounds.min
            = std::min(generation.x_data_bounds.min, node.coordinates.x);
        generation.y_data_bounds.min
            = std::min(generation.y_data_bounds.min, node.coordinates.y);
        generation.x_data_bounds.max
            = std::max(generation.x_data_bounds.max, node.coordinates.x);
        generation.y_data_bounds.max
            = std::max(generation.y_data_bounds.max, node.coordinates.y);
    }
    generation.x_data_bounds.min -= file_coordinate_pad;
    generation.y_data_bounds.min -= file_coordinate_pad;
    generation.x_data_bounds.max += file_coordinate_pad;
    generation.y_data_bounds.max += file_coordinate_pad;
    return generation;
}

std::set<Type_encoding>
get_selected_gyms_attacks(Pokemon_test const &generation,
                          std::set<std::string_view> const &selected_gyms)
{
    return get_selected_gyms_types(generation, selected_gyms, Gym_type::attack);
}

std::set<Type_encoding>
get_selected_gyms_defenses(Pokemon_test const &generation,
                           std::set<std::string_view> const &selected_gyms)
{
    return get_selected_gyms_types(generation, selected_gyms,
                                   Gym_type::defense);
}

} // namespace Dancing_links
