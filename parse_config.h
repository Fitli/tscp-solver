//
// Created by fitli on 10.03.22.
//

#ifndef TSCP_SOLVER_C_PARSE_CONFIG_H
#define TSCP_SOLVER_C_PARSE_CONFIG_H

#include "datatypes.h"

int parse(const char* file, Problem *problem);
Solution read_sol_from_csv(Problem *problem, char *filename);

#endif //TSCP_SOLVER_C_PARSE_CONFIG_H