enable_testing()
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
add_executable(dlxtests tests.cc)
target_link_libraries(dlxtests GTest::gtest_main point type_encoding ranked_set pokemon_links map_parser pokemon_parser)
include(GoogleTest)
gtest_discover_tests(dlxtests)
