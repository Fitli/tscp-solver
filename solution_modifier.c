//
// Created by fitli on 19.03.22.
//

#include "solution_modifier.h"
#include "datatypes.h"

/*
 * Heuristics for add_train_two_side
 */
void first_empty(const Solution *sol, const Problem *problem, const Node *node_front, const Node *node_back,
                 int *front_move_edge_id, int *back_move_edge_id) {
    int front_move_edges[problem->num_stations];
    int back_move_edges[problem->num_stations];

    for (int i = 0; i < problem->num_stations; i++) {
        front_move_edges[i] = -1;
        back_move_edges[i] = -1;
    }

    *back_move_edge_id = -1;
    *front_move_edge_id = -1;

    while(node_front->id != node_back->id) {
        Edge *front_edge = node_front->out_subcon;
        if (front_edge != NULL) {
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

        if(node_front->id == node_back->id) {
            return;
        }

        Edge *back_edge = node_back->in_subcon;
        if (back_edge != NULL) {
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

void add_trainset_to_node(Solution *sol, const Trainset *ts, Edge *edge) {
    sol->edge_solution[edge->id].num_trainsets[ts->id] += 1;
    sol->edge_solution[edge->id].capacity += ts->seats;
}


void add_train_two_side(Solution *sol, const Problem *problem, const Trainset *trainset, const Station *station,
                        void (*heuristic)(const Solution *, const Problem *, const Node *, const Node *, int *, int *)) {
    Node *node_front = station->source_edge->end_node;
    Node *node_back = station->sink_edge->start_node;
    int front_move_edge_id = 0, back_move_edge_id = 0;
    heuristic(sol, problem, node_front, node_back, &front_move_edge_id, &back_move_edge_id);

    while (front_move_edge_id >= 0) {
        while(node_front->out_subcon == NULL || node_front->out_subcon->id != front_move_edge_id) {
            add_trainset_to_node(sol, trainset, node_front->out_waiting);
            node_front = node_front->out_waiting->end_node;
        }
        add_trainset_to_node(sol, trainset, node_front->out_subcon);
        node_front = node_front->out_subcon->end_node;

        while(node_back->in_subcon == NULL || node_back->in_subcon->id != back_move_edge_id) {
            add_trainset_to_node(sol, trainset, node_back->in_waiting);
            node_back = node_back->in_waiting->start_node;
        }
        add_trainset_to_node(sol, trainset, node_back->in_subcon);
        node_back = node_back->in_subcon->start_node;

        heuristic(sol, problem, node_front, node_back, &front_move_edge_id, &back_move_edge_id);
    }

    while (node_front->id != node_back->id) {
        add_trainset_to_node(sol, trainset, node_front->out_waiting);
        node_front = node_front->out_waiting->end_node;
    }
}
