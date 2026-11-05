cmake_minimum_required(VERSION 3.21)
project(sdl_rpg VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find the Homebrew SDL3 package
find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_image CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)


add_executable(${PROJECT_NAME}
  src/main.c
  # add additional c files below
  src/tiled_map.c
)

# Tell the compiler to look inside include/ for header files
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  ${PROJECT_NAME} PRIVATE
  SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf cjson
)

# global variables
target_compile_definitions(${PROJECT_NAME} PRIVATE
  RPG_ASSETS="${CMAKE_CURRENT_SOURCE_DIR}/assets"
  RPG_MAPS="${CMAKE_CURRENT_SOURCE_DIR}/maps"
)

foreach(target ${PROJECT_NAME})
  target_compile_options(
    ${target} PRIVATE
    -Wall -Wextra -Wpedantic -Werror -Wshadow -Wstrict-prototypes
  )
endforeach()

# tests
include(CTest)

if(BUILD_TESTING)
  add_executable(first_map_tests tests/first_map_tests.c)
  target_compile_options(first_map_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
  target_compile_definitions(first_map_tests PRIVATE FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/maps")

  # Link cJSON for test executable
  target_link_libraries(first_map_tests PRIVATE cjson)

  add_test(NAME first_map COMMAND first_map_tests)
endif()

# third_party
add_library(cjson STATIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cjson/cJSON.c)
target_include_directories(
  cjson PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cjson
)


