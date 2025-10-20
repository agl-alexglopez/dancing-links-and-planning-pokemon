add_executable(pokemon_gui pokemon_gui.cc)
target_link_libraries(pokemon_gui dancing_links gui)

add_executable(pokemon_cli pokemon_cli.cc)
target_link_libraries(pokemon_cli dancing_links)
