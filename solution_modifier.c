//
// Created by Ivana Krumlová on 19.03.22.
//

#include <stdio.h>
#include "solution_modifier.h"
#include "datatypes.h"
#include "objective.h"

void add_trainset_to_edge(Solution *sol, const Problem *problem, const Trainset *ts, const Edge *edge) {
    update_obj_add_ts_to_edge(sol, problem, ts, edge);
    sol->edge_solution[edge->id].num_trainsets[ts->id] += 1;
    sol->edge_solution[edge->id].capacity += ts->seats;
}

void remove_trainset_from_edge(Solution *sol, const Problem *problem, const Trainset *ts, const Edge *edge) {
    if(sol->edge_solution[edge->id].num_trainsets[ts->id] > 0) {
        update_obj_remove_ts_from_edge(sol, problem, ts, edge);
        sol->edge_solution[edge->id].num_trainsets[ts->id] -= 1;
        sol->edge_solution[edge->id].capacity -= ts->seats;
    }
    else(fprintf(stderr, "Tried to remove trainset %d form edge %d, but it is not present.\n", ts->id, edge->id));
}

void add_train_array(Solution *sol, const Problem *problem, const Trainset *trainset, Edge **edges, int num_edges) {
    if(num_edges == 0) {
        return;
    }
    for (int i = 0; i < num_edges; ++i) {
        if (edges[i] != NULL) {
            add_trainset_to_edge(sol, problem, trainset, edges[i]);
        }
    }
}

void remove_train_array(Solution *sol, const Problem *problem, const Trainset *trainset, Edge **edges, int num_edges) {
    if(num_edges == 0) {
        return;
    }
    for (int i = 0; i < num_edges; ++i) {
        if(edges[i] != NULL) {
            remove_trainset_from_edge(sol, problem, trainset, edges[i]);
        }
    }
}