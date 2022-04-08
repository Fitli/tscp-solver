//
// Created by fitli on 19.03.22.
//

#include <stdlib.h>
#include <stdio.h>
#include "solution_modifier.h"
#include "datatypes.h"
#include "objective.h"
#include "heuristics.h"


/*
 * Finds the first and the last SUBCONNESTION edges that can be chosen on the path from node_front to node_back
 * where the predicates front_condition and back_condition hold.
 */
void select_front_back_subcons(const Solution *sol, const Problem *problem, const Node *node_front, const Node *node_back,
                               int (*front_condition)(Edge *, EdgeSolution *), int (*back_condition)(Edge *, EdgeSolution *),
                               int *front_move_edge_id, int *back_move_edge_id) {
    int front_move_edges[problem->num_stations];
    int back_move_edges[problem->num_stations];

    for (int i = 0; i < problem->num_stations; i++) {
        front_move_edges[i] = -1;
        back_move_edges[i] = -1;
    }

    *back_move_edge_id = -1;
    *front_move_edge_id = -1;

    time_t end_time = node_back->time;

    while(node_front->time < end_time) {
        Edge *front_edge = node_front->out_subcon;
        EdgeSolution *front_edge_sol;
        if(front_edge != NULL)
             front_edge_sol = &sol->edge_solution[front_edge->id];
        if (front_edge != NULL && front_condition(front_edge, front_edge_sol)) {
            int front_st_id = front_edge->end_node->station->id;

            if (front_move_edges[front_st_id] == -1) {
                front_move_edges[front_st_id] = front_edge->id;
                if (front_move_edges[front_st_id] >= 0 && back_move_edges[front_st_id] >= 0) {
                    time_t arrival = problem->edges[front_move_edges[front_st_id]].end_node->time; // arrival time to front_st
                    time_t departure= problem->edges[back_move_edges[front_st_id]].start_node->time; // dep time from front_st
                    if (departure > arrival) { // check if departure from the station would be after arrival
                        *front_move_edge_id = front_move_edges[front_st_id];
                        *back_move_edge_id = back_move_edges[front_st_id];
                        return;
                    }
                }
            }
        }
        node_front = node_front->out_waiting->end_node;

        Edge *back_edge = node_back->in_subcon;
        EdgeSolution *back_edge_sol;
        if(back_edge != NULL)
            back_edge_sol = &sol->edge_solution[back_edge->id];
        if (back_edge != NULL && back_condition(back_edge, back_edge_sol)) {
            int back_st_id = back_edge->start_node->station->id;

            if (back_move_edges[back_st_id] == -1) {
                back_move_edges[back_st_id] = back_edge->id;
                if (front_move_edges[back_st_id] >= 0 && back_move_edges[back_st_id] >= 0) {
                    time_t arrival = problem->edges[front_move_edges[back_st_id]].end_node->time; // arrival time to front_st
                    time_t departure= problem->edges[back_move_edges[back_st_id]].start_node->time; // dep time from front_st
                    if (departure > arrival) { // check if departure from the station would be after arrival
                        *front_move_edge_id = front_move_edges[back_st_id];
                        *back_move_edge_id = back_move_edges[back_st_id];
                        return;
                    }
                }
            }
        }
        node_back = node_back->in_waiting->start_node;
    }
}

void add_trainset_to_edge(Solution *sol, const Trainset *ts, Edge *edge) {
    update_obj_add_ts_to_edge(sol, ts, edge);
    sol->edge_solution[edge->id].num_trainsets[ts->id] += 1;
    sol->edge_solution[edge->id].capacity += ts->seats;
}


/*
 * Just a helper function for add_train_two_side. Adds an edge to a buffer
 */
void add_to_buffer(Edge ***buffer, Edge *content, int *num_elems, int *buffer_cap) {
    if(num_elems == buffer_cap) {
        Edge **helper = realloc(*buffer, *buffer_cap * 2);
        if (helper == NULL) {
            fprintf(stderr, "Allocation error (add_to_buffer)\n");
            return;
        }
        *buffer = helper;
        *buffer_cap *= 2;
    }
    (*buffer)[*num_elems] = content;
    (*num_elems)++;
}


