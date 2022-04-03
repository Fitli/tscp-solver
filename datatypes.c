//
// Created by fitli on 10.03.22.
//
#include <stdlib.h>
#include <stdio.h>
#include "datatypes.h"
#include "objective.h"

void destroy_problem(Problem *problem) {
    free(problem->stations);
    free(problem->edges);
    free(problem->nodes);
    free(problem->trainset_types);
}

int empty_solution(Problem *problem, Solution *sol) {
    sol->edge_solution = malloc(problem->num_edges * sizeof(EdgeSolution));
    if (problem->edges == NULL) {
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
    recalculate_objective(sol, problem);
}

void destroy_solution(Problem *problem, Solution *sol) {
    for(int i = 0; i < problem->num_edges; i++) {
        free(sol->edge_solution[i].num_trainsets);
    }
    free(sol->edge_solution);
}
