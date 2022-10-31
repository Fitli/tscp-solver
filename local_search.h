//
// Created by fitli on 31.10.22.
//

#ifndef TSCP_SOLVER_LOCAL_SEARCH_H
#define TSCP_SOLVER_LOCAL_SEARCH_H

#include "datatypes.h"

void local_search(Problem *problem, Solution *sol, int taboo_size, int neighborhood_size, int stop_no_improve);
void perturbate(Problem *problem, Solution *solution, double remove_rate);

#endif //TSCP_SOLVER_LOCAL_SEARCH_H
