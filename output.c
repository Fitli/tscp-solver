//
// Created by fitli on 16.10.22.
//

#include <stdio.h>
#include "output.h"
#include "objective.h"

void print_used_trainsets(Solution *sol, Problem *problem) {
    int num_ts[problem->num_trainset_types];
    get_num_ts(sol, problem, num_ts);

    for (int ts = 0; ts < problem->num_trainset_types; ++ts) {
        printf("num ts of type %d: %d\n", ts, num_ts[ts]);
    }
}

void print_soft_constraints(Solution *sol, Problem *problem) {
    int n_bad_len_edges = 0;
    int n_bad_capacity_edges = 0;
    for (int e = 0; e < problem->num_edges; ++e) {
        Edge *edge = &problem->edges[e];
        if(edge->type != SUBCONNECTION) {
            continue;
        }
        EdgeSolution *e_sol = &sol->edge_solution[e];
        if(e_sol->capacity < edge->minimal_capacity || e_sol->capacity > problem->max_cap) {
            n_bad_capacity_edges++;
        }
        int len = 0;
        for (int i = 0; i < problem->num_trainset_types; i++) {
            len += e_sol->num_trainsets[i];
        }
        if(len < 1 || len > problem->max_len) {
            n_bad_len_edges++;
        }
    }
    printf("number of edges with bad length: %d\n", n_bad_len_edges);
    printf("number of edges with bad capacity: %d\n", n_bad_capacity_edges);
}

void analyze_solution(Solution *sol, Problem *problem) {
    print_used_trainsets(sol, problem);
    print_soft_constraints(sol, problem);
}