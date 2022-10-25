//
// Created by fitli on 03.04.22.
//

#ifndef TSCP_SOLVER_C_OBJECTIVE_H
#define TSCP_SOLVER_C_OBJECTIVE_H

#include "datatypes.h"

void recalculate_objective(Solution *sol, Problem *problem);
void update_obj_add_ts_to_edge(Solution *sol, const Problem *problem, const Trainset *ts, const Edge *edge);
void update_obj_remove_ts_from_edge(Solution *sol, const Problem *problem, const Trainset *ts, const Edge *edge);
void get_num_ts(Solution *sol, Problem *problem, int *num_ts);

#endif //TSCP_SOLVER_C_OBJECTIVE_H
