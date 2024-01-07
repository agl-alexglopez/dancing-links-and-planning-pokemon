module;
#include <algorithm>
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
#include <string_view>
#include <vector>
export module dancing_links:map_parser;
import :point;

//////////////////////////////////////   Exported Interface   /////////////////////////////////////////////////////

export namespace Dancing_links {

/**
 * Type representing a test case for the Disaster Preparation problem.
 */
struct Map_test
{
  std::map<std::string, std::set<std::string>> network; // The road network
  std::map<std::string, Point> city_locations;          // Where each city should be drawn
};

/**
 * Given a stream pointing at a test case for Disaster Preparation,
 * pulls the data from that test case.
 *
 * @param source The stream containing the test case.
 * @return A test case from the file.
 * @throws ErrorException If an error occurs or the file is invalid.
 */
Map_test load_map( std::istream& source );

} // namespace Dancing_links

//////////////////////////////////////   Implementation   /////////////////////////////////////////////////////////

namespace Dancing_links {

/* Everything in here is private to this file. */
namespace {

constexpr std::string_view whitespace = " \n\r\t\f\v";
constexpr std::string_view dst_file_regex
  = R"(^([A-Za-z0-9 .\-]+)\(\s*(-?[0-9]+(?:\.[0-9]+)?)\s*,\s*(-?[0-9]+(?:\.[0-9]+)?)\s*\)$)";

enum Name_components
{
  whole_string,
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

std::string trim( const std::string& str )
{
  const size_t begin = str.find_first_not_of( whitespace );
  if ( begin == std::string::npos ) {
    return "";
  }
  const size_t end = str.find_last_not_of( whitespace );
  return str.substr( begin, end - begin + 1 );
}

std::vector<std::string> string_split( const std::string& str, const char delim )
{
  std::vector<std::string> components {};
  std::stringstream ss( str );
  while ( ss.good() ) {
    std::string substr;
    std::getline( ss, substr, delim );
    components.push_back( substr );
  }
  return components;
}

/* Given city information in the form
 *
 *     City_name (X, Y)
 *
 * Parses out the name and the X/Y coordinate, returning the
 * name, and filling in the MapTest with what's found.
 */
std::string parse_city( const std::string& city_info, Map_test& result )
{
  /* Split on all the delimiters and confirm we've only got
   * three components.
   */
  const std::regex pattern( std::string { dst_file_regex } );
  std::smatch components;
  const std::string to_match = trim( city_info );

  if ( !regex_match( to_match, components, pattern ) ) {
    std::cerr << "Can't parse this data; is it city info? " << city_info << "\n";
    std::abort();
  }

  /* There are four components here, actually: the whole match,
   * plus each subexpression we care about.
   */
  if ( components.size() != num_components ) {
    std::cerr << "Could not find all components?\n";
    std::abort();
  }

  /* We're going to get back some extra leading or trailing
   * whitespace here, so peel it off.
   */
  std::string name = trim( components[city_name] );
  if ( name.empty() ) {
    std::cerr << "City names can't be empty.\n";
    std::abort();
  }

  /* Insert the city location */
  result.city_locations.insert( { name, { std::stof( components[x_coord] ), std::stof( components[y_coord] ) } } );

  /* Insert an entry for the city into the road network. */
  result.network[name] = {};
  return name;
}

/* Reads the links out of the back half of the line of a file,
 * adding them to the road network.
 */
void parse_links( const City_links& cl, Map_test& result )
{
  /* It's possible that there are no outgoing links. */
  if ( trim( cl.links ).empty() ) {
    result.network[cl.city] = {};
    return;
  }

  auto components = string_split( cl.links, ',' );
  for ( const std::string& dest : components ) {
    /* Clean up all whitespace and make sure that we didn't
     * discover an empty entry.
     */
    const std::string clean_name = trim( dest );
    if ( clean_name.empty() ) {
      std::cerr << "Blank name in list of outgoing cities?\n";
      std::abort();
    }

    /* Confirm this isn't a dupe. */
    if ( result.network.at( cl.city ).contains( clean_name ) ) {
      std::cerr << "City appears twice in outgoing list?\n";
      std::abort();
    }

    result.network.at( cl.city ).insert( clean_name );
  }
}

/* Parses one line out of the file and updates the network with what
 * it found. This will only add edges in the forward direction as
 * a safety measure; edges are reversed later on.
 */
void parse_city_line( const std::string& line, Map_test& result )
{
  /* Search for a colon on the line. The split function will only return a
   * single component if there are no outgoing links specified.
   */
  const auto num_colons = std::count( line.begin(), line.end(), ':' );
  if ( num_colons != 1 ) {
    std::cerr << "Each data line should have exactly one colon on it.\n";
    std::abort();
  }

  /* Split the line into the city name/location and the list
   * of outgoing cities.
   */
  auto components = string_split( line, ':' );
  if ( components.empty() ) {
    std::cerr << "Data line appears to have no city information.\n";
    std::abort();
  }

  /* Create a dummy list of outgoing cities if one doesn't already exist. */
  if ( components.size() == 1 ) {
    components.emplace_back( "" );
  }

  const std::string name = parse_city( components[0], result );

  parse_links( { name, components[1] }, result );
}

/* Given a graph in which all forward edges have been added, adds
 * the reverse edges to the graph.
 */
void add_reverse_edges( Map_test& result )
{
  for ( const auto& source : result.network ) {
    for ( const std::string& dest : source.second ) {
      if ( result.network.find( dest ) == result.network.end() ) {
        std::cerr << "Outgoing link found to nonexistent city '" + dest + "'\n";
        std::abort();
      }
      result.network[dest].insert( source.first );
    }
  }
}

/* Given a graph, confirms all nodes are at distinct locations. */
void validate_locations( const Map_test& test )
{
  std::map<Point, std::string> locations {};
  for ( const auto& loc : test.city_locations ) {
    if ( locations.find( test.city_locations.at( loc.first ) ) != locations.end() ) {
      throw std::runtime_error( loc.first + " is at the same location as "
                                + locations[test.city_locations.at( loc.first )] );
    }
    locations[test.city_locations.at( loc.first )] = loc.first;
  }
}

} // namespace

/**
 * Given a stream pointing at a test case for a map,
 * pulls the data from that test case.
 *
 * @param source The stream containing the test case.
 * @return A test case from the file.
 * @throws ErrorException If an error occurs or the file is invalid.
 */
Map_test load_map( std::istream& source )
{
  Map_test result;

  for ( std::string line; std::getline( source, line ); ) {
    /* Skip blank lines or comments. */
    if ( trim( line ).empty() || line.starts_with( "#" ) ) {
      continue;
    }

    parse_city_line( line, result );
  }

  add_reverse_edges( result );
  validate_locations( result );
  return result;
}

} // namespace Dancing_links
