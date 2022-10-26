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

#define TO_DOT 0
#define IMPROVE_RATIO 1.1
#define NEIGHBORHOOD_SIZE 30
#define ITERATIONS 5000
#define TABU_SIZE 10
#define SEED 4
#define PERTURBANCE_REMOVE_RATE 0.2
#define PERTURBANCE_NO_NEW_BEST 800

void print_in_out(Problem *problem) {
    for (int i = 0; i < problem->num_stations; i++) {
        printf("%d:", i);
        Node *node = problem->stations[i].source_node->out_waiting->end_node;
        while(node->out_waiting != NULL) {
            if(node->in_subcon != NULL) {
                printf(" i%d", node->in_subcon->start_node->station->id);
            } else {
                printf(" o%d", node->out_subcon->end_node->station->id);
            }
            node = node->out_waiting->end_node;
        }
        printf("\n");
    }
}

// sorting solutions by increasing objective
int cmpfunc (const void * a, const void * b) {
    if (((Solution *) a)->objective > ((Solution *) b)->objective) {
        return 1;
    }
    else {
        return -1;
    }
}

void do_random_operation(Problem *problem, Solution *sol, FILE *operation_data) {
    int r = rand() % 20;
    int rand_st = rand() % problem->num_stations;
    int rand_st2 = rand() % problem->num_stations;
    int rand_edge = rand() % problem->num_edges;
    int rand_start_node_index = rand() % (problem->stations[rand_st].num_nodes - 1);
    int rand_start_node = problem->stations[rand_st].node_ids[rand_start_node_index];
    int rand_end_node_index = rand_start_node_index + 1 + rand() % (problem->stations[rand_st].num_nodes - rand_start_node_index - 1);
    int rand_end_node = problem->stations[rand_st].node_ids[rand_end_node_index];
    int rand_ts = rand() % problem->num_trainset_types;
    int rand_ts2 = rand() % problem->num_trainset_types;
    char *operation_name;
    switch(r) {
        case 0:
            operation_name = "add train";
            oper_add_train_later(sol, problem, rand_st, rand_ts);
            break;
        case 1:
            operation_name = "remove train";
            oper_remove_train(sol, problem, rand_st, rand_ts);
            break;
        case 2:
            operation_name = "remove waiting train";
            oper_remove_waiting_train(sol, problem, rand_st, rand_ts);
            break;
        case 3:
            operation_name = "change train";
            oper_change_train_capacity(sol, problem, rand_st, rand_ts, rand_ts2, 1, 1);
            break;
        case 4:
            operation_name = "move edge back";
            oper_move_edge_back(sol, problem, rand_edge, rand_ts);
            break;
        case 5:
            operation_name = "move edge front";
            //oper_move_edge_front(sol, problem, rand_edge, rand_ts);
            break;
        case 6:
            operation_name = "change train 2:1";
            oper_change_train_capacity(sol, problem, rand_st, rand_ts, rand_ts2, 2, 1);
            break;
        case 7:
            operation_name = "change train 1:2";
            oper_change_train_capacity(sol, problem, rand_st, rand_ts, rand_ts2, 1, 2);
            break;
        case 8:
            operation_name = "add train pair";
            oper_add_train_pair_later(sol, problem, rand_st, rand_st2, rand_ts);
            break;
        case 9:
            operation_name = "change train pair capacity";
            oper_change_train_pair_capacity(sol, problem, rand_st, rand_st2, rand_ts, rand_ts2, 1, 1);
            break;
        case 10:
            operation_name = "reschedule_w_l";
            oper_reschedule_w_l(sol, problem, rand_start_node, rand_end_node, rand_ts);
            break;
        case 11:
            operation_name = "remove train pair";
            oper_remove_train_pair(sol, problem, rand_st, rand_st2, rand_ts);
            break;
        case 12:
        case 13:
            operation_name = "reschedule_n_l";
            oper_reschedule_n_l(sol, problem, rand_start_node, rand_end_node, rand_ts);
            break;
        case 14:
            operation_name = "add_to_empty";
            oper_add_train_to_empty(sol, problem, rand_st);
            break;
        case 15:
            operation_name = "add with edge";
            oper_add_train_with_edge(sol, problem, rand_edge, rand_ts);
            break;
        case 16:
            operation_name = "remove with edge";
            oper_remove_train_with_edge(sol, problem, rand_edge, rand_ts);
            break;
        case 17:
            operation_name = "remove dfs";
            oper_remove_train_dfs(sol, problem, rand_st, rand_ts);
            break;
        case 18:
            operation_name = "change dfs";
            oper_change_train_capacity_dfs(sol, problem, rand_st, rand_ts, rand_ts2, 1, 1);
            break;
        default:
            operation_name = "reschedule_n_w";
            oper_reschedule_n_w(sol, problem, rand_start_node, rand_end_node, rand_ts);
            break;
    }
    if(operation_data) {
        fprintf(operation_data, "%s,", operation_name);
    }
    //printf("%s\n", operation_name);
}

