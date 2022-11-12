//
// Created by fitli on 29.04.22.
//

#include <stdlib.h>
#include <stdio.h>
#include "operations.h"
#include "heuristics.h"
#include "change_finder.h"
#include "solution_modifier.h"
#include "random.h"

#define LOW_PROB 5
#define HIGH_PROB 95

int destroy_part_waiting(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id);
int remove_trip_dfs(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id, int allow_jumps);
int change_trip_capacity_dfs(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int old_ts_id,
                             int new_ts_id, int old_ts_amount, int new_ts_amount, int allow_jumps);
int insert_trip_dfs(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int allow_overnight, int ts_id);


void oper_add_train_dfs(Solution *sol, Problem *problem, int station_id, int ts_id) {
    Station *station = &problem->stations[station_id];

    insert_trip_dfs(sol, problem, station->source_node->id, station->sink_node->id, 1, ts_id);
}

void oper_add_train_with_edge_dfs(Solution *sol, Problem *problem, int edge_id, int ts_id) {
    Edge *edge = &problem->edges[edge_id];

    if(edge->start_node == NULL || edge->end_node == NULL) {
        return;
    }

    if(insert_trip_dfs(sol, problem, edge->end_node->id, edge->start_node->id, 1, ts_id)) {
        add_trainset_to_edge(sol, problem, &problem->trainset_types[ts_id], edge);
    }
}

int insert_trip_dfs(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int allow_overnight, int ts_id) {
    Trainset *ts = &problem->trainset_types[ts_id];

    Edge **edges;
    int num_edges;
    int result = 0;

    int num_probs = 3;
    int probs[num_probs];
    EdgeCondition *prob_conds[num_probs];

    prob_conds[0] = create_edge_condition(&edge_needs_more_capacity, NULL, NULL);
    probs[0] = HIGH_PROB;
    int max_cap_adata[2] = {ts->seats, problem->max_cap};
    prob_conds[1] = create_edge_condition(&edge_has_more_seats_than, max_cap_adata, NULL);
    probs[1] = LOW_PROB;
    int can_have_ts = problem->max_len - 1;
    prob_conds[2] = create_edge_condition(&edge_has_more_ts_than, &can_have_ts, NULL);
    probs[2] = LOW_PROB;

    if(find_trip_randomized_dfs(problem, sol, &problem->nodes[start_node_id], &problem->nodes[end_node_id],
                                NULL, NULL, allow_overnight, num_probs, prob_conds, probs, &edges, &num_edges)) {
        add_train_array(sol, problem, &problem->trainset_types[ts_id], edges, num_edges);

        free(edges);

        result = num_edges;
    }
    return result;
}

void oper_change_train_capacity_dfs(Solution *sol, Problem *problem, int station_id, int old_ts_id, int new_ts_id, int old_ts_amount, int new_ts_amount) {
    Station *station = &problem->stations[station_id];
    change_trip_capacity_dfs(sol, problem, station->source_node->id, station->sink_node->id,
                             old_ts_id, new_ts_id, old_ts_amount, new_ts_amount, 1);
}

int oper_change_train_capacity_dfs_with_edge(Solution *sol, Problem *problem, int old_ts_id, int new_ts_id, int old_ts_amount, int new_ts_amount, int edge_id) {
    if(sol->edge_solution[edge_id].num_trainsets[old_ts_id] < old_ts_amount) {
        return 0;
    }
    Edge *edge = &problem->edges[edge_id];
    int result = change_trip_capacity_dfs(sol, problem, edge->end_node->id, edge->start_node->id,
                                          old_ts_id, new_ts_id, old_ts_amount, new_ts_amount, 1);

    if (result) {
        for (int i = 0; i < old_ts_amount; ++i) {
            remove_trainset_from_edge(sol, problem, &problem->trainset_types[old_ts_id], edge);
        }
        for (int i = 0; i < old_ts_amount; ++i) {
            add_trainset_to_edge(sol, problem, &problem->trainset_types[new_ts_id], edge);
        }
    }
    return result;
}

