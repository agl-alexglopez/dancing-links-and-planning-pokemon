/**
 * Author: Keith Schwarz and Stanford Course Staff
 * File: MapParser.h
 * -----------------
 * This file was written by Keith Schwarz and Stanford Course Staff to parse maps written in
 * logical coordinates in files with the .dst extension. It is a helpful way to load the
 * information into C++ std formats and use in a GUI. I modified it to use the STL containers.
 */
#ifndef MapParser_Included
#define MapParser_Included
#include "gtypes.h"
#include <map>
#include <set>
#include <string>
#include <istream>

/**
 * Type representing a test case for the Disaster Preparation problem.
 */
struct MapTest {
    std::map<std::string, std::set<std::string>> network; // The road network
    std::map<std::string, GPoint> cityLocations;     // Where each city should be drawn
};

/**
 * Given a stream pointing at a test case for Disaster Preparation,
 * pulls the data from that test case.
 *
 * @param source The stream containing the test case.
 * @return A test case from the file.
 * @throws ErrorException If an error occurs or the file is invalid.
 */
MapTest loadDisaster(std::istream& source);

#endif
