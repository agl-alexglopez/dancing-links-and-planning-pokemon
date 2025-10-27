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

class Point {
  public:
    constexpr Point() = default;
    constexpr Point(float const user_x, float const user_y)
        : x(user_x), y(user_y)
    {}
    float x{0.0};
    float y{0.0};
}; // class Point

std::ostream &operator<<(std::ostream &out, Point const &p);
bool operator==(Point const &lhs, Point const &rhs);
std::partial_ordering operator<=>(Point const &lhs, Point const &rhs);

/// Type representing a test case for the Disaster Preparation problem.
struct Map_test
{
    std::map<std::string, std::set<std::string>> network; // The network
    std::map<std::string, Point> city_locations;          // Drawing.
};

/// @brief Given a stream pointing at a test case for Disaster Preparation,
/// pulls the data from that test case.
/// @param source The stream containing the test case.
/// @return A test case from the file.
/// @throws ErrorException If an error occurs or the file is invalid.
Map_test load_map(std::istream &source);

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

    // Insert the city location
    result.city_locations.insert({
        name,
        {
            std::stof(
                components[static_cast<uint8_t>(Name_components::x_coord)]),
            std::stof(
                components[static_cast<uint8_t>(Name_components::y_coord)]),
        },
    });

    // Insert an entry for the city into the road network.
    result.network[name] = {};
    return name;
}

/// Reads the links out of the back half of the line of a file,
/// adding them to the road network.
void
parse_links(City_links const &cl, Map_test &result)
{
    // It's possible that there are no outgoing links.
    if (trim(cl.links).empty())
    {
        result.network[cl.city] = {};
        return;
    }

    auto components = string_split(cl.links, ',');
    for (std::string const &dest : components)
    {
        // Clean up all whitespace and make sure that we didn't
        // discover an empty entry.
        std::string const clean_name = trim(dest);
        if (clean_name.empty())
        {
            std::cerr << "Blank name in list of outgoing cities?\n";
            std::abort();
        }

        // Confirm this isn't a dupe.
        if (result.network.at(cl.city).contains(clean_name))
        {
            std::cerr << "City appears twice in outgoing list?\n";
            std::abort();
        }

        result.network.at(cl.city).insert(clean_name);
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
        {
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
    for (auto const &source : result.network)
    {
        for (std::string const &dest : source.second)
        {
            if (!result.network.contains(dest))
            {
                std::cerr << "Outgoing link found to nonexistent city '" + dest
                                 + "'\n";
                std::abort();
            }
            result.network[dest].insert(source.first);
        }
    }
}

/// Given a graph, confirms all nodes are at distinct locations.
void
validate_locations(Map_test const &test)
{
    std::map<Point, std::string> locations{};
    for (auto const &loc : test.city_locations)
    {
        auto const inserted = locations.try_emplace(
            test.city_locations.at(loc.first), loc.first);
        if (!inserted.second)
        {
            throw std::runtime_error(loc.first + " is at the same location as "
                                     + inserted.first->second);
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
