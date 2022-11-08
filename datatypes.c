//
// Created by fitli on 10.03.22.
//
#include <stdlib.h>
#include <stdio.h>
#include "datatypes.h"
#include "objective.h"

void destroy_problem(Problem *problem) {
    for (int i = 0; i < problem->num_stations; ++i) {
        free(problem->stations[i].node_ids);
    }
    free(problem->stations);
    free(problem->edges);
    free(problem->nodes);
    free(problem->trainset_types);
}

int empty_solution(Problem *problem, Solution *sol) {
    sol->edge_solution = malloc(problem->num_edges * sizeof(EdgeSolution));
    if (sol->edge_solution == NULL) {
        fprintf(stderr, "Allocation error.\n");
        return EXIT_FAILURE;
    }
    for(int i = 0; i < problem->num_edges; i++) {
        sol->edge_solution[i].capacity = 0;
        sol->edge_solution[i].num_trainsets = calloc(problem->num_trainset_types, sizeof(int));
        if (sol->edge_solution[i].num_trainsets == NULL) {
            fprintf(stderr, "Allocation error.\n");
            return EXIT_FAILURE;
        }
    }
    sol->num_trainstes = calloc(problem->num_trainset_types, sizeof(int));
    recalculate_objective(sol, problem);
    return EXIT_SUCCESS;
}

void copy_solution(Problem *problem, Solution *orig_sol, Solution *dest_sol) {
    dest_sol->objective = orig_sol->objective;
    for(int edge_id = 0; edge_id < problem->num_edges; edge_id++) {
        dest_sol->edge_solution[edge_id].capacity = orig_sol->edge_solution[edge_id].capacity;
        for(int ts_id = 0; ts_id < problem->num_trainset_types; ts_id++) {
            dest_sol->edge_solution[edge_id].num_trainsets[ts_id] = orig_sol->edge_solution[edge_id].num_trainsets[ts_id];
        }
    }
    for (int i = 0; i < problem->num_trainset_types; ++i) {
        dest_sol->num_trainstes[i] = orig_sol->num_trainstes[i];
    }
}

void destroy_solution(Problem *problem, Solution *sol) {
    for(int i = 0; i < problem->num_edges; i++) {
        free(sol->edge_solution[i].num_trainsets);
    }
    free(sol->edge_solution);
    free(sol->num_trainstes);
}
