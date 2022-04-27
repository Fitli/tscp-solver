//
// Created by fitli on 27.04.22.
//

#ifndef TSCP_SOLVER_CHANGE_FINDER_H
#define TSCP_SOLVER_CHANGE_FINDER_H

#include "datatypes.h"
#include "heuristics.h"

/*
 * Finds a new trainset path starting and ending in the Station `station`
 * Params:
 *      sol - modified solution
 *      problem - problem of the solution
 *      trainset - type of trainset to be added
 *      station - start and end station of the trainset
 *      edges - output parameter where will be stored an array of edges with the trainset
 *      num_edges - output parameter where will be stored the number of edges with the trainset
 */
void find_train_two_side(Solution *sol, const Problem *problem, const Station *station, int num_conds,
                         EdgeCondition **front_conditions, EdgeCondition **back_conditions,
                         Edge ***edges, int *num_edges) ;

#endif //TSCP_SOLVER_CHANGE_FINDER_H