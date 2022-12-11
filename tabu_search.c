//
// Created by fitli on 31.10.22.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tabu_search.h"
#include "operations.h"

/**
 * sorting solutions by increasing objective
 * @param a pointer to a solution
 * @param b pointer to a solution
 * @return
 */
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

void tabu_search(Problem *problem, Solution *sol, int taboo_size, int neighborhood_size, int max_iters, clock_t inittime, bool verbose) {

    int iteration = 0;
    long long int taboo[taboo_size];
    taboo[0] = sol->objective;
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

    if(verbose)
        printf("iter\tobj\ttime\n------------------------\n");

    while(iteration < max_iters) {
        if(iteration * neighborhood_size % 10000 == 0 && verbose) {
            printf("%d\t%lld\t%f\n", iteration, sol->objective,
                   (double) (clock() - inittime) / (double) CLOCKS_PER_SEC);
            fflush(stdout);
        }

        int local_counter = 0;
        while (local_counter < neighborhood_size) {

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
                continue;
            }

            local_counter++;
        }
        Solution *selected_solution = &new_sols[0];
        for (int i = 1; i < neighborhood_size; i++) {
            if(new_sols[i].objective < selected_solution->objective) {
                selected_solution = &new_sols[i];
            }
        }

        copy_solution(problem, selected_solution, sol);
        taboo[tabu_pos] = sol->objective;
        tabu_pos = (tabu_pos + 1) % taboo_size;
        if(sol->objective < overall_best.objective) {
            copy_solution(problem, sol, &overall_best);
        }
        iteration++;
    }

    if(verbose) {
        printf("------------------------\n");
    }

    copy_solution(problem, &overall_best, sol);
    destroy_solution(problem, &overall_best);
    for (int i = 0; i < neighborhood_size; ++i) {
        destroy_solution(problem, new_sols + i);
    }
}