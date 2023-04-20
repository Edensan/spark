cmake_minimum_required(VERSION 3.5.1)
project(spark)

set(CMAKE_CXX_STANDARD 14)

include_directories(common/includes)
include_directories(game/includes)
set(SOURCE_COMMON_TESTS
  ${CMAKE_SOURCE_DIR}/common/tests/TestMain.cpp
  ${CMAKE_SOURCE_DIR}/common/tests/MathTypesTest.cpp
  ${CMAKE_SOURCE_DIR}/common/tests/ValueTypesTest.cpp)
set(SOURCE_GAME
  ${CMAKE_SOURCE_DIR}/game/src/GameGrid.cpp
  ${CMAKE_SOURCE_DIR}/game/src/GameObject.cpp
  ${CMAKE_SOURCE_DIR}/game/src/GameStats.cpp
  ${CMAKE_SOURCE_DIR}/game/src/objects/Character.cpp)
set(SOURCE_GAME_TESTS
  ${CMAKE_SOURCE_DIR}/game/tests/GameGridTest.cpp
  ${CMAKE_SOURCE_DIR}/game/tests/CharacterTest.cpp)

# Setup googletest for compilation
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(googletest)

include_directories(googletest/googletest/include)

add_executable(
  spark
  ${SOURCE_COMMON_TESTS}
  ${SOURCE_GAME}
  ${SOURCE_GAME_TESTS})

target_link_libraries(
  spark
  PUBLIC
  gtest)
