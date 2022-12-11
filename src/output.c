//
// Created by Ivana Krumlová on 16.10.22.
//

#include <stdio.h>
#include "output.h"
#include "test.h"

void print_objective(Solution *sol) {
    printf("best cost: %lld\n", sol->objective);;
}

void print_used_trainsets(Solution *sol, Problem *problem) {
    for (int ts = 0; ts < problem->num_trainset_types; ++ts) {
        printf("num ts of type %d: %d\n", ts, sol->num_trainstes[ts]);
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
    test_consistency(problem, sol);
    test_objective(problem, sol);
    print_objective(sol);
    print_used_trainsets(sol, problem);
    print_soft_constraints(sol, problem);
}

void sol_to_csv(Solution *sol, Problem *problem, const char* filename) {
    FILE *csv = fopen(filename, "w");
    for(int i = 0; i < problem->num_edges; i++) {
        fprintf(csv, "%d", sol->edge_solution[i].num_trainsets[0]);
        for (int j = 1; j < problem->num_trainset_types; ++j) {
            fprintf(csv, ",%d", sol->edge_solution[i].num_trainsets[j]);
        }
        fprintf(csv, "\n");
    }
    fclose(csv);
}