/**
 * MIT License
 *
 * Copyright (c) 2023 Alex G. Lopez
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Author: Alex Lopez
 * File: PokemonParser.cpp
 * -----------------------
 * This file handles getting Pokemon Data from JSON files and turning it into C++ std compliant
 * data forms like maps and sets. This implementation is heavily dependent on QT libraries for
 * parsing the JSON. I chose to turn the data into C++ std forms to use with the program, even
 * though it would have been possible to just read the data from the files in directly during the
 * program runtime, in case QT becomes deprecated or I want to move this to a CLI implementation.
 * Then, I just need to rewrite the parser that gets me data from JSON files rather than rewrite
 * the whole program.
 *
 * To be more thorough I could seperate out each generation to its own .json file and just directly
 * parse the correct generation information to a map. Right now, I am filtering out types from
 * a map that contains the most recent generation typing information if I want an older generation.
 */
#include "pokemon_parser.hh"
#include "map_parser.hh"
#include "resistance.hh"
#include "type_encoding.hh"

#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <array>
#include <exception>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace {

namespace nlo = nlohmann;

constexpr std::string_view json_all_maps_file = "data/json/all-maps.json";
constexpr std::string_view gym_attacks_key = "attack";
constexpr std::string_view gym_defense_key = "defense";

// There is no 0th generation so we will make it easier to select the right file by leaving 0 "".
constexpr std::array<std::string_view, 10> generation_json_files = {
  "",
  "data/json/gen-1-types.json",
  "data/json/gen-2-types.json",
  "data/json/gen-3-types.json",
  "data/json/gen-4-types.json",
  "data/json/gen-5-types.json",
  "data/json/gen-6-types.json",
  "data/json/gen-7-types.json",
  "data/json/gen-8-types.json",
  "data/json/gen-9-types.json",
};

const std::array<std::pair<std::string_view, Dx::Multiplier>, 6> damage_multipliers = { {
  { "immune", Dx::imm },
  { "quarter", Dx::f14 },
  { "half", Dx::f12 },
  { "normal", Dx::nrm },
  { "double", Dx::dbl },
  { "quad", Dx::qdr },
} };

const Dx::Multiplier& get_multiplier( const std::string& key )
{
  for ( const auto& mult : damage_multipliers ) {
    if ( mult.first == key ) {
      return mult.second;
    }
  }
  throw std::logic_error( "Out of bounds. Key not found. " );
}

void print_generation_error( const std::exception& ex )
{
  std::cerr << "Found this: " << ex.what();
  std::cerr << "Could not choose the correct generation from first line of file.\n";
  std::cerr << "Comment first line as follows. Any other comment can start on the next line\n";
  std::cerr << "# 1\n";
  std::cerr << "# Above, I want to load in this map as Generation One. Choose 1-9\n";
}

nlo::json get_json_object( std::string_view path_to_json )
{
  std::ifstream json_file( std::string { path_to_json } );
  if ( !json_file.is_open() ) {
    std::cerr << "Could not open json file: ." << path_to_json << "\n";
    json_file.close();
    std::abort();
  }
  nlo::json map_data = nlo::json::parse( json_file );
  json_file.close();
  if ( map_data.is_discarded() ) {
    std::cerr << "Error parsing all map data to json object.\n";
    std::abort();
  }
  return map_data;
}

void set_resistances( std::map<Dx::Type_encoding, std::set<Dx::Resistance>>& result,
                      const Dx::Type_encoding& new_type,
                      const nlo::json& multipliers )
{
  for ( const auto& [multiplier, types_in_multiplier] : multipliers.items() ) {
    const Dx::Multiplier multiplier_tag = get_multiplier( multiplier );
    for ( const auto& t : types_in_multiplier ) {
      const std::string& type = t;
      result[new_type].insert( { Dx::Type_encoding( type ), multiplier_tag } );
    }
  }
}

std::map<Dx::Type_encoding, std::set<Dx::Resistance>> from_json_to_map( int generation )
{
  const std::string_view path_to_json = generation_json_files.at( generation );
  const nlo::json json_types = get_json_object( path_to_json );
  std::map<Dx::Type_encoding, std::set<Dx::Resistance>> result = {};
  for ( const auto& [type, resistances] : json_types.items() ) {
    const Dx::Type_encoding encoded( type );
    result.insert( { encoded, {} } );
    set_resistances( result, encoded, resistances );
  }
  return result;
}

std::map<Dx::Type_encoding, std::set<Dx::Resistance>> load_generation_from_json( std::istream& source )
{
  std::string line;
  std::getline( source, line );
  const std::string after_hashtag = line.substr( 1, line.length() - 1 );
  try {
    const int generation = std::stoi( after_hashtag );
    return from_json_to_map( generation );
  } catch ( const std::out_of_range& oor ) {
    print_generation_error( oor );
    std::abort();
  } catch ( const std::invalid_argument& ia ) {
    print_generation_error( ia );
    std::abort();
  }
}

} // namespace

Pokemon_test load_pokemon_generation( std::istream& source )
{
  Pokemon_test generation;
  generation.interactions = load_generation_from_json( source );
  generation.gen_map = load_map( source );
  return generation;
}

std::set<Dx::Type_encoding> load_selected_gyms_defenses( const std::string& selected_map,
                                                         const std::set<std::string>& selected_gyms )
{
  const nlo::json map_data = get_json_object( json_all_maps_file );
  std::set<Dx::Type_encoding> result = {};

  const nlo::json& gym_keys = map_data.at( selected_map );

  for ( const auto& [gym, attack_defense_map] : gym_keys.items() ) {
    if ( !selected_gyms.contains( gym ) ) {
      continue;
    }
    for ( const auto& t : attack_defense_map.at( gym_defense_key ) ) {
      const std::string& type = t;
      result.insert( Dx::Type_encoding( type ) );
    }
  }
  // This will be a much smaller map.
  return result;
}

std::set<Dx::Type_encoding> load_selected_gyms_attacks( const std::string& selected_map,
                                                        const std::set<std::string>& selected_gyms )
{
  const nlo::json map_data = get_json_object( json_all_maps_file );
  std::set<Dx::Type_encoding> result = {};
  const nlo::json& selection = map_data.at( selected_map );

  for ( const auto& [gym, attack_defense_map] : selection.items() ) {
    if ( !selected_gyms.contains( gym ) ) {
      continue;
    }
    for ( const auto& t : attack_defense_map.at( gym_attacks_key ) ) {
      const std::string& type = t;
      result.insert( Dx::Type_encoding( type ) );
    }
  }
  // Return a simple set rather than altering every type's resistances in a large map.
  return result;
}
