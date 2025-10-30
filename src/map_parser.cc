/// Author(s): Keith Schwarz and Stanford course staff with modifications
/// by Alex G. Lopez.
/// File: map_parser.cc
/// This file is a leftover from the old QT project used to visualize the
/// road network and graphs of Pokemon Maps. I will likely make use of it
/// again when the graph cover and Pokemon map visualizer is complete. I
/// modified the entire program to be compliant with C++ 20 and use no
/// Stanford Internal libraries, instead using the stl where needed. Finally,
/// I adjusted the structure to suppport C++ modules.
module;
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <istream>
#include <limits>
#include <map>
#include <ostream>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
export module dancing_links:map_parser;

//////////////////////////////////////   Exported Interface

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
    // Where the user has specified the location of this city should be in
    // their dst file. This is true to what they wrote in the file. We can
    // adjust this to display as needed later in the GUI.
    Point coordinates;
    // The set of pointers to other string keys in the map that serve as our
    // city edge connections. Simple pointers, no wasted strings.
    std::set<std::string const *> edges;
};

/// Type representing a test case for the Disaster Preparation problem.
struct Map_test
{
    // The network stores one string for every city (aka gym) name as the key.
    // The nodes then specify where this city is located and its set of outgoing
    // edges.
    //
    // The outgoing edges store pointers back to the keys in this same map. So
    // This is a self referential data structure relying the stable guarantee of
    // the map keys in memory to function correctly.
    std::map<std::string, Map_node> network;
    struct Min_max x_data_bounds{};
    struct Min_max y_data_bounds{};
};

/// @brief Given a stream pointing at a test case for Disaster Preparation,
/// pulls the data from that test case.
/// @param source The stream containing the test case.
/// @return A test case from the file.
/// @throws ErrorException If an error occurs or the file is invalid.
Map_test load_map(std::istream &source);

std::ostream &operator<<(std::ostream &out, Point const &p);
bool operator==(Point const &lhs, Point const &rhs);
std::partial_ordering operator<=>(Point const &lhs, Point const &rhs);

} // namespace Dancing_links

//////////////////////////////////////   Implementation

namespace std {
template <> struct hash<Dancing_links::Point>
{
    size_t
    operator()(Dancing_links::Point const &p) const noexcept
    {
        return hash<float>()(p.x) ^ (hash<float>()(p.y) << 1U);
    }
};
} // namespace std

