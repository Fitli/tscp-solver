cmake_minimum_required(VERSION 3.17)
project(tscp_solver C)

set(CMAKE_C_STANDARD 99)

if(CMAKE_HOST_WIN32)
    set(libname "libconfig")
else()
    set(libname "config")
endif()

add_executable(tscp_solver main.c datatypes.h dot_printer.c dot_printer.h parse_config.c parse_config.h datatypes.c solution_modifier.c solution_modifier.h objective.c objective.h heuristics.c heuristics.h)

target_link_libraries(tscp_solver ${libname} )