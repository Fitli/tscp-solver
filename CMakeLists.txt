cmake_minimum_required(VERSION 3.17)
project(tscp_solver C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-O3")

if(CMAKE_HOST_WIN32)
    set(libname "libconfig")
else()
    set(libname "config")
endif()

add_executable(tscp_solver main.c datatypes.h dot_printer.c dot_printer.h parse_input.c parse_input.h datatypes.c solution_modifier.c solution_modifier.h objective.c objective.h heuristics.c heuristics.h change_finder.c change_finder.h test.c test.h operations.c operations.h random.c random.h output.c output.h tabu_search.c tabu_search.h simmulated_annealing.c simmulated_annealing.h constructive_alg.c constructive_alg.h min_flow.c min_flow.h experiments.c experiments.h)

target_link_libraries(tscp_solver ${libname} m)