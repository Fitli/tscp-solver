//
// Created by fitli on 03.11.22.
//

#include <stdbool.h>
#include <stdio.h>
#include "min_flow.h"
#include "datatypes.h"
#include "solution_modifier.h"

typedef enum direction Direction;

enum direction {
    FORWARD,
    BACKWARD
};

int get_end_node(Edge *e) {
    if(e->type == SINK_EDGE) {
        return e->end_node->station->source_node->id; // sink node goes directly to source -> circulation
    }
    else {
        return e->end_node->id;
    }
}

void augment_feas(Problem *problem, const int *edge_q, int edge_q_len, int to_augment, int startpoint, int *next_e, int *solution) {
    for (int i = 0; i < edge_q_len; ++i) {
        if(edge_q[i] == 19) {
            printf("E");
        }
        solution[edge_q[i]] += to_augment;
        Edge *edge = &problem->edges[edge_q[i]];
        if(next_e[edge->start_node->id] == -1) {
            next_e[edge->start_node->id] = edge->id;
        }
    }
    Edge *last_edge = &problem->edges[edge_q[edge_q_len - 1]];
    int last_node = get_end_node(last_edge);
    while(last_node != startpoint) {
        int e = next_e[last_node];
        solution[e] += to_augment;
        last_edge = &problem->edges[e];
        last_node = get_end_node(last_edge);
    }
}

void feasible_dfs(Problem *problem, const int *low_bounds, int *feasible, int startpoint) {
    int num_e = problem->num_edges;
    int num_n = problem->num_nodes;

    int n = startpoint;
    int edge_q[num_e];
    int edge_q_len = 0;
    int to_augment = 0;
    Direction dir = BACKWARD;

    int visited[num_n];
    int next_e[num_n];
    for (int i = 0; i < num_n; ++i) {
        next_e[i] = -1;
        visited[i] = 0;
    }

    while (n != startpoint || visited[startpoint] < 2) {
        if(dir == FORWARD && (n == startpoint || next_e[n] >= 0)) {
            augment_feas(problem, edge_q, edge_q_len, to_augment, startpoint, next_e, feasible);
            edge_q_len--;
            n = problem->edges[edge_q[edge_q_len]].start_node->id;
            dir = BACKWARD;
            to_augment = 0;
        }
        else if(visited[n] == 0) {
            visited[n]++;
            Edge *e = problem->nodes[n].out_waiting;
            edge_q[edge_q_len] = e->id;
            edge_q_len++;
            n = get_end_node(e);
            dir = FORWARD;
        }
        else if(visited[n] == 1) {
            visited[n]++;
            Edge *e = problem->nodes[n].out_subcon;
            if(e) {
                if(e->id == 19) {
                    printf("a");
                }
                edge_q[edge_q_len] = e->id;
                edge_q_len++;
                if(low_bounds[e->id] > to_augment) {
                    to_augment = low_bounds[e->id];
                }
                n = get_end_node(e);
                dir = FORWARD;
            }
        }
        else if(visited[n] == 2){
            edge_q_len--;
            n = problem->edges[edge_q[edge_q_len]].start_node->id;
            dir = BACKWARD;
        }
    }
    printf("a");
}

void find_feasible(Problem *problem, int *low_bounds, int *feasible) {
    for (int st = 0; st < problem->num_stations; ++st) {
        int startpoint = problem->stations[st].source_node->id;
        feasible_dfs(problem, low_bounds, feasible, startpoint);
    }
}

int available_flow(int bound, int flow, int dir) {
    if(dir == 1) {
        return bound - flow;
    }
    else {
        return flow;
    }
}

