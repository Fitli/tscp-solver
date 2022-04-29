//
// Created by fitli on 27.04.22.
//

#include <stdlib.h>
#include <stdio.h>
#include "change_finder.h"
#include "heuristics.h"


/*
 * Finds the first and the last SUBCONNESTION edges that can be chosen on the path from node_front to node_back
 * where the predicates front_condition and back_condition hold.
 */
void select_front_back_subcons(const Solution *sol, const Problem *problem, const Node *node_front, const Node *node_back,
                               EdgeCondition *front_cond, EdgeCondition *back_cond,
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
        if (front_edge != NULL && eval_edge_condition(front_cond, front_edge, front_edge_sol)) {
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
        if (back_edge != NULL && eval_edge_condition(back_cond, back_edge, back_edge_sol)) {
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

/*
 * Just a helper function for find_train_two_side. Adds an edge to a buffer
 */
void add_to_buffer(Edge ***buffer, Edge *content, int *num_elems, int *buffer_cap) {
    if(*num_elems >= *buffer_cap) {
        size_t size = (*buffer_cap) * 2 * sizeof(Edge *);
        Edge **helper = realloc(*buffer, size);
        if (helper == NULL) {
            fprintf(stderr, "Allocation error (add_to_buffer)\n");
            return;
        }
        (*buffer) = helper;
        (*buffer_cap) *= 2;
    }
    (*buffer)[*num_elems] = content;
    (*num_elems)++;
}
void find_train_two_side(Solution *sol, const Problem *problem, const Station *station, int num_conds,
                         EdgeCondition **front_conditions, EdgeCondition **back_conditions,
                         Edge ***edges, int *num_edges) {
    Node *node_front = station->source_edge->end_node;
    Node *node_back = station->sink_edge->start_node;
    int front_move_edge_id = 0, back_move_edge_id = 0;

    int num_edges_front = 0;
    int cap_buffer_front = 128;
    int num_edges_back = 0;
    int cap_buffer_back = 128;
    Edge **buffer_front = malloc(cap_buffer_front * sizeof(Edge *));
    Edge **buffer_back = malloc(cap_buffer_back * sizeof(Edge *));

    add_to_buffer(&buffer_front, station->source_edge, &num_edges_front, &cap_buffer_front);
    add_to_buffer(&buffer_back, station->sink_edge, &num_edges_back, &cap_buffer_back);

    while (front_move_edge_id >= 0) {
        for (int i = 0; i < num_conds; i++) {
            select_front_back_subcons(sol, problem, node_front, node_back, front_conditions[i], back_conditions[i], &front_move_edge_id, &back_move_edge_id);
            if(front_move_edge_id >= 0) {
                break;
            }
        }
        if(front_move_edge_id < 0) {
            break;
        }

        while(node_front->out_subcon == NULL || node_front->out_subcon->id != front_move_edge_id) {
            add_to_buffer(&buffer_front, node_front->out_waiting, &num_edges_front, &cap_buffer_front);
            node_front = node_front->out_waiting->end_node;
        }
        add_to_buffer(&buffer_front, node_front->out_subcon, &num_edges_front, &cap_buffer_front);
        node_front = node_front->out_subcon->end_node;

        while(node_back->in_subcon == NULL || node_back->in_subcon->id != back_move_edge_id) {
            add_to_buffer(&buffer_back, node_back->in_waiting, &num_edges_back, &cap_buffer_back);
            node_back = node_back->in_waiting->start_node;
        }
        add_to_buffer(&buffer_back, node_back->in_subcon, &num_edges_back, &cap_buffer_back);
        node_back = node_back->in_subcon->start_node;
    }

    while (node_front->id != node_back->id) {
        add_to_buffer(&buffer_front, node_front->out_waiting, &num_edges_front, &cap_buffer_front);
        node_front = node_front->out_waiting->end_node;
    }

    for(int i = 0; i<num_edges_back; i++) {
        add_to_buffer(&buffer_front, buffer_back[i], &num_edges_front, &cap_buffer_front);
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

void select_next_out_edge(const Solution *sol, const Node *node, EdgeCondition *out_e_cond,
                          EdgeCondition *wait_e_cond, int *selected_edge_id) {
    *selected_edge_id = -1;
    while (node != NULL) {
        if(node->out_subcon && eval_edge_condition(out_e_cond, node->out_subcon, sol->edge_solution)) {
            *selected_edge_id = node->out_subcon->id;
        }
        if(eval_edge_condition(wait_e_cond, node->out_waiting, sol->edge_solution)) {
            node = node->out_waiting->end_node;
        }
        else {
            break;
        }
    }
}

void select_prev_out_edge(const Solution *sol, const Node *node, EdgeCondition *out_e_cond,
                          EdgeCondition *wait_e_cond, int *selected_edge_id) {
    *selected_edge_id = -1;
    while (node != NULL) {
        if(node->out_subcon && eval_edge_condition(out_e_cond, node->out_subcon, sol->edge_solution)) {
            *selected_edge_id = node->out_subcon->id;
        }
        if(eval_edge_condition(wait_e_cond, node->in_waiting, sol->edge_solution)) {
            node = node->in_waiting->start_node;
        }
        else {
            break;
        }
    }
}

void select_next_in_edge(const Solution *sol, const Node *node, EdgeCondition *in_e_cond,
                         EdgeCondition *wait_e_cond, int *selected_edge_id) {
    *selected_edge_id = -1;
    while (node != NULL) {
        if(node->in_subcon && eval_edge_condition(in_e_cond, node->in_subcon, sol->edge_solution)) {
            *selected_edge_id = node->in_subcon->id;
        }
        if(eval_edge_condition(wait_e_cond, node->out_waiting, sol->edge_solution)) {
            node = node->out_waiting->end_node;
        }
        else {
            break;
        }
    }
}

void select_prev_in_edge(const Solution *sol, const Node *node, EdgeCondition *in_e_cond,
                         EdgeCondition *wait_e_cond, int *selected_edge_id) {
    *selected_edge_id = -1;
    while (node != NULL) {
        if(node->in_subcon && eval_edge_condition(in_e_cond, node->in_subcon, sol->edge_solution)) {
            *selected_edge_id = node->in_subcon->id;
        }
        if(eval_edge_condition(wait_e_cond, node->in_waiting, sol->edge_solution)) {
            node = node->in_waiting->start_node;
        }
        else {
            break;
        }
    }
}