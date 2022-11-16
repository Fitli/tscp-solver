//
// Created by fitli on 31.10.22.
//

#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "simmulated_annealing.h"
#include "operations.h"

#define WEIGHT_DECREASE 0.99
#define WEIGHT_UPDATE 100
#define WEIGHT_MIN 100

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
        printf("%d ", 1);
        return 1;
    }
    if(temperature <= 0) {
        printf("%d ", 0);
        return 0;
    }
    double metropolis = exp(-1 * (double) (new_obj-old_obj)/temperature);
    printf("%f ", metropolis);
    return metropolis;
}

void simulated_annealing(Problem *problem, Solution *sol, double init_temp, double temp_decrease, int max_iter, FILE *csv, clock_t inittime) {

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
    while (iter - last_accept_iter < 1000 && iter < max_iter) {
        printf("%d %lld %f %d %d\n", iter, sol->objective, temp, sol->num_trainstes[0], sol->num_trainstes[1]);
        fflush(stdout);
        copy_solution(problem, sol, &new);
        int op;
        op = select_operation(problem, &new, oper_weights);
        bool accepting = anneal_accept_prob(sol->objective, new.objective, temp) > (double) rand()/RAND_MAX;
        if(accepting){
            if(new.objective != sol->objective)
                update_weights(oper_weights, op);
            else
                update_weights(oper_weights, -1);
            copy_solution(problem, &new, sol);
            last_accept_iter = iter;
        }
        if(new.objective < best.objective) {
            copy_solution(problem, &new, &best);
        }
        if(csv && iter%1000==0) {
            fprintf(csv, "%d, %lld, %f, %f, ", accepting, sol->objective,temp,
                    (double)(clock()-inittime)/(double)CLOCKS_PER_SEC);
            for (int i = 0; i < problem->num_trainset_types; ++i) {
                fprintf(csv, "%d, ", sol->num_trainstes[i]);
            }
            fprintf(csv, "\n");
        }

        iter++;
        if(temp > 0) {
            temp -= temp_decrease;
        }
    }

    copy_solution(problem, &best, sol);

    destroy_solution(problem, &new);
    destroy_solution(problem, &best);
}
