#include <stdio.h>
#include <stdlib.h>
#include "datatypes.h"
#include "parse_config.h"
#include "solution_modifier.h"
#include "heuristics.h"
#include "change_finder.h"
#include "test.h"
#include "actions.h"
#include "dot_printer.h"

#define TO_DOT 0

void print_in_out(Problem *problem) {
    for (int i = 0; i < problem->num_stations; i++) {
        printf("%d:", i);
        Node *node = problem->stations[i].source_edge->end_node;
        while(node != NULL) {
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
    int r = rand() % 6;
    int rand_st = rand() % problem->num_stations;
    int rand_edge = rand() % problem->num_edges;
    int rand_ts = rand() % problem->num_trainset_types;
    int rand_ts2 = rand() % problem->num_trainset_types;
    switch(r) {
        case 0:
            printf("add train\n");
            act_add_train_later(sol, problem, rand_st);
            return;
        case 1:
            printf("remove train\n");
            act_remove_train(sol, problem, rand_st, rand_ts);
            return;
        case 2:
            printf("remove waiting train\n");
            act_remove_waiting_train(sol, problem, rand_st, rand_ts);
            return;
        case 3:
            printf("change train\n");
            act_change_train_capacity(sol, problem, rand_st, rand_ts, rand_ts2);
            return;
        case 4:
            printf("move edge back\n");
            act_move_edge_back(sol, problem, rand_edge, rand_ts);
            return;
        case 5:
            printf("move edge front\n");
            act_move_edge_front(sol, problem, rand_edge, rand_ts);
            return;
    }
}

int main() {
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

        act_add_train_to_empty(&sol, &problem, station_id);

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

    int counter = 1000;
    int iteration = 0;
    Solution new_sol;
    empty_solution(&problem, &new_sol);
    while(counter > 0) {
        copy_solution(&problem, &sol, &new_sol);
        do_random_action(&problem, &new_sol);
        if(!test_consistency(&problem, &new_sol)) {
            printf("unconsistent\n");
            break;
        }
        if(new_sol.objective > sol.objective) {
            copy_solution(&problem, &new_sol, &sol);
            counter = 1000;
            printf("objective: %lld\n", sol.objective);
            if(TO_DOT) {
                sprintf(filename, "dot/update%04d.dot", iteration);
                sprintf(title, "objective %lld, accepting", sol.objective);
                print_problem(&problem, &sol, filename, title);
            }
        }
        else {
            counter--;
            if(TO_DOT) {
                sprintf(filename, "dot/update%04d.dot", iteration);
                sprintf(title, "objective %lld, not accepting", new_sol.objective);
                //print_problem(&problem, &new_sol, filename, title);
            }
        }
        iteration++;
    }

    if(TO_DOT)
        print_problem(&problem, &sol, "dot/solution.dot", "final");

    destroy_solution(&problem, &sol);
    destroy_solution(&problem, &new_sol);
    destroy_problem(&problem);
    return 0;
}