namespace Dancing_links {

namespace {

constexpr float file_coordinate_pad = 1.0;
constexpr char const *const whitespace = " \n\r\t\f\v";

/// This regular expression pattern expects for gym location to be listed with
/// either G and a gym number or E4 for the elite four.
///
/// For example to draw a Pokemon map for generation 1 in the Kanto Region we
/// would start by specifying Gym 1 and its connections as follows.
///
/// G1 (3, 4): G8, G2
///
/// Gym 1 is at relative coordinate x = 3, y = 4 and it is connected by roads
/// to Gym 8 and Gym 2. This regex allows for more flexibility in naming the
/// locations, gyms, and elite four for future use but we document the single
/// letter prefix rule elsewhere to make the file formats uniform.
constexpr char const *const dst_file_regex
    = R"(^([A-Za-z0-9 .\-]+)\(\s*(-?[0-9]+(?:\.[0-9]+)?)\s*,\s*(-?[0-9]+(?:\.[0-9]+)?)\s*\)$)";

/// These name components represent the components of a location in a
/// destination map.
///
/// G1 (3, 4)
///
/// The location is listed with its coordinates on the map. After these
/// parts comes the colon and the listing of comma separated connections.
enum class Name_components : uint8_t
{
    whole_string = 0,
    city_name,
    x_coord,
    y_coord,
    num_components
};

struct City_links
{
    std::string city;
    std::string links;
};

std::string
trim(std::string const &str)
{
    size_t const begin = str.find_first_not_of(whitespace);
    if (begin == std::string::npos)
    {
        return "";
    }
    size_t const end = str.find_last_not_of(whitespace);
    return str.substr(begin, end - begin + 1);
}

std::vector<std::string>
string_split(std::string const &str, char const delim)
{
    std::vector<std::string> components{};
    std::stringstream ss(str);
    while (ss.good())
    {
        std::string substr;
        std::getline(ss, substr, delim);
        components.push_back(substr);
    }
    return components;
}

/// Given city information in the form
///     City_name (X, Y)
/// Parses out the name and the X/Y coordinate, returning the
/// name, and filling in the MapTest with what's found.
std::string
parse_city(std::string const &city_info, Map_test &result)
{
    // Split on all the delimiters and confirm we've only got
    // three components.
    std::regex const pattern(dst_file_regex);
    std::smatch components;
    std::string const to_match = trim(city_info);

    if (!regex_match(to_match, components, pattern))
    {
        std::cerr << "Can't parse this data; is it city info? " << city_info
                  << "\n";
        std::abort();
    }

    // There are four components here, actually: the whole match,
    // plus each subexpression we care about.
    if (components.size()
        != static_cast<uint8_t>(Name_components::num_components))
    {
        std::cerr << "Could not find all components?\n";
        std::abort();
    }

    // We're going to get back some extra leading or trailing
    // whitespace here, so peel it off.
    std::string name
        = trim(components[static_cast<uint8_t>(Name_components::city_name)]);
    if (name.empty())
    {
        std::cerr << "City names can't be empty.\n";
        std::abort();
    }

    // In the parse links step we may have inserted a new key and empty default
    // value for this city preemptively because we had not reached that city's
    // line in the file yet. We are at that line in the file now. Carefully add
    // only the data we need leaving the key alone if it has already been
    // inserted. We cannot alter the key because it is now the stable string
    // for all edge sets that other cities will use.
    auto inserted = result.network.try_emplace(std::move(name), Map_node{});
    inserted.first->second.coordinates = Point{
        .x
        = std::stof(components[static_cast<uint8_t>(Name_components::x_coord)]),
        .y
        = std::stof(components[static_cast<uint8_t>(Name_components::y_coord)]),
    };
    return inserted.first->first;
}

/// Reads the links out of the back half of the line of a file,
/// adding them to the road network.
void
parse_links(City_links const &cl, Map_test &result)
{
    // It's possible that there are no outgoing links.
    auto this_city = result.network.find(cl.city);
    if (this_city == result.network.end())
    {
        std::cerr << "City locations map is missing a city and should be "
                     "completed before network map.\n";
        std::abort();
    }
    if (trim(cl.links).empty())
    {
        this_city->second.edges = {};
        return;
    }

    // We will check for repeats and insert into this city's connections.
    std::set<std::string const *> &connections
        = result.network.at(this_city->first).edges;
    std::vector<std::string> const components = string_split(cl.links, ',');
    for (std::string const &dest : components)
    {
        // Clean up all whitespace and make sure that we didn't
        // discover an empty entry.
        std::string clean_name = trim(dest);
        if (clean_name.empty())
        {
            std::cerr << "Blank name in list of outgoing cities?\n";
            std::abort();
        }

        auto const key_string_source = result.network.find(clean_name);
        // Confirm this isn't a dupe.
        if (key_string_source != result.network.end()
            && connections.contains(&key_string_source->first))
        {
            std::cerr << "City appears twice in outgoing list?\n";
            std::abort();
        }
        // We may not have reached this step yet in the parse city operation
        // because we have not reached the line in the file corresponding to
        // this city. We will learn where this city is located and what its
        // edges are later. For now, just give it its string entry in the map
        // so we are pointer stable for the key reference and lookup.
        auto const ensure_inserted
            = result.network.try_emplace(std::move(clean_name), Map_node{});
        connections.insert(&ensure_inserted.first->first);
    }
}

/// Parses one line out of the file and updates the network with what
/// it found. This will only add edges in the forward direction as
/// a safety measure; edges are reversed later on.
void
parse_city_line(std::string const &line, Map_test &result)
{
    // Search for a colon on the line. The split function will only return a
    // single component if there are no outgoing links specified.
    auto const num_colons = std::count(line.begin(), line.end(), ':');
    if (num_colons != 1)
    {
        std::cerr << "Each data line should have exactly one colon on it.\n";
        std::abort();
    }

    // Split the line into the city name/location and the list
    // of outgoing cities.
    auto components = string_split(line, ':');
    if (components.empty())
    {
        std::cerr << "Data line appears to have no city information.\n";
        std::abort();
    }

    // Create a dummy list of outgoing cities if one doesn't already exist.
    if (components.size() == 1)
    {
        components.emplace_back("");
    }

    std::string const name = parse_city(components[0], result);

    parse_links(
        City_links{
            .city = name,
            .links = components[1],
        },
        result);
}

/// Given a graph in which all forward edges have been added, adds
/// the reverse edges to the graph.
void
add_reverse_edges(Map_test &result)
{
    for (auto const &[src, node_data] : result.network)
    {
        for (std::string const *dest : node_data.edges)
        {
            if (!result.network.contains(*dest))
            {
                std::cerr << "Outgoing link found to nonexistent city '" + *dest
                                 + "'\n";
                std::abort();
            }
            result.network.at(*dest).edges.insert(&src);
        }
    }
}

/// Given a graph, confirms all nodes are at distinct locations.
void
validate_locations(Map_test const &test)
{
    std::map<Point, std::string const *> locations{};
    for (auto const &loc : test.network)
    {
        auto const inserted = locations.try_emplace(
            test.network.at(loc.first).coordinates, &loc.first);
        if (!inserted.second)
        {
            throw std::runtime_error(loc.first + " is at the same location as "
                                     + *inserted.first->second);
        }
    }
}

} // namespace

/// @brief Given a stream pointing at a test case for a map,
/// pulls the data from that test case.
/// @param source The stream containing the test case.
/// @return A test case from the file.
/// @throws ErrorException If an error occurs or the file is invalid.
Map_test
load_map(std::istream &source)
{
    Map_test result;

    for (std::string line; std::getline(source, line);)
    {
        // Skip blank lines or comments.
        if (trim(line).empty() || line.starts_with("#"))
        {
            continue;
        }

        parse_city_line(line, result);
    }

    add_reverse_edges(result);
    validate_locations(result);
    result.x_data_bounds.min = std::numeric_limits<float>::infinity();
    result.y_data_bounds.min = std::numeric_limits<float>::infinity();
    result.x_data_bounds.max = -std::numeric_limits<float>::infinity();
    result.y_data_bounds.max = -std::numeric_limits<float>::infinity();
    for (auto const &[_, node] : result.network)
    {
        result.x_data_bounds.min
            = std::min(result.x_data_bounds.min, node.coordinates.x);
        result.y_data_bounds.min
            = std::min(result.y_data_bounds.min, node.coordinates.y);
        result.x_data_bounds.max
            = std::max(result.x_data_bounds.max, node.coordinates.x);
        result.y_data_bounds.max
            = std::max(result.y_data_bounds.max, node.coordinates.y);
    }
    result.x_data_bounds.min -= file_coordinate_pad;
    result.y_data_bounds.min -= file_coordinate_pad;
    result.x_data_bounds.max += file_coordinate_pad;
    result.y_data_bounds.max += file_coordinate_pad;
    return result;
}

std::ostream &
operator<<(std::ostream &out, Point const &p)
{
    return out << "{" << p.x << "," << p.y << "}";
}

bool
operator==(Point const &lhs, Point const &rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

std::partial_ordering
operator<=>(Point const &lhs, Point const &rhs)
{
    auto const cmp = lhs.x <=> rhs.x;
    return cmp == std::partial_ordering::equivalent ? lhs.y <=> rhs.y : cmp;
}

} // namespace Dancing_links
