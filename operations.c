//
// Created by fitli on 29.04.22.
//

#include <stdlib.h>
#include "operations.h"
#include "heuristics.h"
#include "change_finder.h"
#include "solution_modifier.h"

int destroy_part(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id);
int destroy_part_waiting(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id);
void insert_part_later(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id);

void oper_add_train_to_empty(Solution *sol, Problem *problem, int station_id) {
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

    find_train_end_to_end(sol, problem, &problem->stations[station_id], num_conditions, front_conditions,
                          back_conditions, NULL,
                          &edges, &num_edges);
    add_train_array(sol, problem, &problem->trainset_types[1], edges, num_edges);

    free_edge_conditions(cond_empty);
    free_edge_conditions(cond_more_ts);
    free(edges);
}

void oper_add_train_later(Solution *sol, Problem *problem, int station_id, int ts_id) {
    insert_part_later(sol, problem, problem->stations[station_id].source_node->id, problem->stations[station_id].sink_node->id, ts_id);
}

void insert_part_later(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id) {
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

    find_train_between_nodes(sol, problem, &problem->nodes[start_node_id], &problem->nodes[end_node_id], num_conditions, front_conditions,
                          back_conditions, NULL,
                          &edges, &num_edges);
    add_train_array(sol, problem,  &problem->trainset_types[ts_id], edges, num_edges);

    free_edge_conditions(cond_empty);
    free_edge_conditions(cond_more_ts);
    free(edges);
}

void oper_add_train_with_edge(Solution *sol, Problem *problem, int edge_id, int ts_id) {
    Edge **edges;
    int num_edges;

    EdgeCondition *cond_more_ts = create_edge_condition(&edge_needs_more_ts, NULL, NULL);

    int num_conditions = 2;
    EdgeCondition *front_conditions[5];
    front_conditions[0] = cond_more_ts;
    front_conditions[1] = NULL;

    find_train_containing_edge(sol, problem, &problem->edges[edge_id], num_conditions, front_conditions, NULL, &edges, &num_edges);
    add_train_array(sol, problem,  &problem->trainset_types[ts_id], edges, num_edges);

    free_edge_conditions(cond_more_ts);
    free(edges);
}

void oper_insert_part_waiting(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id) {
    Node *start_node = &problem->nodes[start_node_id];
    Node *end_node = &problem->nodes[end_node_id];

    int num_edges = end_node->time - start_node->time;
    Edge *edges[num_edges];

    Node *node = start_node;
    for (int i = 0; i < num_edges; ++i) {
        edges[i] = node->out_waiting;
        node = node->out_waiting->end_node;
    }

    add_train_array(sol, problem, &problem->trainset_types[ts_id], edges, num_edges);
}

void oper_change_train_capacity(Solution *sol, Problem *problem, int station_id, int old_ts_id, int new_ts_id, int old_ts_amount, int new_ts_amount) {
    Edge **edges = NULL;
    int num_edges = 0;

    int capacity_change = problem->trainset_types[new_ts_id].seats - problem->trainset_types[old_ts_id].seats;

    int a_data[2];
    a_data[0] = old_ts_id;
    a_data[1] = old_ts_amount;

    EdgeCondition *has_ts_cond = create_edge_condition(&edge_has_trainset, &a_data, NULL);
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

    if (find_train_end_to_end(sol, problem, &problem->stations[station_id], num_conditions, front_conditions,
                              back_conditions, has_ts_cond,
                              &edges, &num_edges)) {
        change_train_array(sol, problem, &problem->trainset_types[old_ts_id], &problem->trainset_types[new_ts_id], old_ts_amount, new_ts_amount, edges, num_edges);
    }

    if(edges)
        free(edges);
}

void oper_remove_train(Solution *sol, Problem *problem, int station_id, int ts_id) {
    destroy_part(sol, problem, problem->stations[station_id].source_node->id, problem->stations[station_id].sink_node->id, ts_id);
}

void oper_remove_train_with_edge(Solution *sol, Problem *problem, int edge_id, int ts_id) {
    Edge **edges = NULL;
    int num_edges = 0;

    int capacity_change = -1 * problem->trainset_types[ts_id].seats;

    int a_data[2];
    a_data[0] = ts_id;
    a_data[1] = 1;

    EdgeCondition *has_ts_cond = create_edge_condition(&edge_has_trainset, a_data, NULL);
    EdgeCondition *has_ts_and_big_len_cond = create_edge_condition(&edge_has_more_ts_than, &problem->max_len, has_ts_cond);
    EdgeCondition *has_ts_and_capacity_cond = create_edge_condition(&edge_enough_capacity, &capacity_change, has_ts_cond);

    int num_conditions = 2;
    EdgeCondition *move_conditions[4];
    move_conditions[0] = has_ts_and_big_len_cond;
    move_conditions[1] = has_ts_and_capacity_cond;

    if(find_train_containing_edge(sol, problem, &problem->edges[edge_id],num_conditions, move_conditions, has_ts_cond, &edges, &num_edges)) {
        remove_train_array(sol, problem, &problem->trainset_types[ts_id], edges, num_edges);
    }

    free_edge_conditions(has_ts_and_capacity_cond);
    free(edges);
}

