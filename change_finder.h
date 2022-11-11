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
int find_trip_end_to_end(Solution *sol, const Problem *problem, const Station *station, int num_conds,
                         EdgeCondition **front_conditions, EdgeCondition **back_conditions, EdgeCondition *wait_condition,
                         Edge ***edges, int *num_edges);

int find_trip_between_nodes(Solution *sol, const Problem *problem, const Node *node_front, const Node *node_back, int num_conds,
                            EdgeCondition **front_conditions, EdgeCondition **back_conditions, EdgeCondition *wait_condition,
                            Edge ***edges, int *num_edges);

void select_next_out_edge(const Solution *sol, const Node *node, EdgeCondition *out_e_cond,
                          EdgeCondition *wait_e_cond, int *selected_edge_id);

void select_prev_out_edge(const Solution *sol, const Node *node, EdgeCondition *out_e_cond,
                          EdgeCondition *wait_e_cond, int *selected_edge_id);

void select_next_in_edge(const Solution *sol, const Node *node, EdgeCondition *in_e_cond,
                         EdgeCondition *wait_e_cond, int *selected_edge_id);

void select_prev_in_edge(const Solution *sol, const Node *node, EdgeCondition *in_e_cond,
                         EdgeCondition *wait_e_cond, int *selected_edge_id);

int find_train_containing_edge(Solution *sol, const Problem *problem, const Edge *edge, int num_conds,
                                EdgeCondition **move_conditions, EdgeCondition *wait_condition,Edge ***edges, int *num_edges);

int find_trip_randomized_dfs(Problem * problem, Solution *sol, Node *start_node, Node *end_node,
                             EdgeCondition *wait_condition, EdgeCondition *move_condition, int allow_overnight,
                             int num_prob_conditions, EdgeCondition **prob_conditions, const int *probabilities,
                             Edge ***edges, int *num_edges);

int find_random_trip_from(Problem * problem, Solution *sol, Node *start_node, int trip_len,
                          EdgeCondition *wait_condition, EdgeCondition *move_condition, int allow_overnight,
                          int num_prob_conditions, EdgeCondition **prob_conditions, const int *probabilities,
                          Edge ***edges, int *num_edges);

#endif //TSCP_SOLVER_CHANGE_FINDER_H
