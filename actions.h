//
// Created by fitli on 29.04.22.
//

#include "datatypes.h"

#ifndef TSCP_SOLVER_ACTIONS_H
#define TSCP_SOLVER_ACTIONS_H

void act_add_train_to_empty(Solution *sol, Problem *problem, int station_id);
void act_add_train_later(Solution *sol, Problem *problem, int station_id, int ts_id);
void act_change_train_capacity(Solution *sol, Problem *problem, int station_id, int old_ts_id, int new_ts_id, int old_ts_amount, int new_ts_amount);
void act_remove_train(Solution *sol, Problem *problem, int station_id, int ts_id);
void act_remove_waiting_train(Solution *sol, Problem *problem, int station_id, int ts_id);
void act_reschedule_w_l(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id);
void act_reschedule_n_l(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id);
void act_reschedule_n_w(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id);
void act_move_edge_back(Solution *sol, Problem *problem, int edge_id, int ts_id);
void act_move_edge_front(Solution *sol, Problem *problem, int edge_id, int ts_id);


#endif //TSCP_SOLVER_ACTIONS_H
