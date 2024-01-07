module;
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <array>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <istream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
export module dancing_links:pokemon_parser;
import :map_parser;
import :resistance;
import :type_encoding;

export namespace Dancing_links {

/* Leave a comment at the first line of the Pokemon Generation .dst file you want to construct with
 * the pokemon generation in base 10 numbers. This will determine the types availabe for the given
 * map and generation cover problem. Any of the 9 generations are acceptable. If the
 * first line does not match one of these strings I will just include all pokemon types in the cover
 * problem. Like this.
 *
 * # 1
 * # 2
 * # 3
 * # 4
 * # 5
 * # 6
 * # 7
 * # 8
 * # 9
 *
 * This also means you should start every file with some sort of comment labelling it with
 * identifying info, even if there is not generation specification.
 */

struct Pokemon_test
{
  /* This map will hold all types--dual types included--and a map of defense multipliers ranging
   * from x0.0,x0.25,x0.5,x1.0,x2.0,x4.0. In these maps will be the type as the key and
   * all the single attack types that have that multiplier against the current type. There are
   * 18 single attack types. The highest level map holds 162 type keys that exist in Pokemon as
   * of December 2022. However, depending on which generation map you decide to draw some types
   * might be missing. For example, generation one Pokemon did not have types like Fairy, Dark,
   * or Steel.
   */
  std::map<Type_encoding, std::set<Resistance>> interactions;
  Map_test gen_map {};
};

/**
 * @brief load_pokemon_generation  builds the PokemonTest needed to interact with a generation's map
 *                                 in the Pokemon Planning GUI.
 * @param source                   the file with the map that gives us info on which gen to build.
 * @return                         the completed pokemon test with map drawing and Pokemon info.
 */
Pokemon_test load_pokemon_generation( std::istream& source );

/**
 * @brief load_interaction_map  builds the PokemonTest needed to interact with a generation's map
 *                              in the Pokemon Planning GUI.
 * @param source                the file with the map that gives us info on which gen to build.
 * @return                      the completed pokemon test with map drawing and Pokemon info.
 */
std::map<Type_encoding, std::set<Resistance>> load_interaction_map( std::istream& source );

/**
 * @brief load_selected_gyms_defenses  when interacting with the GUI, the user can choose subsets of
 *                                     gyms on the current map they are viewing. If they make these
 *                                     selections we can load in the defensive types that are present
 *                                     at those gyms. These are intended to be fed to a PokemonLinks
 *                                     solver as the items we must defend against.
 * @param selectedMap                  the current .dst file we are viewing.
 * @param selectedGyms                 the gyms G1-E4 that we are considering attacking.
 * @return                             the set of all defensive types present in the selection of gyms.
 */
std::set<Type_encoding> load_selected_gyms_defenses( const std::string& selected_map,
                                                     const std::set<std::string>& selected_gyms );

/**
 * @brief load_selected_gyms_attacks  the user interacting with the GUI may want to defend themselves
 *                                    from only a selection of gyms. We will get the gym info for the
 *                                    selected map and return the types of attacks present across all
 *                                    of those selections. This set can then be passed to the
 *                                    PokemonLinks dancing links class as a second parameter.
 * @param selected_map                the current map the user interacts with.
 * @param selected                    the gyms they have selected.
 * @return                            a set of all attack types present across those gyms.
 */
std::set<Type_encoding> load_selected_gyms_attacks( const std::string& selected_map,
                                                    const std::set<std::string>& selected );

} // namespace Dancing_links

