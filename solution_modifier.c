//
// Created by fitli on 19.03.22.
//

#include <stdlib.h>
#include <stdio.h>
#include "solution_modifier.h"
#include "datatypes.h"
#include "objective.h"

void add_trainset_to_edge(Solution *sol, const Trainset *ts, const Edge *edge) {
    update_obj_add_ts_to_edge(sol, ts, edge);
    sol->edge_solution[edge->id].num_trainsets[ts->id] += 1;
    sol->edge_solution[edge->id].capacity += ts->seats;
}

void remove_trainset_from_edge(Solution *sol, const Trainset *ts, const Edge *edge) {
    if(sol->edge_solution[edge->id].num_trainsets[ts->id] > 0) {
        update_obj_remove_ts_from_edge(sol, ts, edge);
        sol->edge_solution[edge->id].num_trainsets[ts->id] -= 1;
        sol->edge_solution[edge->id].capacity -= ts->seats;
    }
    else(fprintf(stderr, "Tried to remove trainset %d form edge %d, but it is not present.\n", ts->id, edge->id));
}

void add_train_array(Solution *sol, const Trainset *trainset, const Edge **edges, int num_edges) {
    update_obj_add_ts(sol, trainset);
    for (int i = 0; i < num_edges; ++i) {
        if (edges[i] != NULL) {
            add_trainset_to_edge(sol, trainset, edges[i]);
        }
    }
}

void remove_train_array(Solution *sol, const Trainset *trainset, const Edge **edges, int num_edges) {
    update_obj_add_ts(sol, trainset);
    for (int i = 0; i < num_edges; ++i) {
        if(edges[i] != NULL) {
            remove_trainset_from_edge(sol, trainset, edges[i]);
        }
    }
}

void change_train_array(Solution *sol, const Trainset *old_ts, const Trainset *new_ts, Edge **edges, int num_edges) {
    remove_train_array(sol, old_ts, edges, num_edges);
    add_train_array(sol, new_ts, edges, num_edges);
}