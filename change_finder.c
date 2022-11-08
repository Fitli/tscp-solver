//
// Created by fitli on 27.04.22.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "change_finder.h"
#include "heuristics.h"

void fastest_trip_from_node_to_stations(Solution *sol, const Problem *problem, const Node *start, EdgeCondition *move_condition,
                                        EdgeCondition *wait_condition, Edge ***edges, int *nums_edges);

/*
 * Finds the first and the last SUBCONNECTION edges that can be chosen on the path from node_front to node_back
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

int find_trip_end_to_end(Solution *sol, const Problem *problem, const Station *station, int num_conds,
                         EdgeCondition **front_conditions, EdgeCondition **back_conditions, EdgeCondition *wait_condition,
                         Edge ***edges, int *num_edges) {

    Node *node_front = station->source_node;
    Node *node_back = station->sink_node;

    int res = find_trip_between_nodes(sol, problem, node_front, node_back, num_conds, front_conditions, back_conditions,
                                      wait_condition, edges, num_edges);

    return res;
}

//greedy finding of a journey between two nodes of the same station
int find_trip_between_nodes(Solution *sol, const Problem *problem, const Node *node_front, const Node *node_back, int num_conds,
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
            fastest_trip_from_node_to_stations(sol, problem, node_front, front_conditions[i], wait_condition,
                                               station_edges, station_num_edges);
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

void fastest_trip_from_node_to_stations(Solution *sol, const Problem *problem, const Node *start, EdgeCondition *move_condition,
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
                    const Node *node = queue[i].final_node;
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

void latest_trip_from_stations_to_node(Solution *sol, const Problem *problem, const Node *end, EdgeCondition *move_condition,
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
                    const Node *node = queue[i].final_node;
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
bool find_trip_containing_edge(Solution *sol, const Problem *problem, const Edge *edge, EdgeCondition *move_condition,
                               EdgeCondition *wait_condition, Edge ***edges, int *num_edges) {
    if(edge->start_node == NULL || edge->end_node == NULL) { // workaround for unused edges
        return false;
    }

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
    fastest_trip_from_node_to_stations(sol, problem, edge->end_node, move_condition, wait_condition, station_edges,
                                       station_num_edges);
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

    latest_trip_from_stations_to_node(sol, problem, edge->start_node, move_condition, wait_condition, station_edges,
                                      station_num_edges);
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
        if(find_trip_containing_edge(sol, problem, edge, move_conditions[i], wait_condition, &center, &num_center)) {
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
        if (!find_trip_between_nodes(sol, problem, station->source_node, center_begin, num_conds, move_conditions,
                                     move_conditions, wait_condition, &beginning, &num_beginning)) {
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
        if (!find_trip_between_nodes(sol, problem, center_end, station->sink_node, num_conds, move_conditions,
                                     move_conditions, wait_condition, &end, &num_end)) {
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

bool go_to_subcon_first(Problem *problem, Solution *sol, Edge *subcon,
                         int num_prob_conditions, EdgeCondition **prob_conditions, const int *probabilities) {
    int prob = 50;
    if(num_prob_conditions && prob_conditions && probabilities) {
        for (int i = 0; i <num_prob_conditions; ++i) {
            if(eval_edge_condition(prob_conditions[i], subcon, &sol->edge_solution[subcon->id])) {
                prob = probabilities[i];
            }
        }
    }
    return rand() % 100 < prob;
}

/**
 * Finds a sequence of edges connecting `start_node` and `end_node`. Uses DFS with random order of searching branches
 *
 * @param problem
 * @param sol
 * @param start_node
 * @param end_node
 * @param wait_condition must hold for all WAITING edges in the trip
 * @param move_condition must hold for all SUBCONNECTION edges in the trip
 * @param allow_jumps if 1, it is allowed to jump between evening and morning in the same station
 * @param[out] edges dynamically alocated array with edges in the trip.
 * @param[out] num_edges number of edges in `edges`
 * @return if the resulting trip contains at leadst 1 SOURCE edge, then the number of SOURCE edges in the result.
 *         Otherwise 1 if a trip was found and 0 if not.
 */
int find_trip_randomized_dfs(Problem * problem, Solution *sol, Node *start_node, Node *end_node,
                             EdgeCondition *wait_condition, EdgeCondition *move_condition, int allow_jumps,
                             Edge ***edges, int *num_edges) {
    bool used_wait[problem->num_nodes];
    bool used_subcon[problem->num_nodes];
    for (int i = 0; i < problem->num_nodes; ++i) {
        used_wait[i] = false;
        used_subcon[i] = false;
        if(problem->nodes[i].out_subcon == NULL) {
            used_subcon[i] = true;
        }
    }

    Edge *buffer[problem->num_nodes];
    Node *node = start_node;
    int buffer_size = 0;

    while (true)  {
        if(node == end_node) {
            break;
        }

        if(used_wait[node->id] && used_subcon[node->id]) {
            if(buffer_size == 0) {
                break;
            }
            buffer_size--;
            node = buffer[buffer_size]->start_node;
            continue;
        }

        if(used_wait[node->id] || (!used_subcon[node->id] && rand()%2 == 0)) {
            used_subcon[node->id] = true;
            if(node->out_subcon && eval_edge_condition(move_condition, node->out_subcon, &sol->edge_solution[node->out_subcon->id])) {
                buffer[buffer_size] = node->out_subcon;
                buffer_size++;
                node = node->out_subcon->end_node;
            }
        } else {
            used_wait[node->id] = true;
            if(node->out_waiting && eval_edge_condition(wait_condition, node->out_waiting, &sol->edge_solution[node->out_waiting->id])) {
                buffer[buffer_size] = node->out_waiting;
                buffer_size++;
                node = node->out_waiting->end_node;
            } else if (node->out_waiting == NULL && allow_jumps == 1) { // jump into the next day
                node = node->station->source_node;
            }
        }
    }

    int result = 0;
    if(buffer_size > 0) {
        *edges = malloc(buffer_size * sizeof(Edge*));
        *num_edges = buffer_size;
        for (int i = 0; i < buffer_size; ++i) {
            (*edges)[i] = buffer[i];
            if(buffer[i]->type == SOURCE_EDGE) {
                result++;
            }
        }
        if(result == 0) {
            result = 1; // case when the trip contains no source edges, but some trip was found.
        }
    }
    return result;
}