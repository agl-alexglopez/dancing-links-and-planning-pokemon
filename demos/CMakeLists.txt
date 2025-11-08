# Simpler command line application.
add_executable(pokemon_cli pokemon_cli.cc)
target_link_libraries(pokemon_cli dancing_links)

# Dependencies
set(RAYLIB_VERSION 5.5)
include(FetchContent)
FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
)
FetchContent_GetProperties(raylib)
if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(raylib)
endif()
set(RAYGUI_VERSION 4.0)
FetchContent_Declare(
    raygui
    URL "https://github.com/raysan5/raygui/archive/refs/tags/${RAYGUI_VERSION}.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)
FetchContent_GetProperties(raygui)
if (NOT raygui_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(raygui)
endif()
# I don't want to see warnings during compilation about raylib code.
target_compile_options(raylib PRIVATE "-w")

# I don't want to see warnings during compilation about raylib code.
add_library(raygui INTERFACE)
target_include_directories(raygui INTERFACE "${raygui_SOURCE_DIR}/src")
# Even though the header is compiled directly in our code we can ask for no
# raygui related warnings during compilation step.
target_compile_options(raygui INTERFACE "-w")


add_executable(pokemon_gui pokemon_gui.cc)
target_link_libraries(pokemon_gui PRIVATE raylib raygui dancing_links)

