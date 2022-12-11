#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "datatypes.h"
#include "parse_input.h"
#include "test.h"
#include "operations.h"
#include "output.h"
#include "simmulated_annealing.h"
#include "tabu_search.h"
#include "min_flow.h"

#define DATASET "../../small_data_2_ts.cfg"
//#define DATASET "../../big_data_2_ts.cfg"
//#define DATASET "../../data-cfg/real300_800.cfg"



Solution get_init_sol(Problem *problem, bool empty, char *init_sol_csv) {
    if (empty) {
        Solution sol;
        empty_solution(problem, &sol);
        return sol;
    }
    if (init_sol_csv) {
        return read_sol_from_csv(problem, init_sol_csv);
    }
    return min_flow(problem, &problem->trainset_types[1]);
}

void tabu_run(const char *filename, int steps, int tabu_len, int neigh_size, bool verbose, bool empty_init, char *init_sol_csv, char *output_csv) {
    Problem problem;
    parse_problem(filename, &problem);

    Solution sol = get_init_sol(&problem, empty_init, init_sol_csv);

    clock_t init_time = clock();
    tabu_search(&problem, &sol, tabu_len, neigh_size, steps/neigh_size, init_time, verbose);

    analyze_solution(&sol, &problem);
    printf("running time: %f s\n", (double)(clock()-init_time)/(double)CLOCKS_PER_SEC);

    if(output_csv) {
        sol_to_csv(&sol, &problem, output_csv);
    }

    destroy_solution(&problem, &sol);
    destroy_problem(&problem);
}

void annealing_run(const char *filename, int steps, bool verbose, bool empty_init, char *init_sol_csv, char *output_csv) {
    Problem problem;
    parse_problem(filename, &problem);

    Solution sol = get_init_sol(&problem, empty_init, init_sol_csv);

    double temp = init_temp(&problem, &sol, 1000, 0.5);
    double geom_decrease = 1/pow(temp, 1.0/steps);

    clock_t init_time = clock();
    simulated_annealing(&problem, &sol, temp, geom_decrease, steps, init_time, GEOMETRIC, verbose);

    analyze_solution(&sol, &problem);
    printf("running time: %f s\n", (double)(clock()-init_time)/(double)CLOCKS_PER_SEC);

    if(output_csv) {
        sol_to_csv(&sol, &problem, output_csv);
    }

    destroy_solution(&problem, &sol);
    destroy_problem(&problem);
}

int main(int argc, char *argv[]) {
    srand(time(0));
    annealing_run(DATASET, 2000000, true, false, "res.csv", NULL);
    tabu_run(DATASET, 1000000, 5, 200, true, false, NULL, NULL);
    return 0;
}