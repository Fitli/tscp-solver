//
// Created by fitli on 03.04.22.
//

#include "objective.h"
#include "datatypes.h"

//TODO zadefinovat na lepším místě
#define CAPACITY_PENALTY 10000000

void recalculate_objective(Solution *sol, Problem *problem) {
    sol->objective = 0;
    for (int ed_id = 0; ed_id < problem->num_edges; ed_id++) {
        struct Edge *edge = &problem->edges[ed_id];
        EdgeSolution *edge_sol = &sol->edge_solution[ed_id];

        for(int ts_id = 0; ts_id < problem->num_trainset_types; ts_id++) {
            int num_ts = edge_sol->num_trainsets[ts_id];
            struct Trainset *ts = &problem->trainset_types[ts_id];
            sol->objective -= edge->distance * (ts->re_cost + ts->fe_cost + ts->el_cost) * num_ts;
            sol->objective += edge->distance_abroad * ts->abroad_gain * num_ts;
            if (edge->type == SOURCE_EDGE) {
                sol->objective -= ts->investment * num_ts;
            }
        }

        if(edge_sol->capacity < edge->minimal_capacity) {
            sol->objective -= CAPACITY_PENALTY * (edge->minimal_capacity - edge_sol->capacity);
        }
    }
}

/*
 * Update solution objective function after adding a trainset to an edge. Called BEFORE the traiset is really added.
 */
void update_obj_add_ts_to_edge(Solution *sol, const Trainset *ts, Edge *edge) {
    sol->objective -= edge->distance * (ts->re_cost + ts->fe_cost + ts->el_cost);
    sol->objective += edge->distance_abroad * ts->abroad_gain;

    int old_edge_capacity = sol->edge_solution[edge->id].capacity;
    if(old_edge_capacity + ts->seats < edge->minimal_capacity) {
        sol->objective += ts->seats * CAPACITY_PENALTY;
    } else {
        sol->objective += (edge->minimal_capacity - old_edge_capacity) * CAPACITY_PENALTY;
    }
}

/*
 * Update solution objective function after removing a trainset from an edge. Called BEFORE the traiset is really removed.
 */
void update_obj_remove_ts_from_edge(Solution *sol, const Trainset *ts, Edge *edge) {
    sol->objective += edge->distance * (ts->re_cost + ts->fe_cost + ts->el_cost);
    sol->objective -= edge->distance_abroad * ts->abroad_gain;

    int old_edge_capacity = sol->edge_solution[edge->id].capacity;
    if(old_edge_capacity - ts->seats < edge->minimal_capacity) {
        sol->objective -= edge->minimal_capacity - (old_edge_capacity - ts->seats);
    }
}

/*
 * Update solution objective function after insterting a new trainset to the system
 */
void update_obj_add_ts(Solution *sol, const Trainset *ts) {
    sol->objective -= ts->investment;
}

/*
 * Update solution objective function after removing whole trainset from the system
 */
void update_obj_remove_ts(Solution *sol, const Trainset *ts) {
    sol->objective -= ts->investment;
}