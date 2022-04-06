//
// Created by fitli on 19.03.22.
//

#ifndef TSCP_SOLVER_C_SOLUTION_MODIFIER_H
#define TSCP_SOLVER_C_SOLUTION_MODIFIER_H

#include "datatypes.h"

void add_train_two_side(Solution *sol, const Problem *problem, const Trainset *trainset, const Station *station,
                        void (*heuristic)(const Solution *, const Problem *, const Node *, const Node *, int *, int *),
                        Edge ***edges, int *num_edges);

void first_empty_subcon(const Solution *sol, const Problem *problem, const Node *node_front, const Node *node_back,
                        int *front_move_edge_id, int *back_move_edge_id);

#endif //TSCP_SOLVER_C_SOLUTION_MODIFIER_H
