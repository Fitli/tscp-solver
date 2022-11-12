//
// Created by fitli on 31.10.22.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "local_search.h"
#include "operations.h"

// sorting solutions by increasing objective
int cmpfunc (const void * a, const void * b) {
    if (((Solution *) a)->objective > ((Solution *) b)->objective) {
        return 1;
    }
    else {
        return -1;
    }
}

void perturbate(Problem *problem, Solution *solution, double remove_rate) {
    int overall_num_ts = 0;
    for (int i = 0; i < problem->num_trainset_types; ++i) {
        overall_num_ts += solution->num_trainstes[i];
    }

    int to_remove = (int) (overall_num_ts * remove_rate);
    int removed = 0;
    while(removed < to_remove) {
        int rand_st_id = rand() % problem->num_stations;
        int rand_ts_id = rand() % problem->num_trainset_types;
        removed += oper_remove_train_dfs(solution, problem, rand_st_id, rand_ts_id);
    }
}

void local_search(Problem *problem, Solution *sol, int taboo_size, int neighborhood_size, int stop_no_improve, clock_t inittime, FILE *csv_objective, FILE *csv_operations) {

    printf("objective: %lld\n", sol->objective);

    int iteration = 0;
    long long int taboo[taboo_size];
    taboo[1] = sol->objective;
    for (int i = 1; i < taboo_size; ++i) {
        taboo[i] = 0;
    }
    int tabu_pos = 1;
    Solution new_sols [neighborhood_size];
    for(int i = 0; i < neighborhood_size; i++) {
        empty_solution(problem, new_sols + i);
    }
    Solution overall_best;
    empty_solution(problem, &overall_best);
    copy_solution(problem, sol, &overall_best);
    int overall_best_iter = 0;

    while(iteration - overall_best_iter < stop_no_improve) {
        printf("%d\n", iteration);

        int local_counter = 0;
        while (local_counter < neighborhood_size) {
            if(csv_operations) {
                fprintf(csv_operations, "%d,", iteration);
            }

            copy_solution(problem, sol, new_sols + local_counter);
            select_operation(problem, new_sols + local_counter, NULL);

            bool is_tabu = false;
            for (int i = 0; i < taboo_size; ++i) {
                if(new_sols[local_counter].objective == taboo[i]) {
                    is_tabu = true;
                    break;
                }
            }
            if(is_tabu) {
                if(csv_operations) {
                    fprintf(csv_operations, "1,0\n");
                }
                continue;
            }
            if(csv_operations) {
                fprintf(csv_operations, "0,%lld\n", new_sols[local_counter].objective - sol->objective);
            }

            local_counter++;
        }
        qsort(new_sols, neighborhood_size, sizeof(Solution), cmpfunc);

        Solution *selected_solution = &new_sols[0];

        copy_solution(problem, selected_solution, sol);
        taboo[tabu_pos] = sol->objective;
        tabu_pos = (tabu_pos + 1) % taboo_size;
        if(sol->objective < overall_best.objective) {
            copy_solution(problem, sol, &overall_best);
            overall_best_iter = iteration;
        }
        fprintf(csv_objective, "%d,%lld,%f\n", iteration, sol->objective, (double)(clock()-inittime)/(double)CLOCKS_PER_SEC);
        iteration++;
    }

    destroy_solution(problem, &overall_best);
    for (int i = 0; i < neighborhood_size; ++i) {
        destroy_solution(problem, new_sols + i);
    }
}