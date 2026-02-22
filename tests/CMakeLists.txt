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
target_compile_options(gtest PRIVATE "-w")
target_compile_options(gtest_main PRIVATE "-w")
get_target_property(gtest_SOURCE_DIR gtest SOURCE_DIR)
get_target_property(gtest_main_SOURCE_DIR gtest_main SOURCE_DIR)
add_executable(tests tests.cc)
# We don't want compiler or tidy warnings from these libraries.
target_include_directories(tests 
    SYSTEM 
    PRIVATE 
        ${gtest_SOURCE_DIR}
        ${gtest_main_SOURCE_DIR}
)
target_link_libraries(tests GTest::gtest_main dancing_links)
# Don't want any warnings from google test compilation.
include(GoogleTest)
gtest_discover_tests(tests)
