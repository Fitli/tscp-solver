//
// Created by fitli on 27.04.22.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "change_finder.h"
#include "heuristics.h"

void fastest_path_from_node_to_stations(Solution *sol, const Problem *problem, const Node *start, EdgeCondition *move_condition,
                                        EdgeCondition *wait_condition, Edge ***edges, int *nums_edges);

/*
 * Finds the first and the last SUBCONNESTION edges that can be chosen on the path from node_front to node_back
 * where the predicates front_condition and back_condition hold.
 */
void select_front_back_subcons(const Solution *sol, const Problem *problem, const Node *node_front, const Node *node_back,
                               EdgeCondition *front_cond, EdgeCondition *back_cond, EdgeCondition *wait_cond,
                               int *front_move_edge_id, int *back_move_edge_id) {
    int front_move_edges[problem->num_stations];
    int back_move_edges[problem->num_stations];

    for (int i = 0; i < problem->num_stations; i++) {
        front_move_edges[i] = -1;
        back_move_edges[i] = -1;
    }

    *back_move_edge_id = -1;
    *front_move_edge_id = -1;

    int is_new_front = 1;
    int is_new_back = 1;

    time_t end_time = node_back->time;

    while(node_front->time < end_time && (is_new_front || is_new_back)) {
        if (is_new_front) {
            Edge *front_edge = node_front->out_subcon;
            EdgeSolution *front_edge_sol;
            if (front_edge != NULL)
                front_edge_sol = &sol->edge_solution[front_edge->id];
            if (front_edge != NULL && eval_edge_condition(front_cond, front_edge, front_edge_sol)) {
                int front_st_id = front_edge->end_node->station->id;

                if (front_move_edges[front_st_id] == -1) {
                    front_move_edges[front_st_id] = front_edge->id;
                    if (front_move_edges[front_st_id] >= 0 && back_move_edges[front_st_id] >= 0) {
                        time_t arrival = problem->edges[front_move_edges[front_st_id]].end_node->time; // arrival time to front_st
                        time_t departure = problem->edges[back_move_edges[front_st_id]].start_node->time; // dep time from front_st
                        if (departure > arrival) { // check if departure from the station would be after arrival
                            *front_move_edge_id = front_move_edges[front_st_id];
                            *back_move_edge_id = back_move_edges[front_st_id];
                            return;
                        }
                    }
                }
            }
            if (eval_edge_condition(wait_cond, node_front->out_waiting, &sol->edge_solution[node_front->out_waiting->id])) {
                node_front = node_front->out_waiting->end_node;
            }
            else {
                is_new_front  = 0;
            }
        }

        if (is_new_back) {
            Edge *back_edge = node_back->in_subcon;
            EdgeSolution *back_edge_sol;
            if (back_edge != NULL)
                back_edge_sol = &sol->edge_solution[back_edge->id];
            if (back_edge != NULL && eval_edge_condition(back_cond, back_edge, back_edge_sol)) {
                int back_st_id = back_edge->start_node->station->id;

                if (back_move_edges[back_st_id] == -1) {
                    back_move_edges[back_st_id] = back_edge->id;
                    if (front_move_edges[back_st_id] >= 0 && back_move_edges[back_st_id] >= 0) {
                        time_t arrival = problem->edges[front_move_edges[back_st_id]].end_node->time; // arrival time to front_st
                        time_t departure = problem->edges[back_move_edges[back_st_id]].start_node->time; // dep time from front_st
                        if (departure > arrival) { // check if departure from the station would be after arrival
                            *front_move_edge_id = front_move_edges[back_st_id];
                            *back_move_edge_id = back_move_edges[back_st_id];
                            return;
                        }
                    }
                }
            }
            if (eval_edge_condition(wait_cond, node_back->in_waiting, &sol->edge_solution[node_back->in_waiting->id])) {
                node_back = node_back->in_waiting->start_node;
            } else {
                is_new_back = 0;
            }
        }
    }
}

