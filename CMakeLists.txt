cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_COLOR_DIAGNOSTICS ON)

project(Ui)

find_package(gfx REQUIRED)
add_executable(ui main.cc)

target_compile_options(ui PRIVATE -Wall -Wextra -O0 -ggdb)

set(ASAN ON)
if (ASAN)
    target_compile_options(ui PRIVATE -fsanitize=address,undefined)
    target_link_options(ui PRIVATE -fsanitize=address,undefined)
endif()

target_link_libraries(ui PRIVATE gfx)