int change_trip_capacity_dfs(Solution *sol, Problem *problem, int start_node_id, int end_node_id,
                             int old_ts_id, int new_ts_id, int old_ts_amount, int new_ts_amount, int allow_jumps) {
    Edge **edges = NULL;
    int num_edges = 0;
    int result = 0;

    int capacity_diff = problem->trainset_types[new_ts_id].seats * new_ts_amount - problem->trainset_types[old_ts_id].seats * old_ts_amount;

    int a_data[2];
    a_data[0] = old_ts_id;
    a_data[1] = old_ts_amount;

    EdgeCondition *has_ts_cond = create_edge_condition(&edge_has_trainset, a_data, NULL);

    int num_probs = 3;
    int probs[num_probs];
    EdgeCondition *prob_conds[num_probs];

    int max_cap_adata[2] = {capacity_diff, problem->max_cap};
    prob_conds[0] = create_edge_condition(&edge_has_more_seats_than, &max_cap_adata, NULL);
    probs[0] = LOW_PROB;
    prob_conds[1] = create_edge_condition(&edge_needs_more_capacity, &capacity_diff, NULL);
    probs[1] = LOW_PROB;
    prob_conds[3] = create_edge_condition(&edge_enough_capacity, &capacity_diff, NULL);
    probs[3] = HIGH_PROB;

    if(find_trip_randomized_dfs(problem, sol, &problem->nodes[start_node_id], &problem->nodes[end_node_id],
                                has_ts_cond, has_ts_cond, allow_jumps, 0, NULL, NULL, &edges, &num_edges)) {
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

int oper_remove_train_dfs(Solution *sol, Problem *problem, int station_id, int ts_id) {
    Station *station = &problem->stations[station_id];
    return remove_trip_dfs(sol, problem, station->source_node->id, station->sink_node->id, ts_id, 1);
}

int oper_remove_train_with_edge_dfs(Solution *sol, Problem *problem, int edge_id, int ts_id) {
    if(sol->edge_solution[edge_id].num_trainsets[ts_id] == 0) {
        return 0;
    }
    Edge *edge = &problem->edges[edge_id];
    int result = remove_trip_dfs(sol, problem, edge->end_node->id, edge->start_node->id, ts_id, 1);
    if (result) {
        remove_trainset_from_edge(sol, problem, &problem->trainset_types[ts_id], edge);
    }
    return result;
}

int remove_trip_dfs(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id, int allow_jumps) {
    Edge **edges = NULL;
    int num_edges = 0;

    int capacity_diff = -1 * problem->trainset_types[ts_id].seats;

    int a_data[2] = {ts_id, 1};

    EdgeCondition *has_ts_cond = create_edge_condition(&edge_has_trainset, a_data, NULL);

    int num_probs = 3;
    int probs[num_probs];
    EdgeCondition *prob_conds[num_probs];

    prob_conds[0] = create_edge_condition(&edge_needs_more_capacity, &capacity_diff, NULL);
    probs[0] = LOW_PROB;
    int max_cap_adata[2] = {0, problem->max_cap};
    prob_conds[1] = create_edge_condition(&edge_has_more_ts_than, max_cap_adata, NULL);
    probs[1] = HIGH_PROB;
    prob_conds[2] = create_edge_condition(&edge_has_more_ts_than, &problem->num_trainset_types, NULL);
    probs[2] = HIGH_PROB;

    int result = find_trip_randomized_dfs(problem, sol, &problem->nodes[start_node_id], &problem->nodes[end_node_id],
                                      has_ts_cond, has_ts_cond, allow_jumps, 0, NULL, NULL, &edges, &num_edges);
    if(result) {
        remove_train_array(sol, problem, &problem->trainset_types[ts_id], edges, num_edges);
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

    EdgeCondition *has_ts_cond = create_edge_condition(&edge_has_trainset, a_data, NULL);

    if (find_waiting_between_nodes(sol, problem, &problem->nodes[start_node_id], &problem->nodes[end_node_id],
                                   has_ts_cond, &edges, &num_edges)) {
        remove_train_array(sol, problem, &problem->trainset_types[ts_id], edges, num_edges);
        result = 1;
    }

    free_edge_conditions(has_ts_cond);
    if(edges != NULL)
        free(edges);

    return result;
}

void oper_reschedule_dfs(Solution *sol, Problem *problem, int start_node_id, int trip_len, int ts_id) {
    Edge **edges = NULL;
    int num_edges = 0;

    int a_data[2] = {ts_id, 1};
    int capacity_diff = -1 * problem->trainset_types[ts_id].seats;

    EdgeCondition *has_ts_cond = create_edge_condition(&edge_has_trainset, a_data, NULL);

    int num_probs = 3;
    int probs[num_probs];
    EdgeCondition *prob_conds[num_probs];

    prob_conds[0] = create_edge_condition(&edge_needs_more_capacity, &capacity_diff, NULL);
    probs[0] = LOW_PROB;
    int max_cap_adata[2] = {0, problem->max_cap};
    prob_conds[1] = create_edge_condition(&edge_has_more_ts_than, max_cap_adata, NULL);
    probs[1] = HIGH_PROB;
    prob_conds[2] = create_edge_condition(&edge_has_more_ts_than, &problem->num_trainset_types, NULL);
    probs[2] = HIGH_PROB;

    Node *start_node = &problem->nodes[start_node_id];

    if (!find_random_trip_from(problem, sol, start_node, trip_len, has_ts_cond, has_ts_cond, 0, num_probs, prob_conds, probs, &edges, &num_edges)) {
        return;
    }

    remove_train_array(sol, problem, &problem->trainset_types[ts_id], edges, num_edges);

    Node *end_node = edges[num_edges - 1]->end_node;

    insert_trip_dfs(sol, problem, start_node_id, end_node->id, 0, ts_id);

}

void oper_reschedule_dfs_from_to(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id) {
    if(remove_trip_dfs(sol, problem, start_node_id, end_node_id, ts_id, 0))
        insert_trip_dfs(sol, problem, start_node_id, end_node_id, 0, ts_id);
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
    char *operation_name;
    switch(r) {
        case 0:
            operation_name = "add edge dfs";
            int edge = roulette_wheel(sol->cap_can_add, problem->num_edges);
            oper_add_train_with_edge_dfs(sol, problem, edge, rand_ts);
            break;
        case 1:
            operation_name = "remove waiting train";
            oper_remove_waiting_train(sol, problem, rand_st, rand_ts);
            break;
        case 2:
            operation_name = "remove dfs edge";
            edge = roulette_wheel(sol->cap_can_remove, problem->num_edges);
            int ts = 0;
            if(sol->edge_solution[edge].num_trainsets[0] == 0 || rand() % 2 == 0)
                ts = 1;
            oper_remove_train_with_edge_dfs(sol, problem, edge, ts);
            break;
        case 3:
            operation_name = "change dfs";
            int ts1 = 0;
            int ts2 = 1;
            if(sol->edge_solution[rand_edge].num_trainsets[0] == 0 || rand() % 2 == 0) {
                ts1 = 1;
                ts2 = 0;
            }
            oper_change_train_capacity_dfs(sol, problem, rand_st, ts1, ts2, 1, 1);
            break;
        case 4:
            operation_name = "change dfs 2:1";
            ts1 = 0;
            if(sol->edge_solution[rand_edge].num_trainsets[0] < 2 || rand() % 2 == 0) {
                ts1 = 1;
            }
            oper_change_train_capacity_dfs(sol, problem, rand_st, ts1, rand_ts2, 2, 1);
            break;
        case 5:
            operation_name = "change dfs 1:2";
            ts1 = 0;
            if(sol->edge_solution[rand_edge].num_trainsets[0] == 0 || rand() % 2 == 0) {
                ts1 = 1;
            }
            oper_change_train_capacity_dfs(sol, problem, rand_st, ts1, rand_ts2, 1, 2);
            break;
        case 6:
            operation_name = "reschedule_from_to";
            oper_reschedule_dfs_from_to(sol, problem, rand_start_node, rand_end_node, rand_ts);
            break;
        default:
            operation_name = "reschedule dfs";
            oper_reschedule_dfs(sol, problem, rand_start_node, 2 + rand() % 20, rand_ts);
    }
    return r;
}