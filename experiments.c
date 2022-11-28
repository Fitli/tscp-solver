//
// Created by fitli on 22.11.22.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "experiments.h"
#include "datatypes.h"
#include "parse_input.h"
#include "min_flow.h"
#include "simmulated_annealing.h"
#include "tabu_search.h"

#define STEPS 1e6
#define SEEDS 10

void annealing_parameters(const char *filename) {
    Problem problem;
    parse_problem(filename, &problem);

    Solution init_sol;
    empty_solution(&problem, &init_sol);
    Solution sol;
    empty_solution(&problem, &sol);

    init_sol = min_flow(&problem, &problem.trainset_types[1]);
    double temp_formula = init_temp(&problem, &init_sol, 1000, 0.5);

    //double temperatures[7] = {temp_formula, 1e11, 1e10, 1e9, 1e8, 1e7, 1e6};
    double temperatures[1] = {temp_formula};


    printf("SEED,temp,type,decrease,obj,time\n");

    for (int i = 0; i < 1; ++i) {
        double init_temp = temperatures[i];
        double lin_temp_decrease = init_temp/STEPS;
        double geom_decrease = 1/pow(init_temp, 1/STEPS);
        for (int seed = 0; seed < SEEDS; ++seed) {
            srand(seed);
            copy_solution(&problem, &init_sol, &sol);
            clock_t init_time = clock();
            simulated_annealing(&problem, &sol, init_temp, lin_temp_decrease, STEPS, NULL, NULL, init_time, LINEAR, false, false);
            clock_t time = clock() - init_time;
            printf("%d,%f,lin,%f,%lld,%f\n", seed, init_temp, lin_temp_decrease, sol.objective, (double)(time)/(double)CLOCKS_PER_SEC);
        }
        for (int seed = 0; seed < SEEDS; ++seed) {
            srand(seed);
            copy_solution(&problem, &init_sol, &sol);
            clock_t init_time = clock();
            simulated_annealing(&problem, &sol, init_temp, geom_decrease, STEPS, NULL, NULL, init_time, GEOMETRIC, false, false);
            clock_t time = clock() - init_time;
            printf("%d,%f,geo,%f,%lld,%f\n", seed, init_temp, geom_decrease, sol.objective, (double)(time)/(double)CLOCKS_PER_SEC);
        }
    }
}

void hill_climb(const char *filename) {
    Problem problem;
    parse_problem(filename, &problem);

    Solution init_sol;
    empty_solution(&problem, &init_sol);
    Solution sol;
    empty_solution(&problem, &sol);

    init_sol = min_flow(&problem, &problem.trainset_types[1]);

    printf("SEED,temp,type,decrease,obj,time\n");

    for (int seed = 0; seed < SEEDS; ++seed) {
        srand(seed);
        copy_solution(&problem, &init_sol, &sol);
        clock_t init_time = clock();
        simulated_annealing(&problem, &sol, 0.0, 0.0, STEPS, NULL, NULL, init_time, GEOMETRIC, false, false);
        clock_t time = clock() - init_time;
        printf("%d,%f,geo,%f,%lld,%f\n", seed, 0.0, 0.0, sol.objective, (double)(time)/(double)CLOCKS_PER_SEC);
    }
}

void tabu_params(const char *filename) {
    Problem problem;
    parse_problem(filename, &problem);

    Solution init_sol;
    empty_solution(&problem, &init_sol);
    Solution sol;
    empty_solution(&problem, &sol);

    init_sol = min_flow(&problem, &problem.trainset_types[1]);


    printf("SEED,tabu,neigh,obj,time\n");
    fflush(stdout);

    int tabu_lens[4] = {1, 5, 10, 20};
    int neigh_sizes[7] = {10, 30, 50, 100, 200, 500, 1000};
    for(int t = 0; t < 4; t++) {
        int tabu_len = tabu_lens[t];
        for (int n = 0; n < 7; ++n) {
            int neigh_size = neigh_sizes[n];
            for (int seed = 0; seed < SEEDS; ++seed) {
                srand(seed);
                copy_solution(&problem, &init_sol, &sol);
                clock_t init_time = clock();
                tabu_search(&problem, &sol, tabu_len, neigh_size, STEPS, STEPS/neigh_size, init_time, NULL, NULL);
                clock_t time = clock() - init_time;
                printf("%d,%d,%d,%lld,%f\n", seed, tabu_len, neigh_size, sol.objective, (double)(time)/(double)CLOCKS_PER_SEC);
                fflush(stdout);
            }
        }
    }
}

void annealing_run(const char *filename) {
    Problem problem;
    parse_problem(filename, &problem);

    Solution init_sol = min_flow(&problem, &problem.trainset_types[1]);

    double temp = init_temp(&problem, &init_sol, 1000, 0.5);
    double geom_decrease = 1/pow(temp, 1/STEPS);


    printf("seed,accepted,obj,temp,time\n");
    fflush(stdout);

    for (int i = 0; i < SEEDS; ++i) {
        Solution sol;
        empty_solution(&problem, &sol);
        copy_solution(&problem, &sol, &init_sol);
        char sseed[5];
        sprintf(sseed, "%d,", i);
        clock_t init_time = clock();
        simulated_annealing(&problem, &sol, temp, geom_decrease, STEPS, stdout, sseed, init_time, GEOMETRIC, false, false);
    }

}

void tabu_run(const char *filename) {
    Problem problem;
    parse_problem(filename, &problem);

    Solution init_sol = min_flow(&problem, &problem.trainset_types[1]);

    int tabu_len = 10;
    int neigh_size = 5000;

    printf("iter,obj,time\n");
    fflush(stdout);

    for (int i = 0; i < SEEDS; ++i) {
        Solution sol;
        empty_solution(&problem, &sol);
        copy_solution(&problem, &sol, &init_sol);
        char sseed[5];
        sprintf(sseed, "%d,", i);
        clock_t init_time = clock();
        tabu_search(&problem, &sol, tabu_len, neigh_size, STEPS, STEPS/neigh_size, init_time, stdout, NULL);
    }

}