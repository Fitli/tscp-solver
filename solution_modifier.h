//
// Created by fitli on 19.03.22.
//

#ifndef TSCP_SOLVER_C_SOLUTION_MODIFIER_H
#define TSCP_SOLVER_C_SOLUTION_MODIFIER_H

#include "datatypes.h"

/**
 * Adds trainset to a set of edges
 * @param sol
 * @param problem
 * @param trainset trainset type
 * @param edges array of edges
 * @param num_edges number of edges
 */
void add_train_array(Solution *sol, const Problem *problem, const Trainset *trainset, Edge **edges, int num_edges);
/**
 * Adds trainset to individual edge
 * @param sol
 * @param problem
 * @param ts
 * @param edge
 */
void add_trainset_to_edge(Solution *sol, const Problem *problem, const Trainset *ts, const Edge *edge);

/**
 * Removes trainset from a set of edges
 * @param sol
 * @param problem
 * @param trainset trainset type
 * @param edges array of edges
 * @param num_edges number of edges
 */
void remove_train_array(Solution *sol, const Problem *problem, const Trainset *trainset, Edge **edges, int num_edges);
/**
 * Removes a trainset from individual edge
 * @param sol
 * @param problem
 * @param ts
 * @param edge
 */
void remove_trainset_from_edge(Solution *sol, const Problem *problem, const Trainset *ts, const Edge *edge);

#endif //TSCP_SOLVER_C_SOLUTION_MODIFIER_H
