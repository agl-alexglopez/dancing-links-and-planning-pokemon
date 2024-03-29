add_library(point point.hh point.cc)
add_library(util INTERFACE util.hh)

FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
)
FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

if (WIN32)
    set(GLEW_LIB_DEBUG "${PROJECT_SOURCE_DIR}/gui/glew-2.1.0/lib/Debug/x64/glew32sd.lib")
    set(GLEW_LIB_RELEASE "${PROJECT_SOURCE_DIR}/gui/glew-2.1.0/lib/Release/x64/glew32s.lib")
    add_library(window window.hh window.cc)
    target_compile_definitions(window PRIVATE GLEW_STATIC)
    target_link_libraries(window glfw OpenGL::GL $<$<CONFIG:Debug>:${GLEW_LIB_DEBUG}> $<$<CONFIG:Release>:${GLEW_LIB_RELEASE}>)

    add_library(vertex vertex.hh vertex.cc)
    target_compile_definitions(vertex PRIVATE GLEW_STATIC)
    target_link_libraries(vertex OpenGL::GL $<$<CONFIG:Debug>:${GLEW_LIB_DEBUG}> $<$<CONFIG:Release>:${GLEW_LIB_RELEASE}>)

    add_library(shader shader.hh shader.cc)
    target_compile_definitions(shader PRIVATE GLEW_STATIC)
    target_link_libraries(shader OpenGL::GL $<$<CONFIG:Debug>:${GLEW_LIB_DEBUG}> $<$<CONFIG:Release>:${GLEW_LIB_RELEASE}> util)
else()
    find_package(GLEW REQUIRED)
    add_library(window window.hh window.cc)
    target_link_libraries(window glfw OpenGL::GL GLEW::GLEW)

    add_library(vertex vertex.hh vertex.cc)
    target_link_libraries(vertex OpenGL::GL GLEW::GLEW)

    add_library(shader shader.hh shader.cc)
    target_link_libraries(shader OpenGL::GL GLEW::GLEW util)
endif()


add_library(triangle triangle.hh triangle.cc)
target_link_libraries(triangle PRIVATE util shader vertex)
add_library(quad quad.hh quad.cc)
target_link_libraries(quad PRIVATE util shader vertex)