/*
 * Adds a new `trainset` that starts and ends in the `station`.
 * For choosing the exact path of this trainset, the `heuristic` will be used.
 *
 * `edges` is an output parameter for storing array of edges that were affected and `num_edges` is their number.
 */
void add_train_two_side(Solution *sol, const Problem *problem, const Trainset *trainset, const Station *station,
                        Edge ***edges, int *num_edges) {
    update_obj_add_ts(sol, trainset);
    Node *node_front = station->source_edge->end_node;
    Node *node_back = station->sink_edge->start_node;
    int front_move_edge_id = 0, back_move_edge_id = 0;

    int num_edges_front = 0;
    int cap_buffer_front = 128;
    int num_edges_back = 0;
    int cap_buffer_back = 128;
    Edge **buffer_front = malloc(cap_buffer_front * sizeof(Edge *));
    Edge **buffer_back = malloc(cap_buffer_back * sizeof(Edge *));



    while (front_move_edge_id >= 0) {

        select_front_back_subcons(sol, problem, node_front, node_back, &edge_is_empty, &edge_is_empty, &front_move_edge_id, &back_move_edge_id);
        if(front_move_edge_id < 0) {
            select_front_back_subcons(sol, problem, node_front, node_back, &edge_is_empty, &edge_needs_more_ts, &front_move_edge_id, &back_move_edge_id);
        }
        if(front_move_edge_id < 0) {
            select_front_back_subcons(sol, problem, node_front, node_back, &edge_needs_more_ts, &edge_is_empty, &front_move_edge_id, &back_move_edge_id);
        }
        if(front_move_edge_id < 0) {
            select_front_back_subcons(sol, problem, node_front, node_back, &edge_is_empty, &edge_any, &front_move_edge_id, &back_move_edge_id);
        }
        if(front_move_edge_id < 0) {
            select_front_back_subcons(sol, problem, node_front, node_back, &edge_any, &edge_is_empty, &front_move_edge_id, &back_move_edge_id);
        }
        if(front_move_edge_id < 0) {
            break;
        }

        while(node_front->out_subcon == NULL || node_front->out_subcon->id != front_move_edge_id) {
            add_trainset_to_edge(sol, trainset, node_front->out_waiting);
            add_to_buffer(&buffer_front, node_front->out_waiting, &num_edges_front, &cap_buffer_front);
            node_front = node_front->out_waiting->end_node;
        }
        add_trainset_to_edge(sol, trainset, node_front->out_subcon);
        add_to_buffer(&buffer_front, node_front->out_subcon, &num_edges_front, &cap_buffer_front);
        node_front = node_front->out_subcon->end_node;

        while(node_back->in_subcon == NULL || node_back->in_subcon->id != back_move_edge_id) {
            add_trainset_to_edge(sol, trainset, node_back->in_waiting);
            add_to_buffer(&buffer_back, node_front->in_waiting, &num_edges_back, &cap_buffer_back);
            node_back = node_back->in_waiting->start_node;
        }
        add_trainset_to_edge(sol, trainset, node_back->in_subcon);
        add_to_buffer(&buffer_back, node_front->in_subcon, &num_edges_back, &cap_buffer_back);
        node_back = node_back->in_subcon->start_node;
    }

    while (node_front->id != node_back->id) {
        add_trainset_to_edge(sol, trainset, node_front->out_waiting);
        add_to_buffer(&buffer_front, node_front->out_waiting, &num_edges_front, &cap_buffer_front);
        node_front = node_front->out_waiting->end_node;
    }

    for(int i = 0; i<num_edges_back; i++) {
        add_to_buffer(&buffer_front, buffer_back[i], &num_edges_front, &cap_buffer_back);
    }

    free(buffer_back);

    if(edges != NULL) {
        *edges = buffer_front;
    } else {
        free(buffer_front);
    }

    if(num_edges != NULL) {
        *num_edges = num_edges_front;
    }
}
