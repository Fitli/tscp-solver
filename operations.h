//
// Created by fitli on 29.04.22.
//

#include <stdio.h>
#include "datatypes.h"

#ifndef TSCP_SOLVER_ACTIONS_H
#define TSCP_SOLVER_ACTIONS_H

#define NUM_OPERATIONS 8

void oper_add_train_dfs(Solution *sol, Problem *problem, int station_id, int ts_id);
void oper_add_train_with_edge_dfs(Solution *sol, Problem *problem, int edge_id, int ts_id);
void oper_change_train_capacity_dfs(Solution *sol, Problem *problem, int station_id, int old_ts_id, int new_ts_id, int old_ts_amount, int new_ts_amount);
int oper_change_train_capacity_dfs_with_edge(Solution *sol, Problem *problem, int old_ts_id, int new_ts_id, int old_ts_amount, int new_ts_amount, int edge_id);
int oper_remove_train_dfs(Solution *sol, Problem *problem, int station_id, int ts_id);
int oper_remove_waiting_train(Solution *sol, Problem *problem, int station_id, int ts_id);
int oper_remove_train_with_edge_dfs(Solution *sol, Problem *problem, int edge_id, int ts_id);
int select_operation(Problem *problem, Solution *sol, int *weights);


#endif //TSCP_SOLVER_ACTIONS_H
