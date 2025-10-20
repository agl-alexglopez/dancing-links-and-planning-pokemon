import dancing_links;
import gui;
#include <exception>
#include <fstream>
#include <iostream>
#include <string>

namespace Dx = Dancing_links;
namespace {

int
run()
{
    try
    {
        std::ifstream gen(std::string{"data/dst/Gen-1-Kanto.dst"});
        const Dx::Type_encoding tester("Fire");
        const Dx::Pokemon_test interactions = Dx::load_pokemon_generation(gen);
        std::cout << "Hello from the GUI.\n";
        std::cout << "Tester Type_encoding is: " << tester << "\n";
        std::cout << "Generation size is: " << interactions.interactions.size()
                  << "\n";
        std::cout << "Generation map city count is: "
                  << interactions.gen_map.network.size() << "\n";
        if (!Gui::run())
        {
            return 1;
        }
        return 0;
    } catch (const std::exception &e)
    {
        std::cerr << "exception caught: " << e.what() << std::flush;
        return 1;
    }
}

} // namespace

int
main()
{
    return run();
}
