//
// Created by fitli on 27.04.22.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "change_finder.h"
#include "heuristics.h"

bool go_to_subcon_first(Solution *sol, Edge *subcon,
                         int num_prob_conditions, EdgeCondition **prob_conditions, const int *probabilities) {
    if(subcon == NULL) {
        return false;
    }
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
 * @param allow_overnight if 1, it is allowed to jump between evening and morning in the same station
 * @param num_prob_conditions TODO
 * @param prob_conditions TODO
 * @param probabilities TODO
 * @param[out] edges dynamically alocated array with edges in the trip.
 * @param[out] num_edges number of edges in `edges`
 * @return if the resulting trip contains at leadst 1 SOURCE edge, then the number of SOURCE edges in the result.
 *         Otherwise 1 if a trip was found and 0 if not.
 */
int find_trip_randomized_dfs(Problem * problem, Solution *sol, Node *start_node, Node *end_node,
                             EdgeCondition *wait_condition, EdgeCondition *move_condition, int allow_overnight,
                             int num_prob_conditions, EdgeCondition **prob_conditions, const int *probabilities,
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

        if(used_wait[node->id] ||
        (!used_subcon[node->id] && go_to_subcon_first(sol, node->out_subcon, num_prob_conditions, prob_conditions, probabilities))) {
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
            } else if (node->out_waiting == NULL && allow_overnight == 1) { // jump into the next day
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


int find_random_trip_from(Problem * problem, Solution *sol, Node *start_node, int trip_len,
                             EdgeCondition *wait_condition, EdgeCondition *move_condition, int allow_overnight,
                             int num_prob_conditions, EdgeCondition **prob_conditions, const int *probabilities,
                             Edge ***edges, int *num_edges) {
    Edge *buffer[problem->num_nodes];
    Node *node = start_node;
    int buffer_size = 0;

    while (buffer_size < trip_len)  {
        if(node->out_subcon &&
                eval_edge_condition(move_condition, node->out_subcon, &sol->edge_solution[node->out_subcon->id]) &&
                (!eval_edge_condition(wait_condition, node->out_waiting, &sol->edge_solution[node->out_waiting->id]) ||
                 go_to_subcon_first(sol, node->out_subcon, num_prob_conditions, prob_conditions, probabilities))) {
            buffer[buffer_size] = node->out_subcon;
            buffer_size++;
            node = node->out_subcon->end_node;
        } else if(node->out_waiting && eval_edge_condition(wait_condition, node->out_waiting, &sol->edge_solution[node->out_waiting->id])) {
            buffer[buffer_size] = node->out_waiting;
            buffer_size++;
            node = node->out_waiting->end_node;
        } else if (node->out_waiting == NULL) { // sink node
            if (allow_overnight == 1) {
                node = node->station->source_node;
            }
            else {
                break;
            }
        }
        else {
            break;
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

int find_waiting_between_nodes(Solution *sol, const Problem *problem, const Node *node_front, const Node *node_back,
                            EdgeCondition *condition, Edge ***edges, int *num_edges) {
    const Node *n = node_front;
    Edge *edge_buffer[problem->num_edges];
    int buff_size = 0;
    while(n != node_back) {
        if(!n->out_waiting || !eval_edge_condition(condition, n->out_waiting, &sol->edge_solution[n->out_waiting->id])) {
            *edges = NULL;
            *num_edges = 0;
            return 0;
        }
        edge_buffer[buff_size] = n->out_waiting;
        buff_size++;
        n = n->out_waiting->end_node;
    }

    *edges = malloc(buff_size * sizeof(Node *));
    for (int i = 0; i < buff_size; ++i) {
        (*edges)[i] = edge_buffer[i];
    }
    *num_edges = buff_size;
    return buff_size;
}