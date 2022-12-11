//
// Created by fitli on 31.10.22.
//

#include <stdio.h>
#include <stdbool.h>
#include "tabu_search.h"
#include "operations.h"


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