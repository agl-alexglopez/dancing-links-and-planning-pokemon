/**
 * Author: Keith Schwarz and Stanford Course Staff
 * File: MapParser.h
 * -----------------
 * This file was written by Keith Schwarz and Stanford Course Staff to parse maps written in
 * logical coordinates in files with the .dst extension. It is a helpful way to load the
 * information into C++ std formats and use in a GUI. I modified it to use the STL containers.
 */
#ifndef MAP_PARSER_HH
#define MAP_PARSER_HH

#include "point.hh"

#include <istream>
#include <map>
#include <set>
#include <string>

/**
 * Type representing a test case for the Disaster Preparation problem.
 */
struct Map_test
{
  std::map<std::string, std::set<std::string>> network; // The road network
  std::map<std::string, Gui::Point> city_locations;     // Where each city should be drawn
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

#endif // MAP_PARSER_HH
