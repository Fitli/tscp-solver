//
// Created by Ivana Krumlová on 27.04.22.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "change_finder.h"
#include "edge_heuristics.h"

/**
 * Randomly determines if the subconnection branch should be explored before the waiting branch in the DFS.
 * If the subconnection meets any of the condition specified in `prob_conditions`, the probability of returning true
 * is the corresponding probability in `probabilities`. Otherwise, it is 50%.
 *
 * @param sol solution
 * @param subcon subconnection outgoing from the current node
 * @param num_prob_conditions length of `prob-conditions` and `probabilities`
 * @param prob_conditions If `prob_conditions[i]` is met, the probability of returning true is `probabilities[i]`
 * @param probabilities If `prob_conditions[i]` is met, the probability of returning true is `probabilities[i]`
 * @return
 */
bool go_to_subcon_first(Solution *sol, Edge *subcon,
                         int num_prob_conditions, EdgeCondition *prob_conditions, const double *probabilities) {
    if(subcon == NULL) {
        return false;
    }
    double prob = 0.4;
    if(num_prob_conditions && prob_conditions && probabilities) {
        for (int i = 0; i <num_prob_conditions; ++i) {
            if(eval_edge_condition(&prob_conditions[i], subcon, &sol->edge_solution[subcon->id])) {
                prob = probabilities[i];
                break;
            }
        }
    }
    return rand() < prob * RAND_MAX;
}


int find_trip_randomized_dfs(Problem * problem, Solution *sol, Node *start_node, Node *end_node,
                             EdgeCondition *wait_condition, EdgeCondition *move_condition, int allow_overnight,
                             int num_prob_conditions, EdgeCondition *prob_conditions, const double *probabilities,
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
                             int num_prob_conditions, EdgeCondition *prob_conditions, const double *probabilities,
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


int find_waiting_between_nodes(Solution *sol, const Problem *problem, const Node *start_node, const Node *end_node,
                               EdgeCondition *condition, Edge ***edges, int *num_edges) {
    const Node *n = start_node;
    Edge *edge_buffer[problem->num_edges];
    int buff_size = 0;
    while(n != end_node) {
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