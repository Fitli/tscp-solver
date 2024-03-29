//
// Created by Ivana Krumlová on 31.10.22.
//

#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "simulated_annealing.h"
#include "operations.h"
#include "test.h"

#define WEIGHT_DECREASE 0.999
#define WEIGHT_UPDATE 100
#define WEIGHT_MIN 100

double init_temp(Problem *problem, Solution *solution, int neigh_size, double avg_accept_prob) {
    int increas_num = 0;
    long long increas_sum = 0;
    Solution modified_sol;
    empty_solution(problem, &modified_sol);
    copy_solution(problem, solution, &modified_sol);
    for (int i = 0; i < neigh_size; ++i) {
        long long old_obj = modified_sol.objective;
        select_operation(problem, &modified_sol, NULL);
        if(modified_sol.objective > old_obj && modified_sol.objective - old_obj < (long long) 1e12) {
            increas_num++;
            increas_sum += modified_sol.objective - old_obj;
        }
    }
    double increas_avg = (double) increas_sum/increas_num;
    destroy_solution(problem, &modified_sol);
    return -1 * increas_avg/log(avg_accept_prob);
}

void update_weights(int *weights, int accepted_op) {
    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        weights[i] = (int) (weights[i] * WEIGHT_DECREASE);
        if(weights[i] < WEIGHT_MIN) {
            weights[i] = WEIGHT_MIN;
        }
    }
    if(accepted_op >= 0)
        weights[accepted_op] += WEIGHT_UPDATE;
}

double anneal_accept_prob(long long int old_obj, long long int new_obj, double temperature) {
    if(old_obj > new_obj) {
        //printf("%f ", 1);
        return 1;
    }
    if(temperature <= 0) {
        //printf("%f ", 0);
        return 0;
    }
    double metropolis = exp(-1 * (double) (new_obj-old_obj)/temperature);
    //printf("%f ", metropolis);
    return metropolis;
}

void simulated_annealing(Problem *problem, Solution *sol, double init_temp, double temp_decrease, int max_iter,
                         clock_t inittime, enum TempDecrease temp_decrease_type, bool verbose) {

    Solution best;
    Solution new;
    empty_solution(problem, &best);
    empty_solution(problem, &new);

    int oper_weights[NUM_OPERATIONS];
    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        oper_weights[i] = 100;
    }

    double temp = init_temp;
    int iter = 0;
    int last_accept_iter = 0;

    if(verbose) {
        printf("iter\tcost\ttime\ttemperature\n------------------------\n");
    }
    while (iter - last_accept_iter < 100000 && iter < max_iter) {
        if(iter % 20000 == 0 && verbose)
            printf("%d\t%lld\t%f\t%f\n", iter, sol->objective,
                   (double)(clock()-inittime)/(double)CLOCKS_PER_SEC, temp);
        fflush(stdout);
        copy_solution(problem, sol, &new);
        int op;
        op = select_operation(problem, &new, oper_weights);
        bool accepting = anneal_accept_prob(sol->objective, new.objective, temp) > (double) rand()/RAND_MAX;
        if(accepting){
            //update operation weifhts
            /*
            if(new.objective < sol->objective)
                update_weights(oper_weights, op);
            else
                update_weights(oper_weights, -1);
            */
            copy_solution(problem, &new, sol);
            last_accept_iter = iter;
        }
        if(new.objective < best.objective) {
            copy_solution(problem, &new, &best);
        }

        iter++;
        if(temp > 0) {
            if(temp_decrease_type == LINEAR) {
                temp -= temp_decrease;
            }
            if(temp_decrease_type == GEOMETRIC) {
                temp *= temp_decrease;
            }
        }
    }

    copy_solution(problem, &best, sol);

    if(verbose) {
        printf("------------------------\n");
    }

    destroy_solution(problem, &new);
    destroy_solution(problem, &best);
}
