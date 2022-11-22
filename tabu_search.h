//
// Created by fitli on 31.10.22.
//

#ifndef TSCP_SOLVER_TABU_SEARCH_H
#define TSCP_SOLVER_TABU_SEARCH_H

#include "datatypes.h"

void tabu_search(Problem *problem, Solution *sol, int taboo_size, int neighborhood_size, int stop_no_improve, int max_iters, clock_t inittime, FILE *csv_objective, FILE *csv_operations);
/**
 * Remove rmandomly part of the trainsets
 * @param problem
 * @param solution
 * @param remove_rate form interval [0, 1] - portion of trainsets to remove
 */
void perturbate(Problem *problem, Solution *solution, double remove_rate);

#endif //TSCP_SOLVER_TABU_SEARCH_H
