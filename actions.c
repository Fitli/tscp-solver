//
// Created by fitli on 29.04.22.
//

#include <stdlib.h>
#include "actions.h"
#include "heuristics.h"
#include "change_finder.h"
#include "solution_modifier.h"

void act_add_train_to_empty(Solution *sol, Problem *problem, int station_id) {
    Edge **edges;
    int num_edges;

    EdgeCondition *cond_empty = create_edge_condition(&edge_is_empty, NULL, NULL);
    EdgeCondition *cond_more_ts = create_edge_condition(&edge_needs_more_ts, NULL, NULL);

    int num_conditions = 5;
    EdgeCondition *front_conditions[5];
    front_conditions[0] = cond_empty;
    front_conditions[1] = cond_empty;
    front_conditions[2] = cond_more_ts;
    front_conditions[3] = cond_empty;
    front_conditions[4] = NULL;

    EdgeCondition *back_conditions[5];
    back_conditions[0] = cond_empty;
    back_conditions[1] = cond_more_ts;
    back_conditions[2] = cond_empty;
    back_conditions[3] = NULL;
    back_conditions[4] = cond_empty;

    find_train_two_side(sol, problem, &problem->stations[station_id], num_conditions, front_conditions, back_conditions, NULL,
                        &edges, &num_edges);
    add_train_array(sol, &problem->trainset_types[0], edges, num_edges);

    free_edge_conditions(cond_empty);
    free_edge_conditions(cond_more_ts);
    free(edges);
}

void act_add_train_later(Solution *sol, Problem *problem, int station_id) {
    Edge **edges;
    int num_edges;

    EdgeCondition *cond_empty = create_edge_condition(&edge_is_empty, NULL, NULL);
    EdgeCondition *cond_more_ts = create_edge_condition(&edge_needs_more_ts, NULL, NULL);

    int num_conditions = 4;
    EdgeCondition *front_conditions[5];
    front_conditions[0] = cond_more_ts;
    front_conditions[1] = NULL;
    front_conditions[2] = cond_more_ts;
    front_conditions[3] = NULL;

    EdgeCondition *back_conditions[5];
    back_conditions[0] = cond_more_ts;
    back_conditions[1] = cond_more_ts;
    back_conditions[2] = NULL;
    back_conditions[3] = NULL;

    find_train_two_side(sol, problem, &problem->stations[station_id], num_conditions, front_conditions, back_conditions, NULL,
                        &edges, &num_edges);
    add_train_array(sol, &problem->trainset_types[0], edges, num_edges);

    free_edge_conditions(cond_empty);
    free_edge_conditions(cond_more_ts);
    free(edges);
}

void act_change_train_capacity(Solution *sol, Problem *problem, int station_id, int old_ts_id, int new_ts_id) {
    Edge **edges;
    int num_edges;

    int capacity_change = problem->trainset_types[new_ts_id].seats - problem->trainset_types[old_ts_id].seats;

    EdgeCondition *has_ts_cond = create_edge_condition(&edge_has_trainset, &old_ts_id, NULL);
    EdgeCondition *has_ts_and_capacity_cond = create_edge_condition(&edge_enough_capacity, &capacity_change, has_ts_cond);
    EdgeCondition *has_ts_and_needs_and_capacity_cond = create_edge_condition(&edge_needs_more_ts, NULL, has_ts_and_capacity_cond);

    int num_conditions = 7;
    EdgeCondition *front_conditions[7];
    front_conditions[0] = has_ts_and_needs_and_capacity_cond;
    front_conditions[1] = has_ts_and_needs_and_capacity_cond;
    front_conditions[2] = has_ts_cond;
    front_conditions[3] = has_ts_and_capacity_cond;
    front_conditions[4] = has_ts_and_capacity_cond;
    front_conditions[5] = has_ts_cond;
    front_conditions[6] = has_ts_cond;

    EdgeCondition *back_conditions[7];
    back_conditions[0] = has_ts_and_needs_and_capacity_cond;
    back_conditions[1] = has_ts_cond;
    back_conditions[2] = has_ts_and_needs_and_capacity_cond;
    back_conditions[3] = has_ts_and_capacity_cond;
    back_conditions[4] = has_ts_cond;
    back_conditions[5] = has_ts_and_capacity_cond;
    back_conditions[6] = has_ts_cond;

    if (find_train_two_side(sol, problem, &problem->stations[station_id], num_conditions, front_conditions, back_conditions, has_ts_cond,
                        &edges, &num_edges)) {
        change_train_array(sol, &problem->trainset_types[old_ts_id], &problem->trainset_types[new_ts_id], edges, num_edges);
    }

    free(edges);
}

