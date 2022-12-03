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
#include "operations.h"
#include "simmulated_annealing.h"
#include "tabu_search.h"

#define STEPS 2e6
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
            //simulated_annealing(&problem, &sol, init_temp, lin_temp_decrease, STEPS, NULL, NULL, init_time, LINEAR, false, NULL, false);
            clock_t time = clock() - init_time;
            printf("%d,%f,lin,%f,%lld,%f\n", seed, init_temp, lin_temp_decrease, sol.objective, (double)(time)/(double)CLOCKS_PER_SEC);
        }
        for (int seed = 0; seed < SEEDS; ++seed) {
            srand(seed);
            copy_solution(&problem, &init_sol, &sol);
            clock_t init_time = clock();
            //simulated_annealing(&problem, &sol, init_temp, geom_decrease, STEPS, NULL, NULL, init_time, GEOMETRIC, false, NULL, false);
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
        //simulated_annealing(&problem, &sol, 0.0, 0.0, STEPS, NULL, NULL, init_time, GEOMETRIC, false, NULL, false);
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
    //Solution init_sol;
    //empty_solution(&problem, &init_sol);

    double temp = init_temp(&problem, &init_sol, 1000, 0.5);
    double geom_decrease = 1/pow(temp, 1/STEPS);


    printf("seed,accepted,obj,temp,time");
    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        printf(",oper%d", i);
    }
    printf("\n");
    fflush(stdout);

    for (int i = 0; i < SEEDS; ++i) {
        srand(i);
        Solution sol;
        empty_solution(&problem, &sol);
        copy_solution(&problem, &init_sol, &sol);
        char sseed[5];
        sprintf(sseed, "%d,", i);
        clock_t init_time = clock();
        //simulated_annealing(&problem, &sol, temp, geom_decrease, STEPS, stdout, sseed, init_time, GEOMETRIC, false, NULL, false);
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
        srand(i);
        Solution sol;
        empty_solution(&problem, &sol);
        copy_solution(&problem, &init_sol, &sol);
        char sseed[5];
        sprintf(sseed, "%d,", i);
        clock_t init_time = clock();
        tabu_search(&problem, &sol, tabu_len, neigh_size, STEPS, STEPS/neigh_size, init_time, stdout, NULL);
    }

}

void weight_for_change(const char *filename) {
    Problem problem;
    parse_problem(filename, &problem);

    Solution init_sol = min_flow(&problem, &problem.trainset_types[1]);
    //Solution init_sol;
    //empty_solution(&problem, &init_sol);

    double temp = init_temp(&problem, &init_sol, 1000, 0.5);
    double geom_decrease = 1/pow(temp, 1/STEPS);


    printf("seed,accepted,obj,temp,time");
    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        printf(",oper%d", i);
    }
    printf("\n");
    fflush(stdout);

    int weights[4] = {100, 700, 2100, 6300};
    for (int i = 0; i < SEEDS; ++i) {
        for (int j = 0; j < 4; ++j) {
            int op_weights[8] = {100,100,100,weights[j],100,100,100,100};
            Solution sol;
            empty_solution(&problem, &sol);
            copy_solution(&problem, &init_sol, &sol);
            char sseed[5];
            sprintf(sseed, "%d,", i);
            clock_t init_time = clock();
            //simulated_annealing(&problem, &sol, temp, geom_decrease, STEPS, stdout, sseed, init_time, GEOMETRIC, false, op_weights, false);
        }
    }

}

void prob_grid_search(const char *filename) {
    Problem problem;
    parse_problem(filename, &problem);

    Solution sol;
    empty_solution(&problem, &sol);

    Solution init_sol = min_flow(&problem, &problem.trainset_types[1]);

    double p1s[3] = {0,0.01,0.10};
    double p2s[3] = {0.90,0.99,1};
    double p3s[3] = {0.5,0.7,0.9};
    double p4s[3] = {0.3,0.5,0.7};

    double ps[4] = {0.01, 0.99, 0.7, 0.4};
    double temp = init_temp(&problem, &init_sol, 1000, 0.5);
    double geom_decrease = 1/pow(temp, 1/STEPS);
    copy_solution(&problem, &init_sol, &sol);
    //clock_t init_time = clock();
    //simulated_annealing(&problem, &sol, temp, geom_decrease, STEPS, stdout, "", init_time, GEOMETRIC, false, NULL, false, ps);


    printf("SEED,temp,type,decrease,obj,time,p1,p2,p3,p4\n");

    for (int seed = 0; seed < SEEDS; ++seed) {
        srand(seed);
        for (int i = 0; i < 3; ++i) {
            ps[0] = p1s[i];
            for (int j = 0; j < 3; ++j) {
                ps[1] = p2s[j];
                for (int k = 0; k < 3; ++k) {
                    ps[2] = p3s[k];
                    for (int l = 0; l < 3; ++l) {
                        ps[3] = p4s[l];
                        copy_solution(&problem, &init_sol, &sol);
                        clock_t init_time = clock();
                        simulated_annealing(&problem, &sol, temp, geom_decrease, STEPS, NULL, NULL, init_time, GEOMETRIC, false, NULL, false);
                        clock_t time = clock() - init_time;
                        printf("%d,%f,geo,%f,%lld,%f,%f,%f,%f,%f\n", seed, temp, geom_decrease, sol.objective, (double)(time)/(double)CLOCKS_PER_SEC,ps[0],ps[1],ps[2],ps[3]);
                    }
                }
            }
        }
    }
}

