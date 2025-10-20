# Simpler command line application.
add_executable(pokemon_cli pokemon_cli.cc)
target_link_libraries(pokemon_cli dancing_links)

# Raylib graph visualizer demo.

# Adding Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)
# Don't what to see any warnings from raylib.
set_target_properties(raylib PROPERTIES COMPILE_FLAGS "-w")
add_executable(pokemon_gui pokemon_gui.cc)
target_link_libraries(pokemon_gui raylib dancing_links)

