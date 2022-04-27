//
// Created by fitli on 19.03.22.
//

#ifndef TSCP_SOLVER_C_SOLUTION_MODIFIER_H
#define TSCP_SOLVER_C_SOLUTION_MODIFIER_H

#include "datatypes.h"

void add_train_array(Solution *sol, const Trainset *trainset, const Edge **edges, int num_edges);

void remove_train_array(Solution *sol, const Trainset *trainset, const Edge **edges, int num_edges);

void change_train_array(Solution *sol, const Trainset *old_ts, const Trainset *new_ts, Edge **edges, int num_edges);
#endif //TSCP_SOLVER_C_SOLUTION_MODIFIER_H
