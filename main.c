#include <stdio.h>
#include "datatypes.h"
#include "parse_config.h"
#include "solution_modifier.h"

int main() {
    Problem problem;
    parse("../../small_data.cfg", &problem);
    Solution sol;

    empty_solution(&problem, &sol);
    add_train_two_side(&sol, &problem, &problem.trainset_types[0], &problem.stations[0], first_empty);

    destroy_solution(&problem, &sol);
    destroy_problem(&problem);
    return 0;
}
