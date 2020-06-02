cmake_minimum_required(VERSION 3.10)
project(GridScape)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

find_package(glm REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)

file(GLOB_RECURSE SRC "src/*.cpp")

add_executable(GridScape ${SRC})

target_link_libraries(GridScape glad::glad)
target_link_libraries(GridScape glm)
target_link_libraries(GridScape glfw)
target_link_libraries(GridScape freetype)

if (MSVC)
    target_compile_options(GridScape PRIVATE /W4)
else ()
    target_compile_options(GridScape PRIVATE -Wall -Wextra -pedantic)
endif ()