int destroy_part(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id) {
    Edge **edges = NULL;
    int num_edges = 0;
    int result = 0;

    int capacity_change = -1 * problem->trainset_types[ts_id].seats;

    int a_data[2];
    a_data[0] = ts_id;
    a_data[1] = 1;

    EdgeCondition *has_ts_cond = create_edge_condition(&edge_has_trainset, a_data, NULL);
    EdgeCondition *has_ts_and_big_len_cond = create_edge_condition(&edge_has_more_ts_than, &problem->max_len, has_ts_cond);
    EdgeCondition *has_ts_and_capacity_cond = create_edge_condition(&edge_enough_capacity, &capacity_change, has_ts_cond);

    int num_conditions = 2;
    EdgeCondition *front_conditions[4];
    front_conditions[0] = has_ts_and_big_len_cond;
    front_conditions[1] = has_ts_and_capacity_cond;

    EdgeCondition *back_conditions[4];
    back_conditions[0] = has_ts_and_big_len_cond;
    back_conditions[1] = has_ts_and_capacity_cond;

    if (find_train_between_nodes(sol, problem, &problem->nodes[start_node_id], &problem->nodes[end_node_id], num_conditions, front_conditions,
                              back_conditions, has_ts_cond,
                              &edges, &num_edges)) {
        remove_train_array(sol, problem, &problem->trainset_types[ts_id], edges, num_edges);
        result = 1;
    }

    free_edge_conditions(has_ts_and_capacity_cond);
    free(edges);
    return result;
}

void oper_remove_waiting_train(Solution *sol, Problem *problem, int station_id, int ts_id) {
    destroy_part_waiting(sol, problem, problem->stations[station_id].source_node->id, problem->stations[station_id].sink_node->id, ts_id);
}

void oper_reschedule_w_l(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id) {
    if(destroy_part_waiting(sol, problem, start_node_id, end_node_id, ts_id))
        insert_part_later(sol, problem, start_node_id, end_node_id, ts_id);
}

void oper_reschedule_n_l(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id) {
    if(destroy_part(sol, problem, start_node_id, end_node_id, ts_id))
        insert_part_later(sol, problem, start_node_id, end_node_id, ts_id);
}

void oper_reschedule_n_w(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id) {
    if(destroy_part(sol, problem, start_node_id, end_node_id, ts_id))
        oper_insert_part_waiting(sol, problem, start_node_id, end_node_id, ts_id);
}

int destroy_part_waiting(Solution *sol, Problem *problem, int start_node_id, int end_node_id, int ts_id) {
    Edge **edges = NULL;
    int num_edges = 0;
    int result = 0;

    int a_data[2];
    a_data[0] = ts_id;
    a_data[1] = 1;

    EdgeCondition *has_ts_cond = create_edge_condition(&edge_has_trainset, a_data, NULL);
    EdgeCondition *none_cond = create_edge_condition(&edge_none, NULL, NULL);

    int num_conditions = 1;
    EdgeCondition *front_conditions[4];
    front_conditions[0] = none_cond;

    EdgeCondition *back_conditions[4];
    back_conditions[0] = none_cond;

    if (find_train_between_nodes(sol, problem, &problem->nodes[start_node_id], &problem->nodes[end_node_id], num_conditions, front_conditions,
                              back_conditions, has_ts_cond, &edges, &num_edges)) {
        remove_train_array(sol, problem, &problem->trainset_types[ts_id], edges, num_edges);
        result = 1;
    }

    free_edge_conditions(has_ts_cond);
    free_edge_conditions(none_cond);
    if(edges != NULL)
        free(edges);

    return result;
}

void oper_move_edge_back(Solution *sol, Problem *problem, int edge_id, int ts_id) {
    if(problem->edges[edge_id].type != SUBCONNECTION) {
        return;
    }

    if(sol->edge_solution[edge_id].num_trainsets[ts_id] == 0) {
        return;
    }

    int a_data[2];
    a_data[0] = ts_id;
    a_data[1] = 1;

    EdgeCondition *wait_e_cond = create_edge_condition(&edge_has_trainset, a_data, NULL);
    int start_station_id = problem->edges[edge_id].start_node->station->id;
    EdgeCondition *in_e_cond = create_edge_condition(&edge_start_in_station, &start_station_id, NULL);
    int selected_edge_id;
    Edge *out_waiting = problem->edges[edge_id].end_node->out_waiting;
    if(out_waiting->type == SINK_EDGE || !eval_edge_condition(wait_e_cond, out_waiting, &sol->edge_solution[out_waiting->id])) {
        return;
    }
    select_next_in_edge(sol, problem->edges[edge_id].start_node->out_waiting->end_node, in_e_cond,
            wait_e_cond, &selected_edge_id);
    if(selected_edge_id >= 0)
        move_to_other_subcon(sol, problem, &problem->trainset_types[ts_id], &problem->edges[edge_id], &problem->edges[selected_edge_id]);
}

void oper_move_edge_front(Solution *sol, Problem *problem, int edge_id, int ts_id) {
    if(problem->edges[edge_id].type != SUBCONNECTION) {
        return;
    }

    if(sol->edge_solution[edge_id].num_trainsets[ts_id] == 0) {
        return;
    }

    int a_data[2];
    a_data[0] = ts_id;
    a_data[1] = 1;

    EdgeCondition *wait_e_cond = create_edge_condition(&edge_has_trainset, a_data, NULL);
    int end_station_id = problem->edges[edge_id].end_node->station->id;
    EdgeCondition *out_e_cond = create_edge_condition(&edge_ends_in_station, &end_station_id, NULL);
    int selected_edge_id;
    Edge *in_waiting = problem->edges[edge_id].start_node->in_waiting;
    if(in_waiting->type == SOURCE_EDGE || !eval_edge_condition(wait_e_cond, in_waiting, &sol->edge_solution[in_waiting->id])) {
        return;
    }
    select_prev_out_edge(sol, problem->edges[edge_id].start_node->in_waiting->start_node, out_e_cond,
                         wait_e_cond, &selected_edge_id);
    if(selected_edge_id >= 0)
        move_to_other_subcon(sol, problem, &problem->trainset_types[ts_id], &problem->edges[edge_id], &problem->edges[selected_edge_id]);
}