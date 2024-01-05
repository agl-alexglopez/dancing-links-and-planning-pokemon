#include "dancing_links.hh"
#include "pokemon_parser.hh"
#include "ranked_set.hh"

#include <algorithm>
#include <array>
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

constexpr std::string_view nil = "\033[0m";
constexpr std::string_view ansi_yel = "\033[38;5;11m";
constexpr std::string_view ansi_red = "\033[38;5;9m";
constexpr std::string_view ansi_grn = "\033[38;5;10m";
// Obtained from hex color gist: https://gist.github.com/apaleslimghost/0d25ec801ca4fc43317bcff298af43c3
constexpr std::array<std::string_view, 18> type_colors = {
  // "Bug",
  "\033[38;2;166;185;26m",
  // "Dark",
  "\033[38;2;112;87;70m",
  // "Dragon",
  "\033[38;2;111;53;252m",
  // "Electric",
  "\033[38;2;247;208;44m",
  // "Fairy",
  "\033[38;2;214;133;173m",
  // "Fighting",
  "\033[38;2;194;46;40m",
  // "Fire",
  "\033[38;2;238;129;48m",
  // "Flying",
  "\033[38;2;169;143;243m",
  // "Ghost",
  "\033[38;2;115;87;151m",
  // "Grass",
  "\033[38;2;122;199;76m",
  // "Ground",
  "\033[38;2;226;191;101m",
  // "Ice",
  "\033[38;2;150;217;214m",
  // "Normal",
  "\033[38;2;168;167;122m",
  // "Poison",
  "\033[38;2;163;62;161m",
  // "Psychic",
  "\033[38;2;249;85;135m",
  // "Rock",
  "\033[38;2;182;161;54m",
  // "Steel",
  "\033[38;2;183;183;206m",
  // "Water",
  "\033[38;2;99;144;240m",
};

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
void print_solution_msg( const std::set<Ranked_set<Dx::Type_encoding>>& result, const Runner& runner );

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
  print_solution_msg( result, runner );
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
      const std::pair<std::string_view, std::string_view> type_pair = t.decode_type();
      const std::pair<uint64_t, std::optional<uint64_t>> type_indices = t.decode_indices();
      if ( type_indices.second ) {
        const auto output = std::string( type_colors.at( type_indices.first ) )
                              .append( type_pair.first )
                              .append( nil )
                              .append( "-" )
                              .append( type_colors.at( type_indices.second.value() ) )
                              .append( type_pair.second )
                              .append( nil );
        std::cout << "│" << output
                  << std::setw( static_cast<int>( max_name_width
                                                  - ( type_pair.first.size() + 1 + type_pair.second.size() ) ) )
                  << "";
      } else {
        const auto output
          = std::string( type_colors.at( type_indices.first ) ).append( type_pair.first ).append( nil );
        std::cout << "│" << output << std::setw( static_cast<int>( max_name_width - type_pair.first.size() ) )
                  << "";
      }
      ++col;
    }
    while ( col < max_set_len ) {
      std::cout << "│" << std::left << std::setw( max_name_width ) << "";
      ++col;
    }
    std::cout << "│\n";
    break_line( max_set_len );
  }
  print_solution_msg( result, runner );
  print_prep_message( items_options );
  return 0;
}

void print_prep_message( const Universe_sets& sets )
{
  const auto item_msg = std::string( "Trying to cover " )
                          .append( ansi_yel )
                          .append( std::to_string( sets.items.size() ) )
                          .append( " items\n\n" )
                          .append( nil );
  std::cout << item_msg;
  for ( const auto& type : sets.items ) {
    const std::pair<std::string_view, std::string_view> type_pair = type.decode_type();
    const std::pair<uint64_t, std::optional<uint64_t>> type_indices = type.decode_indices();
    if ( type_indices.second ) {
      const auto output = std::string( type_colors.at( type_indices.first ) )
                            .append( type_pair.first )
                            .append( nil )
                            .append( "-" )
                            .append( type_colors.at( type_indices.second.value() ) )
                            .append( type_pair.second )
                            .append( nil );
      std::cout << output << ", ";
    } else {
      const auto output
        = std::string( type_colors.at( type_indices.first ) ).append( type_pair.first ).append( nil );
      std::cout << output << ", ";
    }
  }
  const auto option_msg = std::string( "\n" )
                            .append( ansi_yel )
                            .append( std::to_string( sets.options.size() ) )
                            .append( " options" )
                            .append( nil )
                            .append( " are available:\n\n" );
  std::cout << "\n" << option_msg;
  for ( const auto& type : sets.options ) {
    const std::pair<std::string_view, std::string_view> type_pair = type.decode_type();
    const std::pair<uint64_t, std::optional<uint64_t>> type_indices = type.decode_indices();
    if ( type_indices.second ) {
      const auto output = std::string( type_colors.at( type_indices.first ) )
                            .append( type_pair.first )
                            .append( nil )
                            .append( "-" )
                            .append( type_colors.at( type_indices.second.value() ) )
                            .append( type_pair.second )
                            .append( nil );
      std::cout << output << ", ";
    } else {
      const auto output
        = std::string( type_colors.at( type_indices.first ) ).append( type_pair.first ).append( nil );
      std::cout << output << ", ";
    }
  }
  std::cout << "\n";
}

void print_solution_msg( const std::set<Ranked_set<Dx::Type_encoding>>& result, const Runner& runner )
{
  auto result_color = std::string( result.empty() ? ansi_red : ansi_grn );
  const auto msg = result_color.append( "\nFound " )
                     .append( std::to_string( result.size() ) )
                     .append( runner.sol_type == Solution_type::exact ? " exact" : " overlapping" )
                     .append( " sets of options that cover specified items.\n\n" )
                     .append( nil );
  std::cout << msg;
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