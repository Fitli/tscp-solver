#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "datatypes.h"
#include "parse_input.h"
#include "test.h"
#include "operations.h"
#include "dot_printer.h"
#include "output.h"
#include "simmulated_annealing.h"
#include "tabu_search.h"
#include "constructive_alg.h"
#include "min_flow.h"
#include "experiments.h"

#define TO_DOT 1
#define SEED 2
//#define DATASET "../../small_data_2_ts.cfg"
#define DATASET "../../big_data_2_ts.cfg"
//#define DATASET "../../data-cfg/real300_500.cfg"

void read_solution_main() {
    Problem problem;
    parse_problem(DATASET, &problem);

    Solution sol = read_sol_from_csv(&problem, "../../sol_cplex_big.csv");

    analyze_solution(&sol, &problem);

    if(TO_DOT) {
        print_problem(&problem, &sol, "final.dot", "final solution");
    }

    destroy_solution(&problem, &sol);
    destroy_problem(&problem);
}

void local_search_main() {
    srand(SEED);


    Problem problem;
    parse_problem(DATASET, &problem);

    clock_t inittime = clock();

    Solution sol;
    //empty_solution(&problem, &sol);
    sol = min_flow(&problem, &problem.trainset_types[1]);


    FILE *csv_objective = fopen("local_search_objective_big.csv", "w");
    fprintf(csv_objective, "iter,obj,time\n");
    FILE *csv_operations = fopen("local_search_operations_big.csv", "w");
    fprintf(csv_operations, "iter,operation,taboo,obj_change\n");
    tabu_search(&problem, &sol, 10, 30, 20000, 1000000, inittime, csv_objective, csv_operations);

    analyze_solution(&sol, &problem);

    fclose(csv_objective);
    fclose(csv_operations);

    if(TO_DOT) {
        print_problem(&problem, &sol, "final.dot", "final solution");
    }

    destroy_solution(&problem, &sol);
    destroy_problem(&problem);
}

void constructive_main() {
    srand(SEED);

    Problem problem;
    parse_problem(DATASET, &problem);

    Solution sol;
    empty_solution(&problem, &sol);

    constructive_alg(&sol, &problem, 10000000);

    analyze_solution(&sol, &problem);

    if(TO_DOT) {
        print_problem(&problem, &sol, "final.dot", "final solution");
    }

    destroy_solution(&problem, &sol);
    destroy_problem(&problem);
}

void annealing_main() {
    srand(SEED);


    Problem problem;
    parse_problem(DATASET, &problem);

    FILE *csv = fopen("annealing_big2.csv", "w");
    fprintf(csv, "oper,accepted,obj,temp,time,");
    for (int i = 0; i < problem.num_trainset_types; ++i) {
        fprintf(csv, "trainset_%d, ", i);
    }
    fprintf(csv, "\n");

    clock_t inittime = clock();

    Solution sol;
    empty_solution(&problem, &sol);

    sol = min_flow(&problem, &problem.trainset_types[1]);

    //simulated_annealing(&problem, &sol, 100000000000000, 10000, 10000, csv, inittime);
    //double temp = init_temp(&problem, &sol, 1000, 0.5);
    double temp = 700000000;
    //simulated_annealing(&problem, &sol, 700000000, 1000, 1000000000, csv, inittime, LINEAR, true);
    simulated_annealing(&problem, &sol, temp, 0.99998, 1e6, NULL, NULL, inittime, GEOMETRIC, true, true);
    long long int old_obj;
    int big_iters = 0;
    do {
        old_obj = sol.objective;
        simulated_annealing(&problem, &sol, 100000000, 1000, 1000000000, csv, "", inittime, LINEAR, true, true);
        big_iters++;
    } while(sol.objective < old_obj);

    printf("iters: %d\n", big_iters);

    big_iters = 0;
    do {
        old_obj = sol.objective;
        simulated_annealing(&problem, &sol, 1000000, 10, 1000000000, csv, "", inittime, LINEAR, true, true);
        big_iters++;
    } while(sol.objective < old_obj);

    printf("iters: %d\n", big_iters);
    printf("time: %f s\n", (double)(clock()-inittime)/(double)CLOCKS_PER_SEC);

    analyze_solution(&sol, &problem);

    fclose(csv);

    if(TO_DOT) {
        print_problem(&problem, &sol, "final.dot", "final solution");
    }

    destroy_solution(&problem, &sol);
    destroy_problem(&problem);
}



int min_flow_main() {
    srand(SEED);

    Problem problem;
    parse_problem(DATASET, &problem);

    Solution sol = min_flow(&problem, &problem.trainset_types[1]);

    analyze_solution(&sol, &problem);

    if(TO_DOT) {
        print_problem(&problem, &sol, "minflow.dot", "minflow");
    }

    destroy_solution(&problem, &sol);
    destroy_problem(&problem);
}

int main(int argc, char *argv[]) {
    //tabu_params(argv[1]);
    //annealing_main();
    //hill_climb("../../big_data_2_ts.cfg");
    annealing_run(argv[1]);
    return 0;
}