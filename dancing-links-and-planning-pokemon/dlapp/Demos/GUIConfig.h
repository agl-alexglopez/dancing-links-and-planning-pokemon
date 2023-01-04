WINDOW_TITLE("Dancing Links and Planning Pokemon!")

RUN_TESTS_MENU_OPTION()
MENU_ORDER("PokemonGUI.cpp")

TEST_ORDER("PokemonLinks.cpp")

TEST_BARRIER("PokemonGUI.cpp", "PokemonLinks.cpp")
