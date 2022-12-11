//
// Created by Ivana Krumlová on 29.04.22.
//

#include <stdlib.h>
#include <stdio.h>
#include "operations.h"
#include "edge_heuristics.h"
#include "change_finder.h"
#include "solution_modifier.h"
#include "random.h"

#define P1 0
#define P2 0.9
#define P3 0.7
#define P4 0.3

int destroy_part_waiting(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id);
int change_trip_capacity(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int old_ts_id,
                         int new_ts_id, int old_ts_amount, int new_ts_amount, int allow_jumps);


int oper_change_train_capacity(Solution *sol, Problem *problem, int edge_id, int old_ts_id, int new_ts_id, int old_ts_amount, int new_ts_amount) {
    if(sol->edge_solution[edge_id].num_trainsets[old_ts_id] < old_ts_amount) {
        return 0;
    }
    Edge *edge = &problem->edges[edge_id];

    if(edge->start_node == NULL || edge->end_node == NULL) {
        return 0;
    }

    int result = change_trip_capacity(sol, problem, edge->end_node->id, edge->start_node->id,
                                      old_ts_id, new_ts_id, old_ts_amount, new_ts_amount, 1);

    if (result) {
        for (int i = 0; i < old_ts_amount; ++i) {
            remove_trainset_from_edge(sol, problem, &problem->trainset_types[old_ts_id], edge);
        }
        for (int i = 0; i < new_ts_amount; ++i) {
            add_trainset_to_edge(sol, problem, &problem->trainset_types[new_ts_id], edge);
        }
    }
    return result;
}

int change_trip_capacity(Solution *sol, Problem *problem, int start_node_id, int end_node_id,
                         int old_ts_id, int new_ts_id, int old_ts_amount, int new_ts_amount, int allow_jumps) {
    Trainset *old_ts = &problem->trainset_types[old_ts_id];
    Trainset *new_ts = &problem->trainset_types[new_ts_id];

    Edge **edges = NULL;
    int num_edges = 0;
    int result = 0;

    int capacity_diff = problem->trainset_types[new_ts_id].seats * new_ts_amount - problem->trainset_types[old_ts_id].seats * old_ts_amount;

    int a_data[2];
    a_data[0] = old_ts_id;
    a_data[1] = old_ts_amount;

    EdgeCondition has_ts_cond = create_edge_condition(&edge_has_trainset, a_data, NULL);

    int num_probs = 0;
    double probs[10];
    EdgeCondition prob_conds[10];

    int old_oper_cost = (old_ts->el_cost + old_ts->fe_cost + old_ts->re_cost - old_ts->abroad_gain) * old_ts_amount;
    int new_oper_cost = (new_ts->el_cost + new_ts->fe_cost + new_ts->re_cost - new_ts->abroad_gain) * new_ts_amount;

    if(capacity_diff < 0) {
        int max_cap_adata[2] = {0, problem->max_cap};
        prob_conds[num_probs] = create_edge_condition(&edge_has_more_seats_than, &max_cap_adata, NULL);
        probs[num_probs] = P2;
        num_probs++;
    }
    if(capacity_diff > 0) {
        prob_conds[num_probs] = create_edge_condition(&edge_needs_more_capacity, NULL, NULL);
        probs[num_probs] = P2;
        num_probs++;
    }
    int max_cap_diff_adata[2] = {capacity_diff, problem->max_cap};
    prob_conds[num_probs] = create_edge_condition(&edge_has_more_seats_than, &max_cap_diff_adata, NULL);
    probs[num_probs] = P1;
    num_probs++;
    prob_conds[num_probs] = create_edge_condition(&edge_needs_more_capacity, &capacity_diff, NULL);
    probs[num_probs] = P1;
    num_probs++;
    if(old_oper_cost > new_oper_cost) {
        prob_conds[num_probs] = create_edge_condition(NULL, NULL, NULL);
        probs[num_probs] = P3;
        num_probs++;
    }
    prob_conds[num_probs] = create_edge_condition(NULL, NULL, NULL);
    probs[num_probs] = P4;
    num_probs++;

    if(find_trip_randomized_dfs(problem, sol, &problem->nodes[start_node_id], &problem->nodes[end_node_id],
                                &has_ts_cond, &has_ts_cond, allow_jumps, num_probs, prob_conds, probs, &edges, &num_edges)) {
        for (int i = 0; i < old_ts_amount; ++i) {
            remove_train_array(sol, problem, &problem->trainset_types[old_ts_id], edges, num_edges);
        }
        for (int i = 0; i < new_ts_amount; ++i) {
            add_train_array(sol, problem, &problem->trainset_types[new_ts_id], edges, num_edges);
        }
        result = 1;
    }

    free(edges);
    return result;
}

int oper_remove_waiting_train(Solution *sol, Problem *problem, int station_id, int ts_id) {
    return destroy_part_waiting(sol, problem, problem->stations[station_id].source_node->id, problem->stations[station_id].sink_node->id, ts_id);
}

