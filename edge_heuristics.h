//
// Created by fitli on 06.04.22.
//

#ifndef TSCP_SOLVER_EDGE_HEURISTICS_H
#define TSCP_SOLVER_EDGE_HEURISTICS_H

#include "datatypes.h"

typedef struct EdgeCondition EdgeCondition;
/**
 * EdgeCondition is true for edge e iff `predicate` holds for e, it's solution and a_data AND `conjunction` is true for e.
 */
struct EdgeCondition{
    int (*predicate)(const Edge *, const EdgeSolution *, void *); // predicate of the condition
    void *a_data; // additional data provided to the predicate
    EdgeCondition *conjunction; // another condition conjuncted with this one
};
/**
 * Evaluates `cond` on `edge`
 * @param cond
 * @param edge
 * @param sol EdgeSolution corresponding to `edge`
 * @return
 */
int eval_edge_condition(const EdgeCondition *cond, const Edge *edge, const EdgeSolution *sol);
/**
 * Creates new Edge condition
 * EdgeCondition is true for edge e iff `predicate` holds for e, it's solution and a_data AND `conjunction` is true for e.
 *
 * @param predicate predicate of the condition
 * @param a_data additional data provided to the predicate
 * @param conj another condition conjuncted with this one. Can be NULL.
 * @return
 */
EdgeCondition create_edge_condition(int (*predicate)(const Edge *, const EdgeSolution *, void *), void *a_data, EdgeCondition *conj);

//EDGE PREDICATES
// all predicates has the same format in order to be able to use them as general function pointers. the a_data argument is not used in some cases.
/**
 * 1 iff edge has no trainsets
 * @param edge
 * @param sol
 * @param a_data not used
 * @return
 */
int edge_is_empty(const Edge *edge, const EdgeSolution *sol, void *a_data);

/**
 * 1 iff the capacity, updated by some amount of seats, is less than the minimal capacity of the edge
 * @param edge
 * @param sol
 * @param a_data Pointer to int - the capacity change. NULL means 0 capacity change.
 * @return
 */

int edge_needs_more_capacity(const Edge *edge, const EdgeSolution *sol, void *a_data);
/**
 * 1 iff the capacity, updated by some amount of seats, is greater than the minimal capacity of the edge
 * @param edge
 * @param sol
 * @param a_data Pointer to int - the capacity change. NULL means 0 capacity change.
 * @return
 */
int edge_enough_capacity(const Edge *edge, const EdgeSolution *sol, void *a_data);

/**
 * always 0
 * @param edge
 * @param sol
 * @param a_data not used
 * @return
 */
int edge_none(const Edge *edge, const EdgeSolution *sol, void *a_data);

/**
 * 1 iff there are at least given number of trainsets of given type on this edge.
 * @param edge
 * @param sol
 * @param a_data Array of two ints - First element is id of the trainset, second required amount
 * @return
 */
int edge_has_trainset(const Edge *edge, const EdgeSolution *sol, void *a_data);

/**
 * 1 iff edge ends in specified station
 * @param edge
 * @param sol
 * @param a_data Pointer to int - id of the station
 * @return
 */
int edge_ends_in_station(const Edge *edge, const EdgeSolution *sol, void *a_data);

/**
 * 1 iff edge starts in specified station
 * @param edge
 * @param sol
 * @param a_data Pointer to int - id of the station
 * @return
 */
int edge_start_in_station(const Edge *edge, const EdgeSolution *sol, void *a_data);

/**
 * 1 iff there are at least given number of trainsets of any type on this edge.
 * @param edge
 * @param sol
 * @param a_data Pointer to int - id of the trainset type
 * @return
 */
int edge_has_more_ts_than(const Edge *edge, const EdgeSolution *sol, void *a_data);

/**
 * 1 iff the capacity, updated by some amount of seats, is greater than given number
 * @param edge
 * @param sol
 * @param a_data Array of 2 ints - first is the capacity change, second the required capacity
 * @return
 */
int edge_has_more_seats_than(const Edge *edge, const EdgeSolution *sol, void *a_data);

#endif //TSCP_SOLVER_EDGE_HEURISTICS_H
