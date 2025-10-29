# Simpler command line application.
add_executable(pokemon_cli pokemon_cli.cc)
target_link_libraries(pokemon_cli dancing_links)

# Raylib graph visualizer demo.

# Adding Raylib
include(FetchContent)

# Unfortunately raylib does not include source code in their releases and
# only provide a compiled library file and include headers. So we must
# grab whole repo and compile from source.
FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(raylib)
# I don't want to see warnings during compilation about raylib code.
target_compile_options(raylib PRIVATE "-w")

# Raygui is nice in that it is header only so grabbing a release works fine.
# There is no library file for specific platforms to worry about.
FetchContent_Declare(
    raygui
    URL "https://github.com/raysan5/raygui/archive/refs/tags/4.0.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)
FetchContent_MakeAvailable(raygui)
# I don't want to see warnings during compilation about raylib code.
add_library(raygui INTERFACE)
target_include_directories(raygui INTERFACE "${raygui_SOURCE_DIR}/src")
# Even though the header is compiled directly in our code we can ask for no
# raygui related warnings during compilation step.
target_compile_options(raygui INTERFACE "-w")


add_executable(pokemon_gui pokemon_gui.cc)
target_link_libraries(pokemon_gui PRIVATE raylib raygui dancing_links)

