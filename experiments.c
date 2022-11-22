//
// Created by fitli on 22.11.22.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "experiments.h"
#include "datatypes.h"
#include "parse_input.h"
#include "min_flow.h"
#include "simmulated_annealing.h"

#define STEPS 1e6

void annealing_parameters() {
    Problem problem;
    parse_problem("../../big_data_2_ts.cfg", &problem);

    Solution init_sol;
    empty_solution(&problem, &init_sol);
    Solution sol;
    empty_solution(&problem, &sol);

    init_sol = min_flow(&problem, &problem.trainset_types[1]);
    double temp_formula = init_temp(&problem, &init_sol, 1000, 0.5);

    double temperatures[6] = {temp_formula, 1e11, 1e10, 1e9, 1e8, 1e7};
    double geom_decrease = 0.99;


    printf("SEED,temp,type,decrease,obj,time\n");

    for (int i = 0; i < 6; ++i) {
        double init_temp = temperatures[i];
        double lin_temp_decrease = init_temp/STEPS;
        for (int seed = 0; seed < 1; ++seed) {
            srand(seed);
            copy_solution(&problem, &init_sol, &sol);
            clock_t init_time = clock();
            simulated_annealing(&problem, &sol, init_temp, lin_temp_decrease, STEPS, NULL, init_time, LINEAR, false);
            clock_t time = clock() - init_time;
            printf("%d,%f,lin,%f,%lld,%f\n", seed, init_temp, lin_temp_decrease, sol.objective, (double)(time)/(double)CLOCKS_PER_SEC);
        }
        for (int seed = 0; seed < 10; ++seed) {
            srand(seed);
            copy_solution(&problem, &init_sol, &sol);
            clock_t init_time = clock();
            simulated_annealing(&problem, &sol, init_temp, geom_decrease, STEPS, NULL, init_time, GEOMETRIC, false);
            clock_t time = clock() - init_time;
            printf("%d,%f,geo,%f,%lld,%f\n", seed, init_temp, geom_decrease, sol.objective, (double)(time)/(double)CLOCKS_PER_SEC);
        }
    }
}