add_library(map_parser map_parser.hh map_parser.cc)
target_link_libraries(map_parser point)

include(FetchContent)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)


add_library(pokemon_parser pokemon_parser.hh pokemon_parser.cc)
target_link_libraries(pokemon_parser PRIVATE nlohmann_json::nlohmann_json map_parser type_encoding type_resistance point)

add_executable(pokemon_gui pokemon_gui.cc)
target_link_libraries(pokemon_gui dancing_links pokemon_parser type_resistance window triangle quad)
