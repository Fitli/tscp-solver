//
// Created by fitli on 06.04.22.
//

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "heuristics.h"
#include "datatypes.h"

/*
 * STATIONS
 */
int count_empty_subcons_station(Problem *problem, Station *station, Solution *sol) {
    int result = 0;
    Node *node = station->source_edge->end_node;
    while (1){
        EdgeSolution edge_sol;
        if (node->in_subcon != NULL) {
            edge_sol = sol->edge_solution[node->in_subcon->id];
        } else {
            edge_sol = sol->edge_solution[node->out_subcon->id];
        }

        if (edge_sol.capacity == 0) {
            result++;
        }
        if (node->out_waiting->type == SINK_EDGE)
            break;
        node = node->out_waiting->end_node;
    }
    return result;
}

/*
 * Returns index of station with biggest number of incoming + outgoing subconnections with no trainset
 */
int select_station_max_empty_subcons(Problem *problem, Solution *solution) {
    int result = 0;
    int max_empty = count_empty_subcons_station(problem, &problem->stations[0], solution);
    printf("%d: %d\n", result, max_empty);
    for(int i = 1; i<problem->num_stations; i++) {
        int num_empty = count_empty_subcons_station(problem, &problem->stations[i], solution);
        printf("%d: %d\n", i, num_empty);
        if(num_empty > max_empty) {
            result = i;
            max_empty = num_empty;
        }
    }
    return result;
}

int select_station_first_empty_departure(Problem *problem, Solution *solution) {
    Node *station_nodes[problem->num_stations];
    for(int i = 0; i<problem->num_stations; i++) {
        station_nodes[i] = problem->stations[i].source_edge->end_node;
    }
    bool all_null = false;
    while(!all_null) {
        all_null = true;
        for(int i = 0; i<problem->num_stations; i++) {
            if(station_nodes[i] == NULL) {
                continue;
            }
            Edge *out_subcon = station_nodes[i]->out_subcon;
            if(out_subcon && solution->edge_solution[out_subcon->id].capacity == 0) {
                return i;
            }
            station_nodes[i] = station_nodes[i]->out_waiting->end_node;
            all_null = false;
        }
    }
    return -1;
}

int select_station_last_empty_arrival(Problem *problem, Solution *solution) {
    Node *station_nodes[problem->num_stations];
    for(int i = 0; i<problem->num_stations; i++) {
        station_nodes[i] = problem->stations[i].sink_edge->start_node;
    }
    bool all_null = false;
    while(!all_null) {
        all_null = true;
        for(int i = 0; i<problem->num_stations; i++) {
            if(station_nodes[i] == NULL) {
                continue;
            }
            Edge *in_subcon = station_nodes[i]->in_subcon;
            if(in_subcon && solution->edge_solution[in_subcon->id].capacity == 0) {
                return i;
            }
            station_nodes[i] = station_nodes[i]->in_waiting->start_node;
            all_null = false;
        }
    }
    return -1;
}

int select_station_random(Problem *problem, Solution *solution) {
    return ((int) random())%problem->num_stations;
}

/*
 * EDGES
 */
int eval_edge_condition(struct EdgeCondition *cond, Edge *edge, EdgeSolution *sol){
    int result = 1;
    while(cond != NULL && result) {
        result = cond->predicate(edge, sol, cond->a_data);
        cond = cond->conjunction;
    }
    return result;
}

EdgeCondition *create_edge_condition(int (*predicate)(Edge *, EdgeSolution *, void *), void *a_data, EdgeCondition *conj) {
    EdgeCondition *result = malloc(sizeof(EdgeCondition));
    result->predicate = predicate;
    result->a_data = a_data;
    result->conjunction = conj;
    return result;
}

void free_edge_conditions(EdgeCondition *cond) {
    while (cond != NULL) {
        EdgeCondition *next = cond->conjunction;
        free(cond);
        cond = next;
    }
}

int edge_is_empty(Edge *edge, EdgeSolution *sol, void *a_data) {
    return sol->capacity == 0;
}

int edge_needs_more_ts(Edge *edge, EdgeSolution *sol, void *a_data) {
    return sol->capacity < edge->minimal_capacity;
}

int edge_enough_capacity(Edge *edge, EdgeSolution *sol, void *a_data) {
    int capacity_change = 0;
    if (a_data) {
        capacity_change = *(int *) a_data;
    }
    return sol->capacity + capacity_change > edge->minimal_capacity;
}

int edge_any(Edge *edge, EdgeSolution *sol, void *a_data) {
    return 1;
}

int edge_none(Edge *edge, EdgeSolution *sol, void *a_data) {
    return 0;
}

int edge_has_trainset(Edge *edge, EdgeSolution *sol, void *a_data) {
    int ts = *(int *) a_data;
    return sol->num_trainsets[ts] > 0;
}

int edge_ends_in_station(Edge *edge, EdgeSolution *sol, void *a_data) {
    int station = *(int *) a_data;
    return edge->end_node->station->id == station;
}