/*
 * Just a helper function for find_train_end_to_end. Adds an edge to a buffer
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
int find_train_end_to_end(Solution *sol, const Problem *problem, const Station *station, int num_conds,
                          EdgeCondition **front_conditions, EdgeCondition **back_conditions, EdgeCondition *wait_condition,
                          Edge ***edges, int *num_edges) {

    Node *node_front = station->source_node;
    Node *node_back = station->sink_node;

    int res = find_train_between_nodes(sol, problem, node_front, node_back, num_conds, front_conditions, back_conditions, wait_condition, edges, num_edges);

    // no train found, return
    if(res == 0) {
        return 0;
    }

}

//greedy finding of a journey between two nodes of the same station
int find_train_between_nodes(Solution *sol, const Problem *problem, const Node *node_front, const Node *node_back, int num_conds,
                          EdgeCondition **front_conditions, EdgeCondition **back_conditions, EdgeCondition *wait_condition,
                          Edge ***edges, int *num_edges) {
    int front_move_edge_id = 0, back_move_edge_id = 0;

    int num_edges_front = 0;
    int num_edges_back = 0;
    Edge *buffer_front[problem->num_edges];
    Edge *buffer_back[problem->num_edges];

    if (node_front->station != node_back->station) {
        Edge** station_edges[problem->num_stations];
        int station_num_edges[problem->num_stations];
        for (int i = 0; i < num_conds; ++i) {
            fastest_path_from_node_to_stations(sol, problem, node_front, front_conditions[i], wait_condition, station_edges, station_num_edges);
            if(station_edges[node_back->station->id] == NULL) {
                continue;
            }
            for (int j = 0; j < station_num_edges[node_back->station->id]; ++j) {
                buffer_front[j] = station_edges[node_back->station->id][j];
            }
            num_edges_front = station_num_edges[node_back->station->id];

            node_front = buffer_front[num_edges_front - 1]->end_node;

            for (int j = 0; j < problem->num_stations; ++j) {
                if(station_edges[j])
                    free(station_edges[j]);
            }
            break;
        }
    }

    if(node_front->station != node_back->station || node_front->time > node_back->time) {
        *edges = NULL;
        *num_edges = 0;
        return 0;
    }

    while (front_move_edge_id >= 0) {
        for (int i = 0; i < num_conds; i++) {
            select_front_back_subcons(sol, problem, node_front, node_back, front_conditions[i], back_conditions[i], wait_condition, &front_move_edge_id, &back_move_edge_id);
            if(front_move_edge_id >= 0) {
                break;
            }
        }
        if(front_move_edge_id < 0) {
            break;
        }

        while(node_front->out_subcon == NULL || node_front->out_subcon->id != front_move_edge_id) {
            buffer_front[num_edges_front] = node_front->out_waiting;
            num_edges_front++;
            node_front = node_front->out_waiting->end_node;
        }
        buffer_front[num_edges_front] = node_front->out_subcon;
        num_edges_front++;
        node_front = node_front->out_subcon->end_node;

        while(node_back->in_subcon == NULL || node_back->in_subcon->id != back_move_edge_id) {
            buffer_back[num_edges_back] = node_back->in_waiting;
            num_edges_back++;
            node_back = node_back->in_waiting->start_node;
        }
        buffer_back[num_edges_back] = node_back->in_subcon;
        num_edges_back++;
        node_back = node_back->in_subcon->start_node;
    }

    while (node_front->id != node_back->id) {
        if(!eval_edge_condition(wait_condition, node_front->out_waiting, &sol->edge_solution[node_front->out_waiting->id])) {
            *edges = NULL;
            return 0;
        }
        buffer_front[num_edges_front] = node_front->out_waiting;
        num_edges_front++;
        node_front = node_front->out_waiting->end_node;
    }

    if(edges != NULL) {
        *edges = malloc((num_edges_front + num_edges_back) * sizeof(Edge *));
        for(int i = 0; i<num_edges_front; i++) {
            (*edges)[i] = buffer_front[i];
        }
        for(int i = 0; i<num_edges_back; i++) {
            (*edges)[num_edges_front + num_edges_back - 1 - i] = buffer_back[i];
        }
    }

    if(num_edges != NULL) {
        *num_edges = num_edges_front + num_edges_back;
    }
    return 1;
}

void select_next_out_edge(const Solution *sol, const Node *node, EdgeCondition *out_e_cond,
                          EdgeCondition *wait_e_cond, int *selected_edge_id) {
    *selected_edge_id = -1;
    while (node->out_waiting != NULL) {
        if(node->out_subcon && eval_edge_condition(out_e_cond, node->out_subcon, &sol->edge_solution[node->out_subcon->id])) {
            *selected_edge_id = node->out_subcon->id;
            return;
        }
        if(eval_edge_condition(wait_e_cond, node->out_waiting, &sol->edge_solution[node->out_waiting->id])) {
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
    while (node->in_waiting != NULL) {
        if(node->out_subcon && eval_edge_condition(out_e_cond, node->out_subcon,  &sol->edge_solution[node->out_subcon->id])) {
            *selected_edge_id = node->out_subcon->id;
            return;
        }
        if(eval_edge_condition(wait_e_cond, node->in_waiting, &sol->edge_solution[node->in_waiting->id])) {
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
    while (node->out_waiting != NULL) {
        if(node->in_subcon && eval_edge_condition(in_e_cond, node->in_subcon, &sol->edge_solution[node->in_subcon->id])) {
            *selected_edge_id = node->in_subcon->id;
            return;
        }
        if(eval_edge_condition(wait_e_cond, node->out_waiting, &sol->edge_solution[node->out_waiting->id])) {
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
    while (node->in_waiting != NULL) {
        if(node->in_subcon && eval_edge_condition(in_e_cond, node->in_subcon, &sol->edge_solution[node->in_subcon->id])) {
            *selected_edge_id = node->in_subcon->id;
            return;
        }
        if(eval_edge_condition(wait_e_cond, node->in_waiting, &sol->edge_solution[node->in_waiting->id])) {
            node = node->in_waiting->start_node;
        }
        else {
            break;
        }
    }
}

/*
 * ~ Dijkstra
 */