void initial_add(Problem *problem, Solution *sol) {
    char dot_filename[255];
    char dot_title[255];

    for(int i = 0; i < 1000; i++) {
        printf("iteration %d:\n", i);
        int station_id = -1;
        if(i%2 == 0) {
            station_id = select_station_first_empty_departure(problem, sol);
        }else{
            station_id = select_station_last_empty_arrival(problem, sol);
        }

        if(station_id == -1) {
            break;
        }
        printf("add a train beginning in %d\n", station_id);

        oper_add_train_to_empty(sol, problem, station_id);

        if(TO_DOT) {
            sprintf(dot_filename, "dot/init%03d.dot", i);
            sprintf(dot_title, "initial adding to %d", station_id);
            print_problem(problem, sol, dot_filename, dot_title);
        }

        if(!test_consistency(problem, sol)) {
            break;
        }

        printf("objective: %lld\n", sol->objective);
        int empty_subcons = 0;
        for (int j = 0; j < problem->num_edges; j++) {
            if(problem->edges[j].type == SUBCONNECTION && sol->edge_solution[j].capacity == 0) {
                empty_subcons++;
                if(i > 998) {
                    printf("st %d t %ld ->st %d t %ld \n", problem->edges[j].start_node->station->id,
                           problem->edges[j].start_node->time, problem->edges[j].end_node->station->id,
                           problem->edges[j].end_node->time);
                }
            }
        }
        printf("num_empty = %d\n", empty_subcons);
    }
}

void perturbate(Problem *problem, Solution *solution, double remove_rate) {
    int overall_num_ts = 0;
    for (int i = 0; i < problem->num_trainset_types; ++i) {
        overall_num_ts += solution->num_trainstes[i];
    }

    int to_remove = (int) (overall_num_ts * remove_rate);
    int removed = 0;
    while(removed < to_remove) {
        int rand_st_id = rand() % problem->num_stations;
        int rand_ts_id = rand() % problem->num_trainset_types;
        removed += oper_remove_train_dfs(solution, problem, rand_st_id, rand_ts_id);
    }
}

void local_search(Problem *problem, Solution *sol, int taboo_size, int neighborhood_size, int stop_no_improve) {

    FILE *csv_objective = fopen("objective.csv", "w");
    fprintf(csv_objective, "iter,obj\n");
    FILE *csv_operations = fopen("operations.csv", "w");
    fprintf(csv_operations, "iter,operation,taboo,obj_change\n");

    printf("objective: %lld\n", sol->objective);

    int iteration = 0;
    long long int taboo[taboo_size];
    taboo[1] = sol->objective;
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
    int overall_best_iter = 0;

    while(iteration - overall_best_iter < stop_no_improve) {
        printf("%d\n", iteration);

        int local_counter = 0;
        while (local_counter < neighborhood_size) {
            fprintf(csv_operations, "%d,", iteration);

            copy_solution(problem, sol, new_sols + local_counter);
            do_random_operation(problem, new_sols + local_counter, csv_operations);

            bool is_tabu = false;
            for (int i = 0; i < taboo_size; ++i) {
                if(new_sols[local_counter].objective == taboo[i]) {
                    is_tabu = true;
                    break;
                }
            }
            if(is_tabu) {
                fprintf(csv_operations, "1,0\n");
                continue;
            }
            fprintf(csv_operations, "0,%lld\n", new_sols[local_counter].objective - sol->objective);

            local_counter++;
        }
        qsort(new_sols, neighborhood_size, sizeof(Solution), cmpfunc);

        Solution *selected_solution = &new_sols[0];

        copy_solution(problem, selected_solution, sol);
        taboo[tabu_pos] = sol->objective;
        tabu_pos = (tabu_pos + 1) % taboo_size;
        if(sol->objective < overall_best.objective) {
            copy_solution(problem, sol, &overall_best);
            overall_best_iter = iteration;
        }
        fprintf(csv_objective, "%d, %lld\n", iteration, sol->objective);
        iteration++;
    }

    destroy_solution(problem, &overall_best);
    for (int i = 0; i < neighborhood_size; ++i) {
        destroy_solution(problem, new_sols + i);
    }
    fclose(csv_operations);
    fclose(csv_objective);
}

double anneal_accept_prob(long long int old_obj, long long int new_obj, double temperature) {
    if(old_obj > new_obj) {
        printf("%f ", 1);
        return 1;
    }
    if(temperature <= 0) {
        printf("%f ", 0);
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

    double temp = init_temp;
    int iter = 0;
    int last_accept_iter = 0;
    int best_iter = 0;
    while (iter - last_accept_iter < 1000 && iter < max_iter) {
        printf("%d %lld %f %d %d\n", iter, sol->objective, temp, sol->num_trainstes[0], sol->num_trainstes[1]);
        fflush(stdout);
        copy_solution(problem, sol, &new);
        do_random_operation(problem, &new, csv);
        bool accepting = anneal_accept_prob(sol->objective, new.objective, temp) > (double) rand()/RAND_MAX;
        if(accepting){
            copy_solution(problem, &new, sol);
            last_accept_iter = iter;
        }
        if(new.objective < best.objective) {
            copy_solution(problem, &new, &best);
            best_iter = iter;
        }
        if(csv) {
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