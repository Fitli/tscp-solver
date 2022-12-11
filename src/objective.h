//
// Created by Ivana Krumlová on 03.04.22.
//

#ifndef TSCP_SOLVER_C_OBJECTIVE_H
#define TSCP_SOLVER_C_OBJECTIVE_H

#include "datatypes.h"

/**
 * Recalculate objective of a solution form scratch
 * @param sol
 * @param problem
 */
void recalculate_objective(Solution *sol, Problem *problem);
/**
 * Update objective of a solution by adding one trainset to an edge. This function is called BEFORE the trainset is actually added
 * @param sol
 * @param problem
 * @param ts
 * @param edge
 */
void update_obj_add_ts_to_edge(Solution *sol, const Problem *problem, const Trainset *ts, const Edge *edge);
/**
 * Update objective of a solution by removing one trainset to an edge. This function is called BEFORE the trainset is actually removed
 * @param sol
 * @param problem
 * @param ts
 * @param edge
 */
void update_obj_remove_ts_from_edge(Solution *sol, const Problem *problem, const Trainset *ts, const Edge *edge);

#endif //TSCP_SOLVER_C_OBJECTIVE_H