typedef struct Dijkstra_queue_elem Queue_elem;

struct Dijkstra_queue_elem {
    Edge **path;
    int path_len;
    const Node *final_node;
    bool relaxed;
    EdgeCondition *condition;
};

void fastest_path_from_node_to_stations(Solution *sol, const Problem *problem, const Node *start, EdgeCondition *move_condition,
                                        EdgeCondition *wait_condition, Edge ***edges, int *nums_edges) {
    Queue_elem queue[problem->num_stations];
    for (int i = 0; i < problem->num_stations; ++i) {
        queue[i].final_node = NULL;
        queue[i].path = NULL;
        queue[i].relaxed = true;
        queue[i].condition = create_edge_condition(edge_ends_in_station, &problem->stations[i].id, move_condition);
    }
    queue[start->station->id].final_node = start;
    queue[start->station->id].path_len = 0;
    queue[start->station->id].relaxed = false;

    bool all_relaxed = false;
    while (!all_relaxed) {
        all_relaxed = true;
        for (int i = 0; i < problem->num_stations; ++i) {
            if(queue[i].relaxed) {
                continue;
            }
            all_relaxed = false;
            for (int j = 0; j < problem->num_stations; ++j) {
                if(i == j) {
                    continue;
                }
                int final_edge_id = -1;
                select_next_out_edge(sol, queue[i].final_node, queue[j].condition, wait_condition, &final_edge_id);
                if(final_edge_id >= 0 && (queue[j].final_node == NULL || problem->edges[final_edge_id].end_node->time < queue[j].final_node->time)) {
                    queue[j].final_node = problem->edges[final_edge_id].end_node;
                    queue[j].relaxed = false;
                    queue[j].path_len = queue[i].path_len + problem->edges[final_edge_id].start_node->time - queue[i].final_node->time + 1;
                    if(queue[j].path) {
                        free(queue[j].path);
                    }
                    queue[j].path = malloc(queue[j].path_len * sizeof(Edge *));
                    int k = 0;
                    for(; k < queue[i].path_len; k++) {
                        queue[j].path[k] = queue[i].path[k];
                    }
                    Node *node = queue[i].final_node;
                    while(node->out_subcon == NULL || node->out_subcon->id != final_edge_id) {
                        queue[j].path[k] = node->out_waiting;
                        k++;
                        node = node->out_waiting->end_node;
                    }
                    queue[j].path[k] = node->out_subcon;
                }
            }
            queue[i].relaxed = true;
        }
    }
    for (int i = 0; i < problem->num_stations; ++i) {
        edges[i] = queue[i].path;
        nums_edges[i] = queue[i].path_len;
    }
}

