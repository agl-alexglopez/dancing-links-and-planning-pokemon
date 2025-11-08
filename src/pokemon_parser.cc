module;
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <algorithm>
#include <array>
#include <cassert>
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
    // The maximum number of attack types you would ever see at a gym are all
    // 18 types. This rarely, if ever, happens.
    std::array<Type_encoding, 18> attack;
    // The maximum number of defensive types at any one gym occurs if the
    // Elite Four has all four members with full 6 Pokemon teams.
    std::array<Type_encoding, 24> defense;
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
    std::map<std::string, Map_node> network;
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

/// @brief load_interaction_map builds the PokemonTest needed to interact with a
/// generation's map in the Pokemon Planning GUI.
/// @param source the file with the map that gives us info on which gen to
/// build.
/// @return the completed pokemon test with map drawing and Pokemon info.
std::map<Type_encoding, std::set<Resistance>>
load_interaction_map(std::string_view region_name);

/// @brief load_selected_gyms_defenses when interacting with the GUI, the user
/// can choose subsets of gyms on the current map they are viewing. If they make
/// these selections we can load in the defensive types that are present at
/// those gyms. These are intended to be fed to a PokemonLinks solver as the
/// items we must defend against.
/// @param selectedMap the current .dst file we are viewing.
/// @param selectedGyms the gyms G1-E4 that we are considering attacking.
/// @return the set of all defensive types present in the selection of gyms.
std::set<Type_encoding>
load_selected_gyms_defenses(std::string_view selected_map,
                            std::set<std::string_view> const &selected_gyms);

/// @brief load_selected_gyms_attacks the user interacting with the GUI may want
/// to defend themselves from only a selection of gyms. We will get the gym info
/// for the selected map and return the types of attacks present across all of
/// those selections. This set can then be passed to the PokemonLinks dancing
/// links class as a second parameter.
/// @param selected_map the current map the user interacts with.
/// @param selected the gyms they have selected.
/// @return a set of all attack types present across those gyms.
std::set<Type_encoding>
load_selected_gyms_attacks(std::string_view selected_map,
                           std::set<std::string_view> const &selected);

} // namespace Dancing_links

///////////////////////////////////////   Implementation

namespace Dancing_links {

namespace {

namespace nlo = nlohmann;

constexpr float file_coordinate_pad = 1.0;

constexpr std::string_view json_all_maps_file = "data/json/all-maps.json";
constexpr std::string_view gym_attacks_key = "attack";
constexpr std::string_view gym_defense_key = "defense";

// There is no 0th generation so we will make it easier to select the right file
// by leaving 0 "".
constexpr std::array<
    std::tuple<std::string_view, std::string_view, std::string_view>, 10>
    generation_type_rules_json = {{
        {"", "", ""},
        {"1", "Kanto", "data/json/gen-1-types.json"},
        {"2", "Johto", "data/json/gen-2-types.json"},
        {"3", "Hoenn", "data/json/gen-3-types.json"},
        {"4", "Sinnoh", "data/json/gen-4-types.json"},
        {"5", "Unova", "data/json/gen-5-types.json"},
        {"6", "Kalos", "data/json/gen-6-types.json"},
        {"7", "Alola", "data/json/gen-7-types.json"},
        {"8", "Galar", "data/json/gen-8-types.json"},
        {"9", "Paldea", "data/json/gen-9-types.json"},
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

void
fill_type_table(
    std::span<Type_encoding> table,
    std::tuple_element_t<
        1, nlohmann::detail::iteration_proxy_value<
               nlohmann::detail::iter_impl<nlohmann::basic_json<> const>> const>
        e)
{
    size_t i = 0;
    for (auto const &type : e)
    {
        assert(i < table.size());
        auto const type_view = type.get<std::string_view>();
        table[i++] = Type_encoding(type_view);
    }
}

std::map<std::string, Map_node>
load_map_from_json(std::string_view const region_name)
{
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
    std::map<std::string, Map_node> network{};
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
load_selected_gyms_defenses(std::string_view selected_map,
                            std::set<std::string_view> const &selected_gyms)
{
    if (selected_gyms.empty())
    {
        std::cerr
            << "Requesting to load zero gyms check selected gyms input.\n";
    }
    nlo::json const map_data = get_json_object(json_all_maps_file);
    std::set<Type_encoding> result = {};

    nlo::json const &gym_keys = map_data.at(selected_map);

    std::vector<std::string_view> confirmed{};
    confirmed.reserve(selected_gyms.size());
    for (auto const &[gym, attack_defense_map] : gym_keys.items())
    {
        if (!selected_gyms.contains(gym))
        {
            continue;
        }
        confirmed.push_back(gym);
        for (auto const &t : attack_defense_map.at(gym_defense_key))
        {
            auto const type = t.template get<std::string>();
            result.emplace(type);
        }
    }
    if (confirmed.size() != selected_gyms.size())
    {
        std::cerr << "Mismatch occured for " << selected_map
                  << " gym selection.\nRequested: ";
        for (auto const &s : selected_gyms)
        {
            std::cerr << s << " ";
        }
        std::cerr << "\nConfirmed: ";
        for (auto const &s : confirmed)
        {
            std::cerr << s << " ";
        }
        std::cerr << "\n";
        return {};
    }
    // This will be a much smaller set.
    return result;
}

std::set<Type_encoding>
load_selected_gyms_attacks(std::string_view selected_map,
                           std::set<std::string_view> const &selected_gyms)
{
    if (selected_gyms.empty())
    {
        std::cerr
            << "Requesting to load zero gyms check selected gyms input.\n";
    }
    nlo::json const map_data = get_json_object(json_all_maps_file);
    std::set<Type_encoding> result = {};
    nlo::json const &selection = map_data.at(selected_map);

    std::vector<std::string_view> confirmed{};
    confirmed.reserve(selected_gyms.size());
    for (auto const &[gym, attack_defense_map] : selection.items())
    {
        if (!selected_gyms.contains(gym))
        {
            continue;
        }
        confirmed.push_back(gym);
        for (auto const &t : attack_defense_map.at(gym_attacks_key))
        {
            auto const type = t.template get<std::string_view>();
            result.emplace(type);
        }
    }
    if (confirmed.size() != selected_gyms.size())
    {
        std::cerr << "Mismatch occured for " << selected_map
                  << " gym selection.\nRequested: ";
        for (auto const &s : selected_gyms)
        {
            std::cerr << s << " ";
        }
        std::cerr << "\nConfirmed: ";
        for (auto const &s : confirmed)
        {
            std::cerr << s << " ";
        }
        std::cerr << "\n";
        return {};
    }
    // Return a set rather than altering every resistances in a large map.
    return result;
}

} // namespace Dancing_links
