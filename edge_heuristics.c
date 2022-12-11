//
// Created by fitli on 06.04.22.
//

#include <stdio.h>
#include "edge_heuristics.h"
#include "datatypes.h"


int eval_edge_condition(const EdgeCondition *cond, const Edge *edge, const EdgeSolution *sol){
    int result = 1;
    while(cond != NULL && result) {
        if(cond->predicate)
            result = cond->predicate(edge, sol, cond->a_data);
        cond = cond->conjunction;
    }
    return result;
}

EdgeCondition create_edge_condition(int (*predicate)(const Edge *, const EdgeSolution *, void *), void *a_data, EdgeCondition *conj) {
    EdgeCondition result;
    result.predicate = predicate;
    result.a_data = a_data;
    result.conjunction = conj;
    return result;
}

int edge_is_empty(const Edge *edge, const EdgeSolution *sol, void *a_data) {
    return sol->capacity == 0;
}

int edge_needs_more_capacity(const Edge *edge, const EdgeSolution *sol, void *a_data) {
    int capacity_change = 0;
    if (a_data) {
        capacity_change = *(int *) a_data;
    }
    return sol->capacity + capacity_change < edge->minimal_capacity;
}

int edge_enough_capacity(const Edge *edge, const EdgeSolution *sol, void *a_data) {
    int capacity_change = 0;
    if (a_data) {
        capacity_change = *(int *) a_data;
    }
    return sol->capacity + capacity_change > edge->minimal_capacity;
}

int edge_none(const Edge *edge, const EdgeSolution *sol, void *a_data) {
    return 0;
}

int edge_has_trainset(const Edge *edge, const EdgeSolution *sol, void *a_data) { // a_data is an two-member array containing id of trainset and the minimal amount
    int ts = *(int *) a_data;
    int amount = *((int *) a_data + 1);
    return sol->num_trainsets[ts] >= amount;
}

int edge_ends_in_station(const Edge *edge, const EdgeSolution *sol, void *a_data) {
    int station = *(int *) a_data;
    return edge->end_node->station->id == station;
}

int edge_start_in_station(const Edge *edge, const EdgeSolution *sol, void *a_data) {
    int station = *(int *) a_data;
    return edge->start_node->station->id == station;
}

// Overall number of trainsets is bigger than a_data
int edge_has_more_ts_than(const Edge *edge, const EdgeSolution *sol, void *a_data) {
    int num = *(int *) a_data;
    int n_ts = 0;
    for (int i = 0; i < sizeof(sol->num_trainsets)/sizeof(int*); ++i) {
        n_ts += sol->num_trainsets[i];
    }
    return n_ts > num;
}

// a_data: array of integers - modification of number of seats, capacity treshold
int edge_has_more_seats_than(const Edge *edge, const EdgeSolution *sol, void *a_data) {
    int cap_modif = *(int *) a_data;
    int treshold = *((int *) a_data + 1);
    return sol->capacity + cap_modif > treshold;
}