void annealing_long_schedules_geom(Problem *problem, Solution *sol, double init_temp, int seed, int steps) {
    srand(seed);

    double geom_decrease = 1/pow(init_temp, 1.0/steps);

    char prefix[100];
    sprintf(prefix, "%d,geo,", seed);

    clock_t init_time = clock();
    simulated_annealing(problem, sol, init_temp, geom_decrease, steps, stdout, prefix, init_time, GEOMETRIC, false, NULL, false);
}

void annealing_long_schedules_quick_geom(Problem *problem, Solution *sol, double init_temp, int seed, int steps, int init_steps) {
    srand(seed);

    double geom_decrease = 1/pow(init_temp, 1.0/init_steps);

    char prefix[100];
    sprintf(prefix, "%d,quick_geo%dM,", seed, init_steps/(int)1e6);

    clock_t init_time = clock();
    simulated_annealing(problem, sol, init_temp, geom_decrease, init_steps, stdout, prefix, init_time, GEOMETRIC, false, NULL, false);
    simulated_annealing(problem, sol, 1, 1, steps-init_steps, stdout, prefix, init_time, GEOMETRIC, false, NULL, false);
}

void annealing_long_schedules_iter_geom(Problem *problem, Solution *sol, double init_temp, int seed, int steps, int iter_steps) {
    srand(seed);

    char prefix[100];
    sprintf(prefix, "%d,iter_geo%dM,", seed, iter_steps/(int)1e6);

    clock_t init_time = clock();

    for (int i = 0; i < steps / iter_steps; ++i) {
        double geom_decrease = 1/pow(init_temp, 1.0/iter_steps);
        simulated_annealing(problem, sol, init_temp, geom_decrease, iter_steps, stdout, prefix, init_time, GEOMETRIC, false, NULL, false);
        init_temp/=2;
    }
}

void annealing_long_schedules(const char *filename, int init_seed) {
    Problem problem;
    parse_problem(filename, &problem);

    Solution init_sol = min_flow(&problem, &problem.trainset_types[1]);

    double temp = init_temp(&problem, &init_sol, 1000, 0.5);

    for (int i = init_seed; i < init_seed+SEEDS; ++i) {
        Solution sol;
        empty_solution(&problem, &sol);
        copy_solution(&problem, &init_sol, &sol);
        annealing_long_schedules_geom(&problem, &sol, temp, i, 16e6);
        copy_solution(&problem, &init_sol, &sol);
        annealing_long_schedules_quick_geom(&problem, &sol, temp, i, 16e6, 1e6);
        copy_solution(&problem, &init_sol, &sol);
        annealing_long_schedules_quick_geom(&problem, &sol, temp, i, 16e6, 2e6);
        copy_solution(&problem, &init_sol, &sol);
        annealing_long_schedules_quick_geom(&problem, &sol, temp, i, 16e6, 8e6);
        copy_solution(&problem, &init_sol, &sol);
        annealing_long_schedules_iter_geom(&problem, &sol, temp, i, 16e6, 1e6);
        copy_solution(&problem, &init_sol, &sol);
        annealing_long_schedules_iter_geom(&problem, &sol, temp, i, 16e6, 2e6);
        destroy_solution(&problem, &sol);
    }

    destroy_solution(&problem, &init_sol);
    destroy_problem(&problem);

}

void final_run(const char *filename) {
    Problem problem;
    parse_problem(filename, &problem);

    Solution init_sol = min_flow(&problem, &problem.trainset_types[1]);

    double temp = init_temp(&problem, &init_sol, 1000, 0.5);
    printf("seed,schedule,acc,cost,temp,time,o0,o1,o2,o3,o4,o5,o6,o7\n");

    for (int i = 0; i < SEEDS; ++i) {
        srand(i);
        Solution sol;
        empty_solution(&problem, &sol);
        copy_solution(&problem, &init_sol, &sol);
        annealing_long_schedules_geom(&problem, &sol, temp, i, 16e6);
    }

    destroy_solution(&problem, &init_sol);
    destroy_problem(&problem);

}