#include "dancing_links.hh"
#include "pokemon_parser.hh"
#include "ranked_set.hh"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <span>
#include <string>
#include <string_view>

namespace Dx = Dancing_links;
namespace {

constexpr size_t max_name_width = 17;
constexpr size_t digit_width = 3;

enum class Solution_type
{
  exact,
  overlapping
};

struct Universe_sets
{
  std::vector<Dx::Type_encoding> items;
  std::vector<Dx::Type_encoding> options;
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
int solve( const Runner& runner );
void print_prep_message( const Universe_sets& sets );
void break_line( size_t max_set_len );

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
    return solve( runner );
  } catch ( ... ) {
    std::cerr << "Pokemon CLI application encountered exception.\n";
    return 1;
  }
}

int solve( const Runner& runner )
{
  if ( runner.map.empty() || runner.map.empty() ) {
    std::cerr << "No data loaded from any map to solve.\n";
    return 1;
  }
  std::set<Ranked_set<Dx::Type_encoding>> result {};
  Dx::Pokemon_links links( runner.interactions, runner.type );
  if ( !runner.selected_gyms.empty() ) {
    std::set<Dx::Type_encoding> subset {};
    // Load in the opposite of our coverage type so we know what we attack/defend against in this subset of gyms.
    runner.type == Dx::Pokemon_links::Coverage_type::attack
      ? subset = load_selected_gyms_defenses( runner.map, runner.selected_gyms )
      : subset = load_selected_gyms_attacks( runner.map, runner.selected_gyms );
    Dx::hide_items_except( links, subset );
  }
  Universe_sets items_options = { Dx::items( links ), Dx::options( links ) };
  print_prep_message( items_options );
  const int depth_limit = runner.type == Dx::Pokemon_links::Coverage_type::attack ? 24 : 6;
  result = runner.sol_type == Solution_type::exact ? Dx::exact_cover_stack( links, depth_limit )
                                                   : Dx::overlapping_cover_stack( links, depth_limit );
  std::cout << "Found " << result.size() << ( runner.sol_type == Solution_type::exact ? " exact" : " overlapping" )
            << " sets of options that cover specified items.\n";
  if ( result.empty() ) {
    return 0;
  }
  const auto& largest_ranked_set = std::ranges::max(
    result, []( const Ranked_set<Dx::Type_encoding>& a, const Ranked_set<Dx::Type_encoding>& b ) {
      return a.size() < b.size();
    } );
  const size_t max_set_len = largest_ranked_set.size();
  for ( const auto& res : result ) {
    std::cout << std::left << std::setw( digit_width ) << res.rank();
    size_t col = 0;
    for ( const auto& t : res ) {
      const auto type = t.to_string();
      std::cout << "│" << std::left << std::setw( max_name_width ) << type;
      ++col;
    }
    while ( col < max_set_len ) {
      std::cout << "│" << std::left << std::setw( max_name_width ) << "";
      ++col;
    }
    std::cout << "│\n";
    break_line( max_set_len );
  }
  std::cout << "Found " << result.size() << ( runner.sol_type == Solution_type::exact ? " exact" : " overlapping" )
            << " sets of options that cover specified items.\n";
  print_prep_message( items_options );
  return 0;
}

void print_prep_message( const Universe_sets& sets )
{

  std::cout << "Trying to cover " << sets.items.size() << " items:\n";
  for ( const auto& type : sets.items ) {
    std::cout << type << ", ";
  }
  std::cout << "\n" << sets.options.size() << " options are available:\n";
  for ( const auto& type : sets.options ) {
    std::cout << type << ", ";
  }
  std::cout << "\n";
}

void break_line( size_t max_set_len )
{
  std::cout << std::left << std::setw( digit_width ) << "";
  std::cout << "├";
  for ( size_t col = 0; col < max_set_len; ++col ) {
    for ( size_t line = 0; line < max_name_width; ++line ) {
      std::cout << "─";
    }
    std::cout << ( col == max_set_len - 1 ? "┤" : "┼" );
  }
  std::cout << "\n";
}

} // namespace