void latest_path_from_stations_to_node(Solution *sol, const Problem *problem, const Node *end, EdgeCondition *move_condition,
                                       EdgeCondition *wait_condition, Edge ***edges, int *nums_edges) {
    Queue_elem queue[problem->num_stations];
    for (int i = 0; i < problem->num_stations; ++i) {
        queue[i].final_node = NULL;
        queue[i].path = NULL;
        queue[i].relaxed = true;
        queue[i].condition = create_edge_condition(edge_start_in_station, &problem->stations[i].id, move_condition);
    }
    queue[end->station->id].final_node = end;
    queue[end->station->id].path_len = 0;
    queue[end->station->id].relaxed = false;

    bool all_relaxed = false;
    while (!all_relaxed) {
        all_relaxed = true;
        for (int i = 0; i < problem->num_stations; ++i) {
            if(queue[i].relaxed) {
                continue;
            }
            all_relaxed = false;
            for (int j = 0; j < problem->num_stations; ++j) {
                if(i == j) {
                    continue;
                }
                int final_edge_id;
                select_prev_in_edge(sol, queue[i].final_node, queue[i].condition, wait_condition, &final_edge_id);
                if(final_edge_id >= 0 && (queue[j].final_node == NULL || problem->edges[final_edge_id].start_node->time > queue[j].final_node->time)) {
                    queue[j].final_node = problem->edges[final_edge_id].start_node;
                    queue[j].relaxed = false;
                    queue[j].path_len = queue[i].path_len + queue[i].final_node->time - problem->edges[final_edge_id].end_node->time + 1;
                    if(queue[j].path) {
                        free(queue[j].path);
                    }
                    queue[j].path = malloc(queue[j].path_len * sizeof(Edge *));
                    int k = 1;
                    for(; k <= queue[i].path_len; k++) {
                        queue[j].path[queue[j].path_len - k] = queue[i].path[queue[i].path_len - k];
                    }
                    Node *node = queue[i].final_node;
                    while(node->in_subcon->id != final_edge_id) {
                        queue[j].path[queue[j].path_len - k] = node->in_waiting;
                        k++;
                    }
                    queue[j].path[queue[j].path_len - k] = node->in_subcon;
                }
            }
            queue[i].relaxed = true;
        }
    }
    for (int i = 0; i < problem->num_stations; ++i) {
        edges[i] = queue[i].path;
        nums_edges[i] = queue[i].path_len;
    }
}

