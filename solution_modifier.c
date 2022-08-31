//
// Created by fitli on 19.03.22.
//

#include <stdlib.h>
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
    update_obj_add_ts(sol, problem, trainset);
    for (int i = 0; i < num_edges; ++i) {
        if (edges[i] != NULL) {
            add_trainset_to_edge(sol, problem, trainset, edges[i]);
        }
    }
}

void remove_train_array(Solution *sol, const Problem *problem, const Trainset *trainset, Edge **edges, int num_edges) {
    update_obj_remove_ts(sol, problem, trainset);
    for (int i = 0; i < num_edges; ++i) {
        if(edges[i] != NULL) {
            remove_trainset_from_edge(sol, problem, trainset, edges[i]);
        }
    }
}

void change_train_array(Solution *sol, const Problem *problem, const Trainset *old_ts, const Trainset *new_ts, int old_ts_amount, int new_ts_amount, Edge **edges, int num_edges) {
    for(int i = 0; i < old_ts_amount; i++) {
        remove_train_array(sol, problem, old_ts, edges, num_edges);
    }
    for(int i = 0; i < new_ts_amount; i++) {
        add_train_array(sol, problem, new_ts, edges, num_edges);
    }
}

void move_to_other_subcon(Solution *sol, const Problem *problem, const Trainset *ts, Edge *old_edge, Edge *new_edge) {
    remove_trainset_from_edge(sol, problem, ts, old_edge);
    add_trainset_to_edge(sol, problem, ts, new_edge);
    Node *node = old_edge->start_node;
    while (node != new_edge->start_node) {
        if(node->time < new_edge->start_node->time) {
            add_trainset_to_edge(sol, problem, ts, node->out_waiting);
            node = node->out_waiting->end_node;
        } else {
            remove_trainset_from_edge(sol, problem, ts, node->in_waiting);
            node = node->in_waiting->start_node;
        }
    }
    node = old_edge->end_node;
    while (node != new_edge->end_node) {
        if(node->time < new_edge->end_node->time) {
            remove_trainset_from_edge(sol, problem, ts, node->out_waiting);
            node = node->out_waiting->end_node;
        } else {
            add_trainset_to_edge(sol, problem, ts, node->in_waiting);
            node = node->in_waiting->start_node;
        }
    }
}