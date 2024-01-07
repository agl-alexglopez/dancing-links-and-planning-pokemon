import dancing_links;
#include "quad.hh"
#include "triangle.hh"
#include "window.hh"

#include <array>
#include <chrono>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

namespace Dx = Dancing_links;
namespace {

constexpr std::string_view frag_file = "gui/frag/basic.frag";
constexpr std::string_view vert_file = "gui/vert/basic.vert";

std::string read_shader( std::string_view filename )
{
  const std::ifstream f( filename.data() );
  std::stringstream s;
  s << f.rdbuf();
  return { s.str() };
}

} // namespace

int run()
{
  try {
    std::ifstream gen( std::string { "data/dst/Gen-1-Kanto.dst" } );
    const Dx::Type_encoding tester( "Fire" );
    const Dx::Pokemon_test interactions = Dx::load_pokemon_generation( gen );
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
    const std::string vert = read_shader( vert_file );
    const std::string frag = read_shader( frag_file );
    const Gui::Triangle tri( { vert, frag },
                             {
                               { -0.5F, -0.5F },
                               { 0.5F, -0.5F },
                               { 0.5F, 0.5F },
                             } );
    const Gui::Quad quad( { vert, frag },
                          {
                            { -0.5F, -0.5F },
                            { 0.5F, -0.5F },
                            { 0.5F, 0.5F },
                            { -0.5F, 0.5F },
                          } );
    auto this_time = std::chrono::high_resolution_clock::now();
    auto last_time = this_time;
    std::array<std::function<void()>, 2> shapes = { Gui::Quad::draw, Gui::Triangle::draw };
    bool toggle_index = false;
    while ( !window.should_close() ) {
      Gui::Window::clear();
      this_time = std::chrono::high_resolution_clock::now();
      auto time_since_last = std::chrono::duration_cast<std::chrono::seconds>( this_time - last_time );
      if ( time_since_last >= std::chrono::seconds( 2 ) ) {
        last_time = this_time;
        toggle_index = !toggle_index;
      }
      shapes.at( toggle_index )();
      window.poll();
    }
    return 0;
  } catch ( const std::exception& e ) {
    std::cerr << "exception caught: " << e.what() << std::flush;
    return 1;
  }
}

int main()
{
  return run();
}
