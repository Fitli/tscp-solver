//
// Created by fitli on 31.10.22.
//

#include "datatypes.h"

#ifndef TSCP_SOLVER_SIMMULATED_ANNEALING_H
#define TSCP_SOLVER_SIMMULATED_ANNEALING_H

enum TempDecrease {
    LINEAR,
    GEOMETRIC
};

double init_temp(Problem *problem, Solution *solution, int neigh_size, double avg_accept_prob);
void simulated_annealing(Problem *problem, Solution *sol, double init_temp, double temp_decrease, int max_iter,
                         clock_t inittime, enum TempDecrease temp_decrease_type, bool verbose);

#endif //TSCP_SOLVER_SIMMULATED_ANNEALING_H
