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

void change_train_array(Solution *sol, const Problem *problem, const Trainset *old_ts, const Trainset *new_ts, int old_ts_amount, int new_ts_amount, Edge **edges, int num_edges);

void move_to_other_subcon(Solution *sol, const Problem *problem, const Trainset *ts, Edge *old_edge, Edge *new_edge);
#endif //TSCP_SOLVER_C_SOLUTION_MODIFIER_H