int destroy_part_waiting(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id) {
    Edge **edges = NULL;
    int num_edges = 0;
    int result = 0;

    int a_data[2];
    a_data[0] = ts_id;
    a_data[1] = 1;

    EdgeCondition has_ts_cond = create_edge_condition(&edge_has_trainset, a_data, NULL);

    if (find_waiting_between_nodes(sol, problem, &problem->nodes[start_node_id], &problem->nodes[end_node_id],
                                   &has_ts_cond, &edges, &num_edges)) {
        remove_train_array(sol, problem, &problem->trainset_types[ts_id], edges, num_edges);
        result = 1;
    }

    if(edges != NULL)
        free(edges);

    return result;
}

void oper_reschedule(Solution *sol, Problem *problem, int start_node_id, int trip_len, int ts_id) {
    Edge **edges = NULL;
    int num_edges = 0;

    int a_data[2] = {ts_id, 1};
    int capacity_diff = -1 * problem->trainset_types[ts_id].seats;

    EdgeCondition has_ts_cond = create_edge_condition(&edge_has_trainset, a_data, NULL);

    int num_probs = 4;
    double probs[num_probs];
    EdgeCondition prob_conds[num_probs];

    prob_conds[0] = create_edge_condition(&edge_needs_more_capacity, &capacity_diff, NULL);
    probs[0] = P1;
    int max_cap_adata[2] = {0, problem->max_cap};
    prob_conds[1] = create_edge_condition(&edge_has_more_seats_than, max_cap_adata, NULL);
    probs[1] = P2;
    prob_conds[2] = create_edge_condition(&edge_has_more_ts_than, &problem->max_len, NULL);
    probs[2] = P2;
    prob_conds[3] = create_edge_condition(NULL, NULL, NULL);
    probs[3] = P4;

    Node *start_node = &problem->nodes[start_node_id];

    if (!find_random_trip_from(problem, sol, start_node, trip_len, &has_ts_cond, &has_ts_cond, 0, num_probs, prob_conds, probs, &edges, &num_edges)) {
        return;
    }

    remove_train_array(sol, problem, &problem->trainset_types[ts_id], edges, num_edges);

    Node *end_node = edges[num_edges - 1]->end_node;

    change_trip_capacity(sol, problem, start_node_id, end_node->id, ts_id, ts_id, 0, 1, 0);

}

void oper_reschedule_from_to(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id) {
    if(change_trip_capacity(sol, problem, start_node_id, end_node_id, ts_id, ts_id, 1, 0, 0))
        change_trip_capacity(sol, problem, start_node_id, end_node_id, ts_id, ts_id, 0, 1, 0);
}

int select_operation(Problem *problem, Solution *sol, int *weights) {
    int r;
    if(weights)
        r = roulette_wheel(weights, NUM_OPERATIONS);
    else
        r = rand() % NUM_OPERATIONS;
    int rand_st = rand() % problem->num_stations;
    int rand_edge = rand() % problem->num_edges;
    int rand_start_node_index = rand() % (problem->stations[rand_st].num_nodes - 1);
    int rand_start_node = problem->stations[rand_st].node_ids[rand_start_node_index];
    int rand_end_node_index = rand_start_node_index + 1 + rand() % (problem->stations[rand_st].num_nodes - rand_start_node_index - 1);
    int rand_end_node = problem->stations[rand_st].node_ids[rand_end_node_index];
    int rand_ts = rand() % problem->num_trainset_types;
    int rand_ts2 = rand() % problem->num_trainset_types;
    int ts, ts1, ts2;
    switch(r) {
        case 0:
            oper_change_train_capacity(sol, problem, rand_edge, rand_ts, rand_ts, 0, 1);
            break;
        case 1:
            oper_remove_waiting_train(sol, problem, rand_st, rand_ts);
            break;
        case 2:
            ts = 0;
            if(sol->edge_solution[rand_edge].num_trainsets[0] == 0 || rand() % 2 == 0)
                ts = 1;
            oper_change_train_capacity(sol, problem, rand_edge, ts, ts, 1, 0);
            break;
        case 3:
            ts1 = 0;
            ts2 = 1;
            if(sol->edge_solution[rand_edge].num_trainsets[0] == 0 || rand() % 2 == 0) {
                ts1 = 1;
                ts2 = 0;
            }
            oper_change_train_capacity(sol, problem, rand_edge, ts1, ts2, 1, 1);
            break;
        case 4:
            ts1 = 0;
            if(sol->edge_solution[rand_edge].num_trainsets[0] < 2 || rand() % 2 == 0) {
                ts1 = 1;
            }
            oper_change_train_capacity(sol, problem, rand_edge, ts1, rand_ts2, 2, 1);
            break;
        case 5:
            ts1 = 0;
            if(sol->edge_solution[rand_edge].num_trainsets[0] == 0 || rand() % 2 == 0) {
                ts1 = 1;
            }
            oper_change_train_capacity(sol, problem, rand_edge, ts1, rand_ts2, 1, 2);
            break;
        case 6:
            oper_reschedule_from_to(sol, problem, rand_start_node, rand_end_node, rand_ts);
            break;
        default:
            oper_reschedule(sol, problem, rand_start_node, 2 + rand() % 20, rand_ts);
    }
    return r;
}