#include "dancing_links.hh"
#include "pokemon_parser.hh"
#include "type_resistance.hh"
#include "window.hh"

#include <fstream>
#include <map>

namespace Dx = Dancing_links;
namespace {} // namespace

int main()
{

  std::ifstream gen( std::string { "data/dst/Gen-1-Kanto.dst" } );
  const Dx::Type_encoding tester( "Fire" );
  const Pokemon_test interactions = load_pokemon_generation( gen );
  std::cout << "Hello from the GUI.\n";
  std::cout << "Tester Type_encoding is: " << tester << "\n";
  std::cout << "Generation size is: " << interactions.interactions.size() << "\n";
  std::cout << "Generation map city count is: " << interactions.gen_map.network.size() << "\n";
  Gui::Window window( { .width = 1280,
                        .height = 720,
                        .title = std::string { "Pokemon Type Coverage" },
                        .monitor = nullptr,
                        .share = nullptr } );
  while ( !window.should_close() ) {
    Gui::Window::poll();
  }
}
