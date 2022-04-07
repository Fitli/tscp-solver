//
// Created by fitli on 19.03.22.
//

#ifndef TSCP_SOLVER_C_SOLUTION_MODIFIER_H
#define TSCP_SOLVER_C_SOLUTION_MODIFIER_H

#include "datatypes.h"

void add_train_two_side(Solution *sol, const Problem *problem, const Trainset *trainset, const Station *station,
                        Edge ***edges, int *num_edges);

#endif //TSCP_SOLVER_C_SOLUTION_MODIFIER_H
