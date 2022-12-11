//
// Created by fitli on 27.04.22.
//

#ifndef TSCP_SOLVER_CHANGE_FINDER_H
#define TSCP_SOLVER_CHANGE_FINDER_H

#include "datatypes.h"
#include "edge_heuristics.h"

/**
 * Finds a sequence of edges connecting `start_node` and `end_node`. Uses DFS with random order of searching branches
 *
 * @param problem
 * @param sol
 * @param start_node
 * @param end_node
 * @param wait_condition must hold for all WAITING edges in the trip
 * @param move_condition must hold for all SUBCONNECTION edges in the trip
 * @param allow_overnight if 1, it is allowed to jump between evening and morning in the same station
 * @param num_prob_conditions length of `prob-conditions` and `probabilities`
 * @param prob_conditions At every node, if `prob_conditions[i]` is met, the probability of inspecting
 * SUBCONNECTION edge before WAITING edge true is `probabilities[i]`
 * @param probabilities At every node, if `prob_conditions[i]` is met, the probability of inspecting
 * SUBCONNECTION edge before WAITING edge true is `probabilities[i]`
 * @param[out] edges dynamically alocated array with edges in the trip.
 * @param[out] num_edges number of edges in `edges`.
 * @return if the resulting trip contains at leadst 1 SOURCE edge, then the number of SOURCE edges in the result.
 *         Otherwise 1 if a trip was found and 0 if not.
 */
int find_trip_randomized_dfs(Problem * problem, Solution *sol, Node *start_node, Node *end_node,
                             EdgeCondition *wait_condition, EdgeCondition *move_condition, int allow_overnight,
                             int num_prob_conditions, EdgeCondition *prob_conditions, const double *probabilities,
                             Edge ***edges, int *num_edges);

/**
 * Finds a random sequence of maximally `trip_len` edges starting at `start_node`. Uses DFS with random order of searching branches.
 *
 * @param problem
 * @param sol
 * @param start_node
 * @param trip_len length of the trip
 * @param wait_condition must hold for all WAITING edges in the trip
 * @param move_condition must hold for all SUBCONNECTION edges in the trip
 * @param allow_overnight if 1, it is allowed to jump between evening and morning in the same station
 * @param num_prob_conditions length of `prob-conditions` and `probabilities`
 * @param prob_conditions At every node, if `prob_conditions[i]` is met, the probability of inspecting
 * SUBCONNECTION edge before WAITING edge true is `probabilities[i]`
 * @param probabilities At every node, if `prob_conditions[i]` is met, the probability of inspecting
 * SUBCONNECTION edge before WAITING edge true is `probabilities[i]`
 * @param[out] edges dynamically allocated array with edges in the trip.
 * @param[out] num_edges number of edges in `edges`.
 * @return if the resulting trip contains at leadst 1 SOURCE edge, then the number of SOURCE edges in the result.
 *         Otherwise 1 if a trip was found and 0 if not.
 */
int find_random_trip_from(Problem * problem, Solution *sol, Node *start_node, int trip_len,
                          EdgeCondition *wait_condition, EdgeCondition *move_condition, int allow_overnight,
                          int num_prob_conditions, EdgeCondition *prob_conditions, const double *probabilities,
                          Edge ***edges, int *num_edges);

/**
 * Finds a sequence of waiting nodes connecting `start_node` and `end_node`
 * @param sol
 * @param problem
 * @param start_node
 * @param end_node
 * @param condition must hold for all WAITING edges in the trip
 * @param edges dynamically allocated array with edges in the trip
 * @param num_edges Number of edges in `edges`
 * @return Number of edges if a path is found, 0 otherwise.
 */
int find_waiting_between_nodes(Solution *sol, const Problem *problem, const Node *start_node, const Node *end_node,
                               EdgeCondition *condition, Edge ***edges, int *num_edges);

#endif //TSCP_SOLVER_CHANGE_FINDER_H
