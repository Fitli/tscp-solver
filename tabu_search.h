//
// Created by Ivana Krumlová on 31.10.22.
//

#ifndef TSCP_SOLVER_TABU_SEARCH_H
#define TSCP_SOLVER_TABU_SEARCH_H

#include "datatypes.h"

void tabu_search(Problem *problem, Solution *sol, int taboo_size, int neighborhood_size, int max_iters, clock_t inittime, bool verbose);

#endif //TSCP_SOLVER_TABU_SEARCH_H
