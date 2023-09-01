#include "dancing_links.hh"
#include "pokemon_parser.hh"
#include "triangle.hh"
#include "type_resistance.hh"
#include "window.hh"

#include <fstream>
#include <string_view>
#include <vector>

namespace Dx = Dancing_links;
namespace {

constexpr std::string_view triangle_vert = R"glsl(
#version 330 core

layout(location = 0) in vec4 position;

void main(){
  gl_Position = position;
}
)glsl";

constexpr std::string_view triangle_frag = R"glsl(
#version 330 core

layout(location = 0) out vec4 color;

void main(){
  color = vec4(0.4, 0.3, 0.8, 0.7);
}
)glsl";

} // namespace

int main()
{
  try {
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
    if ( window.error() ) {
      std::cerr << "window could not open\n";
      return 1;
    }
    const Gui::Triangle triangle( { triangle_vert, triangle_frag },
                                  { { -0.75F, -0.75F }, { 0.0F, 0.75F }, { 0.75F, -0.75F } } );
    while ( !window.should_close() ) {
      Gui::Window::clear();
      Gui::Triangle::draw( { 0, 3 } );
      window.poll();
    }
  } catch ( const std::exception& e ) {
    std::cerr << "exception caught: " << e.what() << std::flush;
    return 1;
  }
  return 0;
}
