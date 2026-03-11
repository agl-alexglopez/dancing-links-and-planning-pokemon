include(FetchContent)

FetchContent_Declare(
    json 
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    URL_HASH SHA256=42f6e95cad6ec532fd372391373363b62a14af6d771056dbfc86160e6dfff7aa
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)
FetchContent_MakeAvailable(json)

# Don't want clang tidy parsing this library
get_target_property(json_INCLUDES
    nlohmann_json::nlohmann_json
    INTERFACE_INCLUDE_DIRECTORIES
)

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
      ${PROJECT_SOURCE_DIR}/src/pokemon_parser.cc
      ${PROJECT_SOURCE_DIR}/src/resistance.cc
)

# Treat json as system library so clang tidy ignores.
target_include_directories(dancing_links
    SYSTEM
    PRIVATE
        ${json_INCLUDES}
)

target_link_libraries(dancing_links 
    PRIVATE 
        nlohmann_json::nlohmann_json
)
target_compile_features(dancing_links PUBLIC cxx_std_20)
