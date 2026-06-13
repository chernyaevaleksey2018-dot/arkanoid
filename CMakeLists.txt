cmake_minimum_required(VERSION 3.30.4)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_NAME ARKANOID)

project(${PROJECT_NAME} LANGUAGES C CXX)

# Adding SFML
FetchContent_Declare(
  sfml 
  GIT_REPOSITORY "https://github.com/SFML/SFML.git"
  GIT_TAG "3.1.0"
)
FetchContent_MakeAvailable(sfml)

add_executable(${PROJECT_NAME} src/main.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC src/public)

target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics sfml-system sfml-window)
