//
// Created by fitli on 19.03.22.
//

#ifndef TSCP_SOLVER_C_SOLUTION_MODIFIER_H
#define TSCP_SOLVER_C_SOLUTION_MODIFIER_H

#include "datatypes.h"

/*
 * Adds to the Solution `sol` a new trainset starting and ending in the Station `station`
 * Params:
 *      sol - modified solution
 *      problem - problem of the solution
 *      trainset - type of trainset to be added
 *      station - start and end station of the trainset
 *      edges - output parameter where will be stored an array of edges with the trainset
 *      num_edges - output parameter where will be stored the number of edges with the trainset
 */
void find_train_two_side(Solution *sol, const Problem *problem, const Trainset *trainset, const Station *station,
                         Edge ***edges, int *num_edges);

void add_train_array(Solution *sol, const Trainset *trainset, const Edge **edges, int num_edges);

void remove_train_array(Solution *sol, const Trainset *trainset, const Edge **edges, int num_edges);

void change_train_array(Solution *sol, const Trainset *old_ts, const Trainset *new_ts, Edge **edges, int num_edges);
#endif //TSCP_SOLVER_C_SOLUTION_MODIFIER_H