void act_remove_train(Solution *sol, Problem *problem, int station_id, int ts_id) {
    Edge **edges;
    int num_edges;

    int capacity_change = -1 * problem->trainset_types[ts_id].seats;

    EdgeCondition *has_ts_cond = create_edge_condition(&edge_has_trainset, &ts_id, NULL);
    EdgeCondition *has_ts_and_capacity_cond = create_edge_condition(&edge_enough_capacity, &capacity_change, has_ts_cond);

    int num_conditions = 1;
    EdgeCondition *front_conditions[4];
    front_conditions[0] = has_ts_and_capacity_cond;

    EdgeCondition *back_conditions[4];
    back_conditions[0] = has_ts_and_capacity_cond;

    if (find_train_two_side(sol, problem, &problem->stations[station_id], num_conditions, front_conditions, back_conditions, has_ts_cond,
                        &edges, &num_edges)) {
        remove_train_array(sol, &problem->trainset_types[ts_id], edges, num_edges);
    }

    free_edge_conditions(has_ts_and_capacity_cond);
    free(edges);
}

void act_remove_waiting_train(Solution *sol, Problem *problem, int station_id, int ts_id) {
    Edge **edges;
    int num_edges;

    int capacity_change = -1 * problem->trainset_types[ts_id].seats;

    EdgeCondition *has_ts_cond = create_edge_condition(&edge_has_trainset, &ts_id, NULL);
    EdgeCondition *none_cond = create_edge_condition(&edge_none, NULL, NULL);

    int num_conditions = 1;
    EdgeCondition *front_conditions[4];
    front_conditions[0] = none_cond;

    EdgeCondition *back_conditions[4];
    back_conditions[0] = none_cond;

    if (find_train_two_side(sol, problem, &problem->stations[station_id], num_conditions, front_conditions, back_conditions, has_ts_cond,
                            &edges, &num_edges)) {
        remove_train_array(sol, &problem->trainset_types[ts_id], edges, num_edges);
    }

    free_edge_conditions(has_ts_cond);
    free_edge_conditions(none_cond);
    free(edges);
}

void act_move_edge_back(Solution *sol, Problem *problem, int edge_id, int ts_id) {
    EdgeCondition *wait_e_cond = create_edge_condition(&edge_has_trainset, &ts_id, NULL);
    int end_station_id = problem->edges[edge_id].end_node->station->id;
    EdgeCondition *out_e_cond = create_edge_condition(&edge_ends_in_station, &end_station_id, NULL);
    int selected_edge_id;
    select_next_out_edge(sol, problem->edges[edge_id].start_node->out_waiting->end_node, out_e_cond,
            wait_e_cond, &selected_edge_id);
    if(selected_edge_id >= 0)
        move_to_other_subcon(sol, &problem->trainset_types[ts_id], &problem->edges[edge_id], &problem->edges[selected_edge_id]);
}

void act_move_edge_front(Solution *sol, Problem *problem, int edge_id, int ts_id) {
    EdgeCondition *wait_e_cond = create_edge_condition(&edge_has_trainset, &ts_id, NULL);
    int end_station_id = problem->edges[edge_id].end_node->station->id;
    EdgeCondition *out_e_cond = create_edge_condition(&edge_ends_in_station, &end_station_id, NULL);
    int selected_edge_id;
    select_prev_out_edge(sol, problem->edges[edge_id].start_node->in_waiting->start_node, out_e_cond,
                         wait_e_cond, &selected_edge_id);
    if(selected_edge_id >= 0)
        move_to_other_subcon(sol, &problem->trainset_types[ts_id], &problem->edges[edge_id], &problem->edges[selected_edge_id]);
}