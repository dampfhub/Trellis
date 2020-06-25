cmake_minimum_required(VERSION 3.10)
project(Trellis)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

find_package(asio REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)

file(GLOB_RECURSE SRC "src/*.cpp")

add_executable(Trellis ${SRC})

target_link_libraries(Trellis glad::glad)
target_link_libraries(Trellis glm)
target_link_libraries(Trellis glfw)
target_link_libraries(Trellis freetype)
target_link_libraries(Trellis asio)

if (MINGW)
    target_link_libraries(Trellis ws2_32)
    target_link_libraries(Trellis wsock32)
    option(MINGW_STDTHREADS_GENERATE_STDHEADERS "" ON)
    add_subdirectory(mingw_stdthreads)
    target_link_libraries(Trellis mingw_stdthreads)
endif ()
if (MSVC)
    target_compile_options(Trellis PRIVATE /W4)
    add_definitions(-D_WIN32_WINNT=0x601)
else ()
    target_compile_options(Trellis PRIVATE -Wall -Wextra -pedantic)
endif ()