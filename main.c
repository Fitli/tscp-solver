#include <stdio.h>
#include <stdlib.h>
#include "datatypes.h"
#include "parse_config.h"
#include "solution_modifier.h"
#include "heuristics.h"

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

int main() {
    Problem problem;
    parse("../../big_data.cfg", &problem);
    Solution sol;

    empty_solution(&problem, &sol);
    printf("objective: %lld hard-objective: %lld\n", sol.objective, sol.hard_objective);
    Edge **edges;
    int num_edges;
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
        add_train_two_side(&sol, &problem, &problem.trainset_types[0], &problem.stations[station_id],
                           &edges, &num_edges);
        printf("objective: %lld hard-objective: %lld\n", sol.objective, sol.hard_objective);
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
        free(edges);
    }


    //destroy_solution(&problem, &sol);
    //destroy_problem(&problem);
    return 0;
}
