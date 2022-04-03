#include <stdio.h>
#include "datatypes.h"
#include "parse_config.h"
#include "solution_modifier.h"

int main() {
    Problem problem;
    parse("../../small_data.cfg", &problem);
    Solution sol;

    empty_solution(&problem, &sol);
    printf("objectiive: %lld\n", sol.objective);
    printf("add a train beginning in 0\n");
    add_train_two_side(&sol, &problem, &problem.trainset_types[0], &problem.stations[0], first_empty);
    printf("objectiive: %lld\n", sol.objective);
    printf("add a train beginning in 1\n");
    add_train_two_side(&sol, &problem, &problem.trainset_types[0], &problem.stations[1], first_empty);
    printf("objectiive: %lld\n", sol.objective);
    printf("add a train beginning in 3\n");
    add_train_two_side(&sol, &problem, &problem.trainset_types[0], &problem.stations[3], first_empty);
    printf("objectiive: %lld\n", sol.objective);
    printf("add a train beginning in 2\n");
    add_train_two_side(&sol, &problem, &problem.trainset_types[0], &problem.stations[2], first_empty);
    printf("objectiive: %lld\n", sol.objective);


    destroy_solution(&problem, &sol);
    destroy_problem(&problem);
    return 0;
}
