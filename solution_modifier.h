//
// Created by fitli on 19.03.22.
//

#ifndef TSCP_SOLVER_C_SOLUTION_MODIFIER_H
#define TSCP_SOLVER_C_SOLUTION_MODIFIER_H

#include "datatypes.h"

void add_train_array(Solution *sol, const Problem *problem, const Trainset *trainset, Edge **edges, int num_edges);
void add_trainset_to_edge(Solution *sol, const Problem *problem, const Trainset *ts, const Edge *edge);

void remove_train_array(Solution *sol, const Problem *problem, const Trainset *trainset, Edge **edges, int num_edges);
void remove_trainset_from_edge(Solution *sol, const Problem *problem, const Trainset *ts, const Edge *edge);

#endif //TSCP_SOLVER_C_SOLUTION_MODIFIER_H
