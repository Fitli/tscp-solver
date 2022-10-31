#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "datatypes.h"
#include "parse_config.h"
#include "solution_modifier.h"
#include "heuristics.h"
#include "change_finder.h"
#include "test.h"
#include "operations.h"
#include "dot_printer.h"
#include "random.h"
#include "objective.h"
#include "output.h"
#include "simmulated_annealing.h"
#include "local_search.h"
#include "constructive_alg.h"

#define TO_DOT 1
#define SEED 4

int main() {
    srand(SEED);


    Problem problem;
    parse("../../small_data_2_ts.cfg", &problem);
    //parse("../../big_data_2_ts.cfg", &problem);

    FILE *csv = fopen("annealing.csv", "w");
    fprintf(csv, "oper, accepted, obj, temp, time, ");
    for (int i = 0; i < problem.num_trainset_types; ++i) {
        fprintf(csv, "trainset_%d, ", i);
    }
    fprintf(csv, "\n");

    clock_t inittime = clock();

    Solution sol;
    empty_solution(&problem, &sol);

    //constructive_alg(&sol, &problem, 10000000);

    simulated_annealing(&problem, &sol, 100000000000000, 100000, 100000, csv, inittime);
    simulated_annealing(&problem, &sol, 1000000000, 500, 1000000000, csv, inittime);
    long long int old_obj = sol.objective;
    int big_iters = 0;
    do {
        old_obj = sol.objective;
        simulated_annealing(&problem, &sol, 100000000, 100, 1000000000, csv, inittime);
        big_iters++;
    } while(sol.objective < old_obj);

    printf("iters: %d\n", big_iters);

    big_iters = 0;
    do {
        old_obj = sol.objective;
        simulated_annealing(&problem, &sol, 1000000, 10, 1000000000, csv, inittime);
        big_iters++;
    } while(sol.objective < old_obj);

    printf("iters: %d\n", big_iters);

    //local_search(&problem, &sol, 10, 30, 2000);

    analyze_solution(&sol, &problem);

    fclose(csv);

    if(TO_DOT) {
        print_problem(&problem, &sol, "final.dot", "final solution");
    }

    destroy_solution(&problem, &sol);
    destroy_problem(&problem);

    return 0;
}