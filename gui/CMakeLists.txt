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

add_library(gui)
target_sources(gui
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS
      ${PROJECT_SOURCE_DIR}/gui
    FILES
      ${PROJECT_SOURCE_DIR}/gui/gui.cc
)
target_link_libraries(gui raylib)
