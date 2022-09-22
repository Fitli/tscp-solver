#include <stdio.h>
#include <stdlib.h>
#include "datatypes.h"
#include "parse_config.h"
#include "solution_modifier.h"
#include "heuristics.h"
#include "change_finder.h"
#include "test.h"
#include "operations.h"
#include "dot_printer.h"

#define TO_DOT 1
#define IMPROVE_RATIO 1.001
#define NUM_LOCAL_CHANGES 1000
#define NUM_RESTART_BEST 100

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

void do_random_action(Problem *problem, Solution *sol) {
    int r = rand() % 20;
    int rand_st = rand() % problem->num_stations;
    int rand_edge = rand() % problem->num_edges;
    int rand_start_node_index = rand() % (problem->stations[rand_st].num_nodes - 1);
    int rand_start_node = problem->stations[rand_st].node_ids[rand_start_node_index];
    int rand_end_node_index = rand_start_node_index + 1 + rand() % (problem->stations[rand_st].num_nodes - rand_start_node_index - 1);
    int rand_end_node = problem->stations[rand_st].node_ids[rand_end_node_index];
    int rand_ts = rand() % problem->num_trainset_types;
    int rand_ts2 = rand() % problem->num_trainset_types;
    switch(r) {
        case 0:
            printf("add train\n");
            oper_add_train_later(sol, problem, rand_st, rand_ts);
            return;
        case 1:
            printf("remove train\n");
            oper_remove_train(sol, problem, rand_st, rand_ts);
            return;
        case 2:
            printf("remove waiting train\n");
            oper_remove_waiting_train(sol, problem, rand_st, rand_ts);
            return;
        case 3:
            printf("change train\n");
            oper_change_train_capacity(sol, problem, rand_st, rand_ts, rand_ts2, 1, 1);
            return;
        case 4:
            printf("move edge back\n");
            oper_move_edge_back(sol, problem, rand_edge, rand_ts);
            return;
        case 5:
            printf("move edge front\n");
            oper_move_edge_front(sol, problem, rand_edge, rand_ts);
            return;
        case 6:
            printf("change train 2:1\n");
            oper_change_train_capacity(sol, problem, rand_st, rand_ts, rand_ts2, 2, 1);
            return;
        case 7:
            printf("change train 1:2\n");
            oper_change_train_capacity(sol, problem, rand_st, rand_ts, rand_ts2, 1, 2);
            return;
        case 8:
        case 9:
        case 10:
            printf("reschedule_w_l\n");
            oper_reschedule_w_l(sol, problem, rand_start_node, rand_end_node, rand_ts);
            return;
        case 11:
        case 12:
        case 13:
            printf("reschedule_n_l\n");
            oper_reschedule_n_l(sol, problem, rand_start_node, rand_end_node, rand_ts);
            return;
        case 14:
            printf("add_to_empty\n");
            oper_add_train_to_empty(sol, problem, rand_st);
            return;
        case 15:
            printf("add with edge\n");
            oper_add_train_with_edge(sol, problem, rand_edge, rand_ts);
            return;
        case 16:
            printf("remove with edge\n");
            oper_remove_train_with_edge(sol, problem, rand_edge, rand_ts);
            return;
        default:
            printf("reschedule_n_w\n");
            oper_reschedule_n_w(sol, problem, rand_start_node, rand_end_node, rand_ts);
            return;
    }
}

int main() {
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

    int local_counter = 0;
    int iteration = 0;
    Solution new_sol;
    empty_solution(&problem, &new_sol);
    Solution best_new;
    empty_solution(&problem, &best_new);
    while(iteration < NUM_RESTART_BEST) {
        local_counter = 0;
        while (local_counter < NUM_LOCAL_CHANGES) {
            copy_solution(&problem, &sol, &new_sol);
            do_random_action(&problem, &new_sol);

            if(!test_consistency(&problem, &new_sol)) {
                printf("unconsistent\n");
                break;
            }

            if(local_counter == 0 || new_sol.objective < best_new.objective) {
                copy_solution(&problem, &new_sol, &best_new);
            }

            local_counter++;
        }

        copy_solution(&problem, &best_new, &sol);
        if(TO_DOT) {
            sprintf(filename, "dot/update%04d.dot", iteration);
            sprintf(title, "objective %lld, accepting", sol.objective);
            print_problem(&problem, &sol, filename, title);
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
        do_random_action(&problem, &new_sol);
        if(!test_consistency(&problem, &new_sol)) {
            printf("unconsistent\n");
            break;
        }
        if(new_sol.objective < best_sol.objective * IMPROVE_RATIO) {
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