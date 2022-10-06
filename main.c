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

#define TO_DOT 1
#define IMPROVE_RATIO 1.1
#define NUM_LOCAL_CHANGES 100
#define NUM_RESTART_BEST 1000
#define TABU_SIZE 10

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
            oper_move_edge_front(sol, problem, rand_edge, rand_ts);
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
        default:
            operation_name = "reschedule_n_w";
            oper_reschedule_n_w(sol, problem, rand_start_node, rand_end_node, rand_ts);
            break;
    }
    if(operation_data) {
        fprintf(operation_data, "%s,", operation_name);
    }
    printf("%s\n", operation_name);
}

int main() {
    srand(1);
    Problem problem;
    parse("../../small_data_2_ts.cfg", &problem);
    Solution sol;
    char dot_filename[255];
    char dot_title[255];

    FILE *csv_objective = fopen("objective.csv", "w");
    fprintf(csv_objective, "iter,obj\n");
    FILE *csv_operations = fopen("operations.csv", "w");
    fprintf(csv_operations, "iter,operation,tabu,obj_change\n");

    empty_solution(&problem, &sol);
    printf("objective: %lld\n", sol.objective);
    /*for(int i = 0; i < 1000; i++) {
        printf("iteration %d:\n", i);
        int station_id = -1;
        if(i%2 == 0) {
            station_id = select_station_first_empty_departure(&problem, &sol);
        }else{
            station_id = select_station_last_empty_arrival(&problem, &sol);
        }

        if(station_id == -1) {
            break;
        }
        printf("add a train beginning in %d\n", station_id);

        oper_add_train_to_empty(&sol, &problem, station_id);

        if(TO_DOT) {
            sprintf(dot_filename, "dot/init%03d.dot", i);
            sprintf(dot_title, "initial adding to %d", station_id);
            print_problem(&problem, &sol, dot_filename, dot_title);
        }

        if(!test_consistency(&problem, &sol)) {
            break;
        }

        printf("objective: %lld\n", sol.objective);
        int empty_subcons = 0;
        for (int j = 0; j < problem.num_edges; j++) {
            if(problem.edges[j].type == SUBCONNECTION && sol.edge_solution[j].capacity == 0) {
                empty_subcons++;
                if(i > 998) {
                    printf("st %d t %ld ->st %d t %ld \n", problem.edges[j].start_node->station->id, problem.edges[j].start_node->time, problem.edges[j].end_node->station->id, problem.edges[j].end_node->time);
                }
            }
        }
        printf("num_empty = %d\n", empty_subcons);
    }*/

    int iteration = 0;
    long long int tabu[TABU_SIZE];
    tabu[1] = sol.objective;
    for (int i = 1; i < TABU_SIZE; ++i) {
        tabu[i] = 0;
    }
    int tabu_pos = 1;
    Solution new_sols [NUM_LOCAL_CHANGES];
    for(int i = 0; i < NUM_LOCAL_CHANGES; i++) {
        empty_solution(&problem, new_sols + i);
    }
    Solution overall_best;
    empty_solution(&problem, &overall_best);
    copy_solution(&problem, &sol, &overall_best);
    while(iteration < NUM_RESTART_BEST) {
        int local_counter = 0;
        while (local_counter < NUM_LOCAL_CHANGES) {
            fprintf(csv_operations, "%d,", iteration);

            copy_solution(&problem, &sol, new_sols + local_counter);
            do_random_operation(&problem, new_sols + local_counter, csv_operations);

            if(!test_consistency(&problem, new_sols + local_counter)) {
                printf("unconsistent\n");
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
        qsort(new_sols, NUM_LOCAL_CHANGES, sizeof(Solution), cmpfunc);

        Solution *selected_solution = &new_sols[0];
        //Solution *selected_solution = &new_sols[random_1_over_square(NUM_LOCAL_CHANGES)];
        copy_solution(&problem, selected_solution, &sol);
        tabu[tabu_pos] = sol.objective;
        tabu_pos = (tabu_pos + 1) % TABU_SIZE;
        if(sol.objective < overall_best.objective) {
            copy_solution(&problem, &sol, &overall_best);
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
    destroy_solution(&problem, &sol);
    destroy_solution(&problem, &overall_best);
    for (int i = 0; i < NUM_LOCAL_CHANGES; ++i) {
        destroy_solution(&problem, new_sols + i);
    }
    destroy_problem(&problem);
    fclose(csv_operations);
    fclose(csv_objective);
    return 0;
}


int main2() {
    srand(3);
    Problem problem;
    parse("../../small_data.cfg", &problem);
    Solution sol;
    char filename[255];
    char title[255];

    empty_solution(&problem, &sol);
    printf("objective: %lld\n", sol.objective);
    for(int i = 0; i < 1000; i++) {
        printf("iteration %d:\n", i);
        int station_id = -1;
        if(i%2 == 0) {
            station_id = select_station_first_empty_departure(&problem, &sol);
        }else{
            station_id = select_station_last_empty_arrival(&problem, &sol);
        }

        if(station_id == -1) {
            break;
        }
        printf("add a train beginning in %d\n", station_id);

        oper_add_train_to_empty(&sol, &problem, station_id);

        if(TO_DOT) {
            sprintf(filename, "dot/init%03d.dot", i);
            sprintf(title, "initial adding to %d", station_id);
            print_problem(&problem, &sol, filename, title);
        }

        if(!test_consistency(&problem, &sol)) {
            break;
        }

        printf("objective: %lld\n", sol.objective);
        int empty_subcons = 0;
        for (int j = 0; j < problem.num_edges; j++) {
            if(problem.edges[j].type == SUBCONNECTION && sol.edge_solution[j].capacity == 0) {
                empty_subcons++;
                if(i > 998) {
                    printf("st %d t %ld ->st %d t %ld \n", problem.edges[j].start_node->station->id, problem.edges[j].start_node->time, problem.edges[j].end_node->station->id, problem.edges[j].end_node->time);
                }
            }
        }
        printf("num_empty = %d\n", empty_subcons);
    }

    int local_counter = NUM_LOCAL_CHANGES;
    int back_to_best_counter = NUM_RESTART_BEST;
    int iteration = 0;
    Solution new_sol;
    empty_solution(&problem, &new_sol);
    Solution best_sol;
    empty_solution(&problem, &best_sol);
    copy_solution(&problem, &sol, &best_sol);
    while(back_to_best_counter > 0) {
        copy_solution(&problem, &sol, &new_sol);
        do_random_operation(&problem, &new_sol, NULL);
        if(!test_consistency(&problem, &new_sol)) {
            printf("unconsistent\n");
            break;
        }
        if(new_sol.objective < best_sol.objective * IMPROVE_RATIO && new_sol.objective != sol.objective) {
            //if(new_sol.objective > sol.objective) { //vždy zlepšení
            copy_solution(&problem, &new_sol, &sol);
            //local_counter = 1000;
            printf("objective: %lld\n", sol.objective);

            if(new_sol.objective < best_sol.objective) {
                copy_solution(&problem, &new_sol, &best_sol);
                back_to_best_counter = NUM_RESTART_BEST;
                local_counter = NUM_LOCAL_CHANGES;

                if(TO_DOT) {
                    sprintf(filename, "dot/update%04d.dot", iteration);
                    sprintf(title, "objective %lld, accepting", sol.objective);
                    print_problem(&problem, &sol, filename, title);
                }
            }
            local_counter--;
        }
        else {
            local_counter--;
            //if(TO_DOT) {
            //sprintf(filename, "dot/update%04d.dot", iteration);
            //sprintf(title, "objective %lld, not accepting", new_sol.objective);
            //print_problem(&problem, &new_sol, filename, title);
            //}
        }
        if(local_counter <= 0) {
            copy_solution(&problem, &best_sol, &sol);
            back_to_best_counter--;
            local_counter = NUM_LOCAL_CHANGES;
        }
        iteration++;
    }

    if(TO_DOT)
        print_problem(&problem, &sol, "dot/solution.dot", "final");

    printf("final objective: %lld\n", sol.objective);
    destroy_solution(&problem, &sol);
    destroy_solution(&problem, &new_sol);
    destroy_problem(&problem);
    return 0;
}