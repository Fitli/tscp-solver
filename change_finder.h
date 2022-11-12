//
// Created by fitli on 27.04.22.
//

#ifndef TSCP_SOLVER_CHANGE_FINDER_H
#define TSCP_SOLVER_CHANGE_FINDER_H

#include "datatypes.h"
#include "heuristics.h"

int find_trip_randomized_dfs(Problem * problem, Solution *sol, Node *start_node, Node *end_node,
                             EdgeCondition *wait_condition, EdgeCondition *move_condition, int allow_overnight,
                             int num_prob_conditions, EdgeCondition **prob_conditions, const int *probabilities,
                             Edge ***edges, int *num_edges);

int find_random_trip_from(Problem * problem, Solution *sol, Node *start_node, int trip_len,
                          EdgeCondition *wait_condition, EdgeCondition *move_condition, int allow_overnight,
                          int num_prob_conditions, EdgeCondition **prob_conditions, const int *probabilities,
                          Edge ***edges, int *num_edges);

int find_waiting_between_nodes(Solution *sol, const Problem *problem, const Node *node_front, const Node *node_back,
                               EdgeCondition *condition, Edge ***edges, int *num_edges);

#endif //TSCP_SOLVER_CHANGE_FINDER_H