//Find a sequence of edges that starts and ends in the same station and contains `edge`.
bool find_part_containing_edge(Solution *sol, const Problem *problem, Edge *edge, EdgeCondition *move_condition,
                               EdgeCondition *wait_condition, Edge ***edges, int *num_edges) {
    if ((edge->type == SUBCONNECTION && !eval_edge_condition(move_condition, edge, &sol->edge_solution[edge->id])) ||
        (edge->type != SUBCONNECTION && !eval_edge_condition(wait_condition, edge, &sol->edge_solution[edge->id]))) {
        *edges = NULL;
        *num_edges = 0;
        return false;
    }

    if(edge->start_node->station == edge->end_node->station) {
        *edges = malloc(sizeof(Edge));
        **edges = edge;
        *num_edges = 1;
        return true;
    }

    Edge** station_edges[problem->num_stations];
    int station_num_edges[problem->num_stations];
    fastest_path_from_node_to_stations(sol, problem, edge->end_node, move_condition, wait_condition, station_edges, station_num_edges);
    if(station_edges[edge->start_node->station->id]) {
        *num_edges = station_num_edges[edge->start_node->station->id] + 1;
        *edges = malloc(*num_edges * sizeof(Edge *));
        *edges[0] = edge;
        for (int i = 0; i < station_num_edges[edge->start_node->station->id]; ++i) {
            (*edges)[i+1] = station_edges[edge->start_node->station->id][i];
        }
        for (int i = 0; i < problem->num_stations; ++i) {
            if(station_edges[i])
                free(station_edges[i]);
        }
        return true;
    }
    for (int i = 0; i < problem->num_stations; ++i) {
        if(station_edges[i])
            free(station_edges[i]);
    }

    latest_path_from_stations_to_node(sol, problem, edge->start_node, move_condition, wait_condition, station_edges, station_num_edges);
    if(station_edges[edge->end_node->station->id]) {
        *num_edges = station_num_edges[edge->end_node->station->id] + 1;
        *edges = malloc(*num_edges * sizeof(Edge));
        for (int i = 0; i < station_num_edges[edge->end_node->station->id]; ++i) {
            *edges[i] = station_edges[edge->end_node->station->id][i];
        }
        *edges[*num_edges - 1] = edge;
        for (int i = 0; i < problem->num_stations; ++i) {
            if(station_edges[i])
                free(station_edges[i]);
        }
        return true;
    }

    *edges = NULL;
    *num_edges = 0;
    for (int i = 0; i < problem->num_stations; ++i) {
        if(station_edges[i])
            free(station_edges[i]);
    }
    return false;
}

int find_train_containing_edge(Solution *sol, const Problem *problem, const Edge *edge, int num_conds,
                                EdgeCondition **move_conditions, EdgeCondition *wait_condition,Edge ***edges, int *num_edges) {
    Edge **beginning = NULL;
    Edge **center = NULL;
    Edge **end = NULL;
    int num_beginning, num_center, num_end;

    bool found_center = false;

    for (int i = 0; i < num_conds; ++i) {
        if(find_part_containing_edge(sol, problem, edge, move_conditions[i], wait_condition, &center, &num_center)) {
            found_center = true;
            break;
        }
    }

    if (!found_center) {
        *edges = NULL;
        *num_edges = 0;
        return 0;
    }

    Node *center_begin = center[0]->start_node;
    Node *center_end = center[num_center-1]->end_node;
    Station *station = center_begin->station;

    if (center_begin == station->source_node) {
        num_beginning = 0;
        beginning = NULL;
    }
    else {
        if (!find_train_between_nodes(sol, problem,station->source_node, center_begin, num_conds, move_conditions, move_conditions, wait_condition, &beginning, &num_beginning)) {
            free(center);
            *edges = NULL;
            *num_edges = 0;
            return 0;
        }
    }

    if (center_end == station->sink_node) {
        num_end = 0;
    }
    else {
        if (!find_train_between_nodes(sol, problem, center_end, station->sink_node, num_conds, move_conditions, move_conditions, wait_condition, &end, &num_end)) {
            free(center);
            if(beginning) {
                free(beginning);
            }
            *edges = NULL;
            *num_edges = 0;
            return 0;
        }
    }

    *edges = malloc((num_beginning + num_center + num_end) * sizeof(Edge));
    for (int i = 0; i < num_beginning; ++i) {
        (*edges)[i] = beginning[i];
    }
    for (int i = 0; i < num_center; ++i) {
        (*edges)[num_beginning + i] = center[i];
    }
    for (int i = 0; i < num_end; ++i) {
        (*edges)[num_beginning + num_center + i] = end[i];
    }
    *num_edges = num_beginning + num_center + num_end;
    if(beginning) {
        free(beginning);
    }
    if(center) {
        free(center);
    }
    if(end) {
        free(end);
    }
    return 1;
}