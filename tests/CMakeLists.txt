enable_testing()
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP FALSE # CMake may raise a warning to set this. If so, uncomment and set.
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
add_executable(tests tests.cc)
target_link_libraries(tests GTest::gtest_main dancing_links)
# Don't want any warnings from google test compilation.
set_target_properties(gtest PROPERTIES COMPILE_FLAGS "-w")
set_target_properties(gtest_main PROPERTIES COMPILE_FLAGS "-w")
include(GoogleTest)
gtest_discover_tests(tests)
