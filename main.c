#include <stdio.h>
#include <stdlib.h>
#include "datatypes.h"
#include "parse_config.h"
#include "solution_modifier.h"
#include "heuristics.h"

int main() {
    Problem problem;
    parse("../../small_data.cfg", &problem);
    Solution sol;

    empty_solution(&problem, &sol);
    printf("objectiive: %lld hard-objective: %lld\n", sol.objective, sol.hard_objective);
    Edge **edges;
    int num_edges;
    for(int i = 0; i < 20; i++) {
        int station_id = select_station_first_empty_departure(&problem, &sol);
        if(station_id == -1) {
            break;
        }
        printf("add a train beginning in %d\n", station_id);
        add_train_two_side(&sol, &problem, &problem.trainset_types[0], &problem.stations[station_id],
                           &edges, &num_edges);
        printf("objectiive: %lld hard-objective: %lld\n", sol.objective, sol.hard_objective);
        free(edges);
    }


    destroy_solution(&problem, &sol);
    destroy_problem(&problem);
    return 0;
}
