#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <argp.h>
#include "datatypes.h"
#include "parse_input.h"
#include "test.h"
#include "operations.h"
#include "output.h"
#include "simulated_annealing.h"
#include "tabu_search.h"
#include "min_flow.h"


typedef struct Arguments Arguments;

struct Arguments {
    char *problem_spec_file;
    bool use_tabu;
    int tabu_len;
    int tabu_neigh;
    int num_steps;
    bool empty_init;
    char *init_sol_file;
    bool verbose;
    char *out_file;
};

static char doc[] = "Solver for trainset capacity problem";

static char args_doc[] = "PROBLEM_FILE";

static struct argp_option options[] = {
        {"verbose",     'v', 0,      0,  "Produce verbose output", 1 },
        {"empty_init",  'e', 0,      0,  "Use empty initial solution", 3 },
        {"tabu",        't', 0,      0,  "Use tabu search (default is simulated annealing)", 5  },
        {"init_file",   'i', "FILE", 0,  "Load the initial solution from csv file", 3 },
        {"output_sol",  'o', "FILE", 0,  "Write the solution to file as csv", 4 },
        {"steps",       's', "N",    0,  "Number of steps of the algorithm", 2 },
        {"neigh",       'n', "N",    0,  "Number of generated neighbors for tabu search", 6 },
        {"tabu_len",    'u', "N",    0,  "Tabu list length", 6 },
        { 0 }
};

bool is_number(char* str) {
    int i = 0;
    while(str[i] != '\0') {
        if(str[i] < '0' || str[i] > '9') {
            return false;
        }
        i++;
    }
    return true;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    Arguments *arguments = state->input;

    switch(key)
    {
        case 'v':
            arguments->verbose = true;
            break;
        case 'e':
            arguments->empty_init = true;
            break;
        case 't':
            arguments->use_tabu = true;
            break;
        case 'o':
            arguments->out_file = arg;
            break;
        case 'i':
            arguments->init_sol_file = arg;
            break;
        case 's':
            if (!is_number(arg)) {
                printf("Number of steps must be a positive integer.\n");
                argp_usage(state);
                break;
            }
            arguments->num_steps = atoi(arg);
            break;
        case 'n':
            if (!is_number(arg)) {
                printf("Neighborhood size must be a positive integer.\n");
                argp_usage(state);
                break;
            }
            arguments->tabu_neigh = atoi(arg);
            break;
        case 'u':
            if (!is_number(arg)) {
                printf("Tabu length must be a positive integer or 0.\n");
                argp_usage(state);
                break;
            }
            arguments->tabu_len = atoi(arg);
            break;
        case ARGP_KEY_ARG:
            arguments->problem_spec_file = arg;
            break;
        case ARGP_KEY_END::
            if (state->arg_num != 1) {
                printf("Wrong number of arguments.\n");
                argp_usage(state);
            }
            break;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

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
    Arguments arguments;
    arguments.problem_spec_file = NULL;
    arguments.use_tabu = false;
    arguments.tabu_len = 5;
    arguments.tabu_neigh = 200;
    arguments.num_steps = 2000000;
    arguments.empty_init = false;
    arguments.init_sol_file = NULL;
    arguments.verbose = false;
    arguments.out_file = NULL;
    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    srand(time(0));
    if(arguments.use_tabu) {
        tabu_run(arguments.problem_spec_file, arguments.num_steps, arguments.tabu_len, arguments.tabu_neigh,
                 arguments.verbose, arguments.empty_init,arguments.init_sol_file, arguments.out_file);

    }
    else {
        annealing_run(arguments.problem_spec_file, arguments.num_steps, arguments.verbose, arguments.empty_init,
                      arguments.init_sol_file, arguments.out_file);
    }
    return 0;
}