bool find_augmenting_from_to(Problem *problem, int *upper_bounds, int *max_flow, int *path, int *dirs, int *path_len, int start, int end) {
    int num_e = problem->num_edges;
    int num_n = problem->num_nodes;

    *path_len = 0;

    bool used_edges[num_e];
    for (int i = 0; i < num_e; ++i) {
        used_edges[i] = false;
    }
    int visited[num_n];
    for (int i = 0; i < num_n; ++i) {
        visited[i] = 0;
    }

    Node *node = &problem->nodes[start];

    while (node->id != start || visited[start] < 4) {
        if(node->id == end) {
            return true;
        }
        else if(visited[node->id] == 0) {
            visited[node->id]++;
            if(node->out_waiting != NULL && !used_edges[node->out_waiting->id]
                    && available_flow(upper_bounds[node->out_waiting->id], max_flow[node->out_waiting->id], 1)) {
                path[*path_len] = node->out_waiting->id;
                dirs[*path_len] = 1;
                used_edges[node->out_waiting->id] = true;
                (*path_len)++;
                node = node->out_waiting->end_node;
            }
            else if(node->out_waiting == NULL) {
                node = node->station->source_node;
            }
        }
        else if(visited[node->id] == 1) {
            visited[node->id]++;
            if(node->out_subcon != NULL && !used_edges[node->out_subcon->id]
                    && available_flow(upper_bounds[node->out_subcon->id], max_flow[node->out_subcon->id], 1)) {
                path[*path_len] = node->out_subcon->id;
                dirs[*path_len] = 1;
                used_edges[node->out_subcon->id] = true;
                (*path_len)++;
                node = node->out_subcon->end_node;
            }
        }
        else if(visited[node->id] == 2) {
            visited[node->id]++;
            if(node->in_waiting != NULL && !used_edges[node->in_waiting->id]
                    && available_flow(upper_bounds[node->in_waiting->id], max_flow[node->in_waiting->id], -1)) {
                path[*path_len] = node->in_waiting->id;
                dirs[*path_len] = -1;
                used_edges[node->in_waiting->id] = true;
                (*path_len)++;
                node = node->in_waiting->start_node;
            }
            else if(node->in_waiting == NULL && *path_len > 0) {
                node = node->station->sink_node;
            }
        }
        else if(visited[node->id] == 3) {
            visited[node->id]++;
            if(node->in_subcon != NULL && !used_edges[node->in_subcon->id]
                    && available_flow(upper_bounds[node->in_subcon->id], max_flow[node->in_subcon->id], -1)) {
                path[*path_len] = node->in_subcon->id;
                dirs[*path_len] = -1;
                used_edges[node->in_subcon->id] = true;
                (*path_len)++;
                node = node->in_subcon->start_node;
            }
        }
        else {
            (*path_len)--;
            Edge * removed_e = &problem->edges[path[*path_len]];
            used_edges[removed_e->id] = false;
            if(dirs[*path_len] == 1) {
                node = removed_e->start_node;
            } else {
                node = removed_e->end_node;
            }
        }
    }

    return false;
}

bool find_augmenting_path(Problem *problem, int *upper_bounds, int *max_flow, int *path, int *dirs, int *path_len) {
    for (int s = 0; s < problem->num_stations; ++s) {
        if(find_augmenting_from_to(problem, upper_bounds, max_flow, path, dirs, path_len,
                                   problem->stations[s].source_node->id, problem->stations[s].sink_node->id)) {
            return true;
        }
    }
    return false;
}

void augment_max_flow(int *upper_bounds, int *flows, int *path, int *dirs, int path_len) {
    int to_augment = available_flow(upper_bounds[path[0]], flows[path[0]], dirs[0]);
    for (int i = 1; i < path_len; ++i) {
        int av_fl_i = available_flow(upper_bounds[path[i]], flows[path[i]], dirs[i]);
        if(av_fl_i < to_augment) {
            to_augment = av_fl_i;
        }
    }

    for (int i = 0; i < path_len; ++i) {
        flows[path[i]] += dirs[i] * to_augment;
    }
}

void find_max_flow(Problem *problem, int *upper_bounds, int *max_flow) {
    int path[problem->num_edges];
    int dirs[problem->num_edges];
    int path_len = 0;
    while(find_augmenting_path(problem, upper_bounds, max_flow, path, dirs, &path_len)) {
        augment_max_flow(upper_bounds, max_flow, path, dirs, path_len);
    }
}

Solution min_flow(Problem *problem, Trainset *ts) {
    int low_bounds[problem->num_edges];
    int feasible[problem->num_edges];
    for (int i = 0; i < problem->num_edges; ++i) {
        low_bounds[i] = problem->edges[i].minimal_capacity / ts->seats;
        if(problem->edges[i].minimal_capacity % ts->seats != 0) {
            low_bounds[i]++;
        }
        feasible[i] = 0;
    }
    find_feasible(problem, low_bounds, feasible);

    int upper_bounds[problem->num_edges];
    int max_flow[problem->num_edges];
    for (int i = 0; i < problem->num_edges; ++i) {
        upper_bounds[i] = feasible[i] - low_bounds[i];
        max_flow[i] = 0;
    }
    find_max_flow(problem, upper_bounds, max_flow);

    int solut[problem->num_edges];
    for (int i = 0; i < problem->num_edges; ++i) {
        solut[i] = feasible[i] - max_flow[i];
    }

    Solution sol;
    empty_solution(problem, &sol);
    for (int i = 0; i < problem->num_edges; ++i) {
        int num_ts = feasible[i] - max_flow[i];
        for (int j = 0; j < num_ts; ++j) {
            add_trainset_to_edge(&sol, problem, ts, &problem->edges[i]);
        }
    }
    return sol;
}