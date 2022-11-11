//
// Created by fitli on 06.04.22.
//

#ifndef TSCP_SOLVER_HEURISTICS_H
#define TSCP_SOLVER_HEURISTICS_H

#include "datatypes.h"

int select_station_max_empty_subcons(Problem *problem, Solution *solution);
int select_station_first_empty_departure(Problem *problem, Solution *solution);
int select_station_last_empty_arrival(Problem *problem, Solution *solution);
int select_station_random(Problem *problem, Solution *solution);

typedef struct EdgeCondition EdgeCondition;
struct EdgeCondition{
    int (*predicate)(const Edge *, const EdgeSolution *, void *);
    void *a_data;
    EdgeCondition *conjunction;
};
int eval_edge_condition(const EdgeCondition *cond, const Edge *edge, const EdgeSolution *sol);
EdgeCondition *create_edge_condition(int (*predicate)(const Edge *, const EdgeSolution *, void *), void *a_data, EdgeCondition *conj);
void free_edge_conditions(EdgeCondition *cond);

int edge_is_empty(const Edge *edge, const EdgeSolution *sol, void *a_data);
int edge_needs_more_capacity(const Edge *edge, const EdgeSolution *sol, void *a_data);
int edge_enough_capacity(const Edge *edge, const EdgeSolution *sol, void *a_data);
int edge_any(const Edge *edge, const EdgeSolution *sol, void *a_data);
int edge_none(const Edge *edge, const EdgeSolution *sol, void *a_data);
/*
 * True if there is at least 1 trainset of given type on this edge. a_data is the trainset tupe.
 */
int edge_has_trainset(const Edge *edge, const EdgeSolution *sol, void *a_data);
int edge_ends_in_station(const Edge *edge, const EdgeSolution *sol, void *a_data);
int edge_start_in_station(const Edge *edge, const EdgeSolution *sol, void *a_data);
int edge_has_more_ts_than(const Edge *edge, const EdgeSolution *sol, void *a_data);
int edge_has_more_seats_than(const Edge *edge, const EdgeSolution *sol, void *a_data);

#endif //TSCP_SOLVER_HEURISTICS_H
