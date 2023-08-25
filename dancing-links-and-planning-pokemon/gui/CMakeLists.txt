add_library(point point.hh point.cc)

FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
)
FetchContent_MakeAvailable(glfw)

add_library(window window.hh window.cc)
target_link_libraries(window glfw)
