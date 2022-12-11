cmake_minimum_required(VERSION 3.6)

project(tscp_solver C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-O3")

if(CMAKE_HOST_WIN32)
    set(libname "libconfig")
else()
    set(libname "config")
endif()

add_executable(tscp_solver src/main.c src/datatypes.h src/dot_printer.c src/dot_printer.h src/parse_input.c src/parse_input.h src/datatypes.c src/solution_modifier.c src/solution_modifier.h src/objective.c src/objective.h src/edge_heuristics.c src/edge_heuristics.h src/change_finder.c src/change_finder.h src/test.c src/test.h src/operations.c src/operations.h src/random.c src/random.h src/output.c src/output.h src/tabu_search.c src/tabu_search.h src/simulated_annealing.c src/simulated_annealing.h src/min_flow.c src/min_flow.h)

find_library(TEST_LIBRARY ${libname})
if(NOT TEST_LIBRARY)
    message(FATAL_ERROR "libconfig not found")
endif()
target_link_libraries(tscp_solver ${libname} m)
