//
// Created by fitli on 03.04.22.
//

#include <stdio.h>
#include "objective.h"
#include "datatypes.h"

//TODO zadefinovat na lepším místě
#define CAPACITY_PENALTY 1000000000000
#define MAX_LEN_PENALTY 1000000000000000

void get_num_ts(Solution *sol, Problem *problem, int *num_ts) {
    for (int ts = 0; ts < problem->num_trainset_types; ++ts) {
        num_ts[ts] = 0;
    }

    for (int st = 0; st < problem->num_stations; ++st) {
        Edge *source_edge = problem->stations[st].source_node->out_waiting;
        for (int ts = 0; ts < problem->num_trainset_types; ++ts) {
            num_ts[ts] += sol->edge_solution[source_edge->id].num_trainsets[ts];
        }
    }
}

void recalculate_objective(Solution *sol, Problem *problem) {
    sol->objective = 0;
    for (int ed_id = 0; ed_id < problem->num_edges; ed_id++) {
        struct Edge *edge = &problem->edges[ed_id];
        if(edge->type != SUBCONNECTION) {
            continue;
        }

        EdgeSolution *edge_sol = &sol->edge_solution[ed_id];

        for(int ts_id = 0; ts_id < problem->num_trainset_types; ts_id++) {
            int num_ts = edge_sol->num_trainsets[ts_id];
            struct Trainset *ts = &problem->trainset_types[ts_id];
            // OPERATIONAL COSTS
            sol->objective += (long long) problem->time_horizon * edge->distance * (ts->re_cost + ts->fe_cost + ts->el_cost) * num_ts;
            // ABROAD GAINS
            sol->objective -= (long long) problem->time_horizon * edge->distance_abroad * ts->abroad_gain * num_ts;
        }


        //SOFT CONSTRAINT PENALTIES
        if(edge_sol->capacity < edge->minimal_capacity) {
            sol->objective += CAPACITY_PENALTY * (edge->minimal_capacity - edge_sol->capacity);
        }

        if(edge_sol->capacity > problem->max_cap) {
            sol->objective += CAPACITY_PENALTY * (edge_sol->capacity - problem->max_cap);
        }

        int sum_ts = 0;
        for (int i = 0; i < problem->num_trainset_types; ++i) {
            sum_ts += edge_sol->num_trainsets[i];
        }
        if(sum_ts > problem->max_len) {
            sol->objective += (sum_ts - problem->max_len) * MAX_LEN_PENALTY;
        }
        if(sum_ts == 0) {
            sol->objective +=  MAX_LEN_PENALTY;
        }
    }

    // INVESTMENT COST
    int num_ts[problem->num_trainset_types];
    get_num_ts(sol, problem, num_ts);
    for (int i = 0; i < problem->num_trainset_types; ++i) {
        long long investment = num_ts[i] * (long long) problem->trainset_types[i].investment;
        sol->objective += (long long) ((double) investment * problem->mod_cost);
    }
}

/*
 * Update solution objective function after adding a trainset to an edge. Called BEFORE the traiset is really added.
 */
void update_obj_add_ts_to_edge(Solution *sol, const Problem *problem, const Trainset *ts, const Edge *edge) {
    if(edge->type == SOURCE_EDGE) {
        sol->objective += (long long) ((double) ts->investment * problem->mod_cost);
    }

    if(edge->type != SUBCONNECTION) {
        return;
    }

    sol->objective += (long long) problem->time_horizon * edge->distance * (ts->re_cost + ts->fe_cost + ts->el_cost);
    sol->objective -= (long long) problem->time_horizon * edge->distance_abroad * ts->abroad_gain;

    int old_edge_capacity = sol->edge_solution[edge->id].capacity;

    // PENALTY UNDER MINIMAL CAPACITY
    if(old_edge_capacity + ts->seats < edge->minimal_capacity) {
        sol->objective -= ts->seats * CAPACITY_PENALTY;
    }
    else if (old_edge_capacity <= edge->minimal_capacity) {
        sol->objective -= (edge->minimal_capacity - old_edge_capacity) * CAPACITY_PENALTY;
    }

    // PENALTY OVER MAXIMAL CAPACITY
    if (old_edge_capacity >= problem->max_cap) {
        sol->objective += ts->seats * CAPACITY_PENALTY;
    }
    else if(old_edge_capacity + ts->seats > problem->max_cap) {
        sol->objective += (old_edge_capacity + ts->seats - problem->max_cap) * CAPACITY_PENALTY;
    }

    // PENALTY FOR WRONG LENGTH
    int sum_ts = 1; // the new ts
    for (int i = 0; i < problem->num_trainset_types; ++i) {
        sum_ts += sol->edge_solution[edge->id].num_trainsets[i];
    }
    if(sum_ts > problem->max_len) {
        sol->objective += MAX_LEN_PENALTY;
    }
    if(sum_ts == 1) {
        sol->objective -=  MAX_LEN_PENALTY;
    }
}

/*
 * Update solution objective function after removing a trainset from an edge. Called BEFORE the trainset is really removed.
 */
void update_obj_remove_ts_from_edge(Solution *sol, const Problem *problem, const Trainset *ts, const Edge *edge) {
    if(edge->type == SOURCE_EDGE) {
        sol->objective -= (long long) ((double) ts->investment * problem->mod_cost);
    }

    if(edge->type != SUBCONNECTION) {
        return;
    }

    sol->objective -= (long long) problem->time_horizon * edge->distance * (ts->re_cost + ts->fe_cost + ts->el_cost);
    sol->objective += (long long) problem->time_horizon * edge->distance_abroad * ts->abroad_gain;

    int old_edge_capacity = sol->edge_solution[edge->id].capacity;
    // PENALTY UNDER MINIMAL CAPACITY
    if(old_edge_capacity < edge->minimal_capacity) {
        sol->objective += ts->seats * CAPACITY_PENALTY;
    }
    else if (old_edge_capacity - ts->seats < edge->minimal_capacity){
        sol->objective += (edge->minimal_capacity - (old_edge_capacity - ts->seats)) * CAPACITY_PENALTY;
    }

    // PENALTY OVER MAXIMAL CAPACITY
    if(old_edge_capacity - ts->seats > problem->max_cap) {
        sol->objective -= ts->seats * CAPACITY_PENALTY;
    }
    else if(old_edge_capacity > problem->max_cap) {
        sol->objective -= (old_edge_capacity - problem->max_cap) * CAPACITY_PENALTY;
    }

    int sum_ts = 0;
    for (int i = 0; i < problem->num_trainset_types; ++i) {
        sum_ts += sol->edge_solution[edge->id].num_trainsets[i];
    }
    if(sum_ts > problem->max_len) {
        sol->objective -= MAX_LEN_PENALTY;
    }
    if(sum_ts == 1) {
        sol->objective +=  MAX_LEN_PENALTY;
    }
}