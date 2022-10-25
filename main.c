#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
#define SEED 3
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

void perturbate(Problem *problem, Solution *solution) {
    int num_ts[problem->num_trainset_types];
    get_num_ts(solution, problem, num_ts);
    int overall_num_ts = 0;
    for (int i = 0; i < problem->num_trainset_types; ++i) {
        overall_num_ts += num_ts[i];
    }

    int to_remove = (int) (overall_num_ts * PERTURBANCE_REMOVE_RATE);
    int removed = 0;
    while(removed < to_remove) {
        int rand_st_id = rand() % problem->num_stations;
        int rand_ts_id = rand() % problem->num_trainset_types;
        removed += oper_remove_train_dfs(solution, problem, rand_st_id, rand_ts_id);
    }
}

void local_search(Problem *problem) {
    Solution sol;
    char dot_filename[255];
    char dot_title[255];

    FILE *csv_objective = fopen("objective.csv", "w");
    fprintf(csv_objective, "iter,obj\n");
    FILE *csv_operations = fopen("operations.csv", "w");
    fprintf(csv_operations, "iter,operation,tabu,obj_change\n");

    empty_solution(&problem, &sol);
    printf("objective: %lld\n", sol.objective);

    int iteration = 0;
    long long int tabu[TABU_SIZE];
    tabu[1] = sol.objective;
    for (int i = 1; i < TABU_SIZE; ++i) {
        tabu[i] = 0;
    }
    int tabu_pos = 1;
    Solution new_sols [NEIGHBORHOOD_SIZE];
    for(int i = 0; i < NEIGHBORHOOD_SIZE; i++) {
        empty_solution(&problem, new_sols + i);
    }
    Solution overall_best;
    empty_solution(&problem, &overall_best);
    copy_solution(&problem, &sol, &overall_best);
    int overall_best_iter = 0;
    int last_perturbance_iter = 0;
    while(iteration-overall_best_iter < ITERATIONS) {
        printf("%d\n", iteration);


        if(iteration - overall_best_iter >= PERTURBANCE_NO_NEW_BEST && iteration - last_perturbance_iter >= PERTURBANCE_NO_NEW_BEST) {
            copy_solution(&problem, &overall_best, &sol);
            perturbate(&problem, &sol);
            last_perturbance_iter = iteration;
        }

        int local_counter = 0;
        while (local_counter < NEIGHBORHOOD_SIZE) {
            fprintf(csv_operations, "%d,", iteration);

            copy_solution(&problem, &sol, new_sols + local_counter);
            do_random_operation(&problem, new_sols + local_counter, csv_operations);

            if(!test_consistency(&problem, new_sols + local_counter)) {
                printf("unconsistent\n");
                break;
            }

            if(!test_objective(&problem, new_sols + local_counter)) {
                printf("broken objective\n");
                break;
            }

            bool is_tabu = false;
            for (int i = 0; i < TABU_SIZE; ++i) {
                if(new_sols[local_counter].objective == tabu[i]) {
                    is_tabu = true;
                    break;
                }
            }
            if(is_tabu) {
                fprintf(csv_operations, "1,0\n");
                continue;
            }
            fprintf(csv_operations, "0,%lld\n", new_sols[local_counter].objective - sol.objective);

            long long updated_obj = new_sols[local_counter].objective;

            if(updated_obj != new_sols[local_counter].objective) {
                print_problem(&problem, &new_sols[local_counter], "dot/broken_sol.dot", "broken");
                return 0;
            }

            local_counter++;
        }
        qsort(new_sols, NEIGHBORHOOD_SIZE, sizeof(Solution), cmpfunc);

        Solution *selected_solution = &new_sols[0];

        /*Solution *selected_solution;
        if(new_sols[0].objective > sol.objective) {
            selected_solution = &new_sols[0];
        }
        else {
            int num_improving = 0;
            for (; num_improving < NEIGHBORHOOD_SIZE; ++num_improving) {
                if(new_sols[num_improving].objective > sol.objective) {
                    break;
                }
            }
            int selected_index = random_1_over_square(num_improving);
            printf("%d\n", selected_index);
            selected_solution = &new_sols[selected_index];
        }*/
        copy_solution(&problem, selected_solution, &sol);
        tabu[tabu_pos] = sol.objective;
        tabu_pos = (tabu_pos + 1) % TABU_SIZE;
        if(sol.objective < overall_best.objective) {
            copy_solution(&problem, &sol, &overall_best);
            overall_best_iter = iteration;
        }
        fprintf(csv_objective, "%d, %lld\n", iteration, sol.objective);
        if(TO_DOT) {
            sprintf(dot_filename, "dot/update%04d.dot", iteration);
            sprintf(dot_title, "objective %lld, accepting", sol.objective);
            print_problem(&problem, &sol, dot_filename, dot_title);
        }
        iteration++;
    }

    if(TO_DOT)
        print_problem(&problem, &overall_best, "dot/solution.dot", "final");

    printf("best objective: %lld\n", overall_best.objective);
    recalculate_objective(&overall_best, &problem);
    printf("best objective: %lld\n", overall_best.objective);
    analyze_solution(&overall_best, &problem);
    destroy_solution(&problem, &sol);
    destroy_solution(&problem, &overall_best);
    for (int i = 0; i < NEIGHBORHOOD_SIZE; ++i) {
        destroy_solution(&problem, new_sols + i);
    }
    destroy_problem(&problem);
    fclose(csv_operations);
    fclose(csv_objective);
}

int main() {
    srand(SEED);
    Problem problem;
    //parse("../../small_data_2_ts.cfg", &problem);
    parse("../../big_data_2_ts.cfg", &problem);

    local_search(&problem);

    return 0;
}