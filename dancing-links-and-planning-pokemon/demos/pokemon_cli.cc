#include "dancing_links.hh"
#include "pokemon_parser.hh"
#include "ranked_set.hh"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <map>
#include <span>
#include <string>
#include <string_view>

namespace Dx = Dancing_links;
namespace {

enum class Solution_type
{
  exact,
  overlapping
};

struct Runner
{
  std::string map {};
  std::map<Dx::Type_encoding, std::set<Dx::Resistance>> interactions {};
  std::set<std::string> selected_gyms {};
  Dx::Pokemon_links::Coverage_type type { Dx::Pokemon_links::Coverage_type::defense };
  Solution_type sol_type { Solution_type::exact };
};

int run( std::span<const char* const> args );

} // namespace

/* Run this program from the root of the repository. That way your request for any generation
 * map will be handled correctly becuase the map you are requesting will be in the /data/dst
 * or /data/json relative to the root.
 */
int main( int argc, char** argv )
{
  const auto args = std::span<const char* const> { argv, static_cast<size_t>( argc ) }.subspan( 1 );
  if ( args.empty() ) {
    return 0;
  }
  return run( args );
}

namespace {

int run( const std::span<const char* const> args )
{
  try {
    Runner runner;
    for ( const auto& arg : args ) {
      std::string_view arg_str { arg };
      if ( arg_str.find( '/' ) != std::string::npos ) {
        if ( !runner.interactions.empty() ) {
          std::cerr << "Cannot load multiple generations simultaneously. Specify one.\n";
          return 1;
        }
        const auto owned = std::string( arg_str );
        std::ifstream f( owned );
        runner.interactions = load_interaction_map( f );
        runner.map = owned.substr( owned.find_last_of( '/' ) + 1 );
      } else if ( arg_str.starts_with( 'G' ) || arg_str.starts_with( 'E' ) ) {
        runner.selected_gyms.emplace( arg_str );
      } else if ( arg_str == "A" ) {
        runner.type = Dx::Pokemon_links::attack;
      } else if ( arg_str == "D" ) {
        runner.type = Dx::Pokemon_links::defense;
      } else if ( arg_str == "E" ) {
        runner.sol_type = Solution_type::exact;
      } else if ( arg_str == "O" ) {
        runner.sol_type = Solution_type::overlapping;
      } else {
        std::cerr << "Unknown argument: " << arg_str << "\n";
        return 1;
      }
    }
    std::set<Ranked_set<Dx::Type_encoding>> result {};
    Dx::Pokemon_links links( runner.interactions, runner.type );
    if ( !runner.selected_gyms.empty() ) {
      std::set<Dx::Type_encoding> subset {};
      switch ( runner.type ) {
        case Dx::Pokemon_links::Coverage_type::attack: {
          subset = load_selected_gyms_attacks( runner.map, runner.selected_gyms );
        } break;
        case Dx::Pokemon_links::Coverage_type::defense: {
          subset = load_selected_gyms_defenses( runner.map, runner.selected_gyms );
        } break;
      }
      Dx::hide_items_except( links, subset );
    }
    int depth_limit = runner.type == Dx::Pokemon_links::Coverage_type::attack ? 24 : 6;
    result = runner.sol_type == Solution_type::exact ? Dx::exact_cover_stack( links, depth_limit )
                                                     : Dx::overlapping_cover_stack( links, depth_limit );
    for ( const auto& res : result ) {
      std::cout << res;
    }
    return 0;
  } catch ( ... ) {
    std::cerr << "Pokemon CLI application encountered exception.\n";
    return 1;
  }
}

} // namespace
