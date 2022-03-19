#include <stdio.h>
#include "datatypes.h"
#include "parse_config.h"

int main() {
    Problem problem;
    parse("../../small_data.cfg", &problem);
    Solution sol;
    empty_solution(&problem, &sol);
    destroy_solution(&problem, &sol);
    destroy_problem(&problem);
    return 0;
}
