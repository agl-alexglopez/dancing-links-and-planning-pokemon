add_library(ranked_set INTERFACE ranked_set.hh)
add_library(type_encoding type_encoding.hh type_encoding.cc)
add_library(resistance resistance.hh resistance.cc)
target_link_libraries(resistance PUBLIC type_encoding)

add_library(pokemon_links pokemon_links.hh pokemon_links.cc)
target_link_libraries(pokemon_links PRIVATE ranked_set resistance type_encoding)

add_library(type_resistance INTERFACE type_resistance.hh)
target_link_libraries(type_resistance INTERFACE resistance type_encoding)
add_library(dancing_links INTERFACE dancing_links.hh)
target_link_libraries(dancing_links INTERFACE pokemon_links)