namespace Dancing_links {

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

const std::array<std::pair<std::string_view, Multiplier>, 6> damage_multipliers = { {
  { "immune", imm },
  { "quarter", f14 },
  { "half", f12 },
  { "normal", nrm },
  { "double", dbl },
  { "quad", qdr },
} };

const Multiplier& get_multiplier( const std::string& key )
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

void set_resistances( std::map<Type_encoding, std::set<Resistance>>& result,
                      const Type_encoding& new_type,
                      const nlo::json& multipliers )
{
  for ( const auto& [multiplier, types_in_multiplier] : multipliers.items() ) {
    const Multiplier multiplier_tag = get_multiplier( multiplier );
    for ( const auto& t : types_in_multiplier ) {
      const std::string& type = t;
      result[new_type].insert( { Type_encoding( type ), multiplier_tag } );
    }
  }
}

std::map<Type_encoding, std::set<Resistance>> from_json_to_map( int generation )
{
  const std::string_view path_to_json = generation_json_files.at( generation );
  const nlo::json json_types = get_json_object( path_to_json );
  std::map<Type_encoding, std::set<Resistance>> result = {};
  for ( const auto& [type, resistances] : json_types.items() ) {
    const Type_encoding encoded( type );
    result.insert( { encoded, {} } );
    set_resistances( result, encoded, resistances );
  }
  return result;
}

std::map<Type_encoding, std::set<Resistance>> load_generation_from_json( std::istream& source )
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

std::map<Type_encoding, std::set<Resistance>> load_interaction_map( std::istream& source )
{
  return load_generation_from_json( source );
}

std::set<Type_encoding> load_selected_gyms_defenses( const std::string& selected_map,
                                                     const std::set<std::string>& selected_gyms )
{
  if ( selected_gyms.empty() ) {
    std::cerr << "Requesting to load zero gyms check selected gyms input.\n";
  }
  const nlo::json map_data = get_json_object( json_all_maps_file );
  std::set<Type_encoding> result = {};

  const nlo::json& gym_keys = map_data.at( selected_map );

  std::vector<std::string_view> confirmed {};
  confirmed.reserve( selected_gyms.size() );
  for ( const auto& [gym, attack_defense_map] : gym_keys.items() ) {
    if ( !selected_gyms.contains( gym ) ) {
      continue;
    }
    confirmed.push_back( gym );
    for ( const auto& t : attack_defense_map.at( gym_defense_key ) ) {
      const std::string& type = t;
      result.insert( Type_encoding( type ) );
    }
  }
  if ( confirmed.size() != selected_gyms.size() ) {
    std::cerr << "Mismatch occured for " << selected_map << " gym selection.\nRequested: ";
    for ( const auto& s : selected_gyms ) {
      std::cerr << s << " ";
    }
    std::cerr << "\nConfirmed: ";
    for ( const auto& s : confirmed ) {
      std::cerr << s << " ";
    }
    std::cerr << "\n";
  }
  // This will be a much smaller set.
  return result;
}

std::set<Type_encoding> load_selected_gyms_attacks( const std::string& selected_map,
                                                    const std::set<std::string>& selected_gyms )
{
  if ( selected_gyms.empty() ) {
    std::cerr << "Requesting to load zero gyms check selected gyms input.\n";
  }
  const nlo::json map_data = get_json_object( json_all_maps_file );
  std::set<Type_encoding> result = {};
  const nlo::json& selection = map_data.at( selected_map );

  std::vector<std::string_view> confirmed {};
  confirmed.reserve( selected_gyms.size() );
  for ( const auto& [gym, attack_defense_map] : selection.items() ) {
    if ( !selected_gyms.contains( gym ) ) {
      continue;
    }
    confirmed.push_back( gym );
    for ( const auto& t : attack_defense_map.at( gym_attacks_key ) ) {
      const std::string& type = t;
      result.insert( Type_encoding( type ) );
    }
  }
  if ( confirmed.size() != selected_gyms.size() ) {
    std::cerr << "Mismatch occured for " << selected_map << " gym selection.\nRequested: ";
    for ( const auto& s : selected_gyms ) {
      std::cerr << s << " ";
    }
    std::cerr << "\nConfirmed: ";
    for ( const auto& s : confirmed ) {
      std::cerr << s << " ";
    }
    std::cerr << "\n";
  }
  // Return a simple set rather than altering every type's resistances in a large map.
  return result;
}

} // namespace Dancing_links
