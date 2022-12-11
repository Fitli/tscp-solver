//
// Created by Ivana Krumlová on 29.04.22.
//

#include <stdio.h>
#include "datatypes.h"

#ifndef TSCP_SOLVER_ACTIONS_H
#define TSCP_SOLVER_ACTIONS_H

#define NUM_OPERATIONS 8

/**
 * Add trainset beginning and ending in a station.
 * @param sol
 * @param problem
 * @param station_id Train will start and end here
 * @param ts_id
 */
void oper_add_train_dfs(Solution *sol, Problem *problem, int station_id, int ts_id);

/**
 * Add trainset to a trip containing edge
 * @param sol
 * @param problem
 * @param edge_id this edge will be contained in the resulting trip
 * @param ts_id
 */
void oper_add_train_with_edge_dfs(Solution *sol, Problem *problem, int edge_id, int ts_id);

/**
 * Change trainsets on a trip beginning and ending in a station.
 * @param sol
 * @param problem
 * @param station_id Train will start and end here
 * @param old_ts_id
 * @param new_ts_id
 * @param old_ts_amount
 * @param new_ts_amount
 */
void oper_change_train_capacity_dfs(Solution *sol, Problem *problem, int station_id, int old_ts_id, int new_ts_id, int old_ts_amount, int new_ts_amount);
/**
 * Change trainsets on a trip beginning containing edge.
 * @param sol
 * @param problem
 * @param edge_id this edge will be contained in the resulting trip
 * @param old_ts_id
 * @param new_ts_id
 * @param old_ts_amount
 * @param new_ts_amount
 * @return
 */
int oper_change_train_capacity(Solution *sol, Problem *problem, int edge_id, int old_ts_id, int new_ts_id, int old_ts_amount, int new_ts_amount);
/**
 * Remove trainset from a trip beginning and ending in a station.
 * @param sol
 * @param problem
 * @param station_id Train will start and end here
 * @param ts_id
 * @return
 */
int oper_remove_train_dfs(Solution *sol, Problem *problem, int station_id, int ts_id);
/**
 * Remove trainsets waiting whole period in a station
 * @param sol
 * @param problem
 * @param station_id
 * @param ts_id
 * @return
 */
int oper_remove_waiting_train(Solution *sol, Problem *problem, int station_id, int ts_id);
/**
 * Remove trainset from a trip containing an edge.
 * @param sol
 * @param problem
 * @param edge_id
 * @param ts_id
 * @return
 */
int oper_remove_train_with_edge_dfs(Solution *sol, Problem *problem, int edge_id, int ts_id);
/**
 * Selects random operation
 * @param problem
 * @param sol
 * @param weights for selectiong individual operations
 * @return
 */
int select_operation(Problem *problem, Solution *sol, int *weights);

#endif //TSCP_SOLVER_ACTIONS_H
