//
// Created by fitli on 29.04.22.
//

#include "datatypes.h"

#ifndef TSCP_SOLVER_ACTIONS_H
#define TSCP_SOLVER_ACTIONS_H

void oper_add_train_to_empty(Solution *sol, Problem *problem, int station_id);
void oper_add_train_later(Solution *sol, Problem *problem, int station_id, int ts_id);
void oper_add_train_pair_later(Solution *sol, Problem *problem, int station1_id, int station2_id, int ts_id);
void oper_add_train_with_edge(Solution *sol, Problem *problem, int edge_id, int ts_id);
void oper_change_train_capacity(Solution *sol, Problem *problem, int station_id, int old_ts_id, int new_ts_id, int old_ts_amount, int new_ts_amount);
void oper_change_train_pair_capacity(Solution *sol, Problem *problem, int station1_id, int station2_id, int old_ts_id, int new_ts_id, int old_ts_amount, int new_ts_amount);
void oper_remove_train(Solution *sol, Problem *problem, int station_id, int ts_id);
void oper_remove_train_pair(Solution *sol, Problem *problem, int station1_id, int station2_id, int ts_id);
void oper_remove_train_with_edge(Solution *sol, Problem *problem, int edge_id, int ts_id);
void oper_remove_waiting_train(Solution *sol, Problem *problem, int station_id, int ts_id);
void oper_reschedule_w_l(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id);
void oper_reschedule_n_l(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id);
void oper_reschedule_n_w(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id);
void oper_move_edge_back(Solution *sol, Problem *problem, int edge_id, int ts_id);
void oper_move_edge_front(Solution *sol, Problem *problem, int edge_id, int ts_id);


#endif //TSCP_SOLVER_ACTIONS_H
