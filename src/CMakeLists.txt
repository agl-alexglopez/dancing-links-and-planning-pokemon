include(FetchContent)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

add_library(dancing_links)
target_sources(dancing_links
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS
      ${PROJECT_SOURCE_DIR}/src
    FILES
      ${PROJECT_SOURCE_DIR}/src/dancing_links.cc
      ${PROJECT_SOURCE_DIR}/src/pokemon_links.cc
      ${PROJECT_SOURCE_DIR}/src/ranked_set.cc
      ${PROJECT_SOURCE_DIR}/src/type_encoding.cc
      ${PROJECT_SOURCE_DIR}/src/map_parser.cc
      ${PROJECT_SOURCE_DIR}/src/pokemon_parser.cc
      ${PROJECT_SOURCE_DIR}/src/resistance.cc
)
target_link_libraries(dancing_links nlohmann_json::nlohmann_json)
