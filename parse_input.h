//
// Created by Ivana Krumlová on 10.03.22.
//

#ifndef TSCP_SOLVER_C_PARSE_CONFIG_H
#define TSCP_SOLVER_C_PARSE_CONFIG_H

#include "datatypes.h"

/**
 * Perse problem specification in libconfig format
 * @param filename
 * @param problem
 * @return
 */
int parse_problem(const char* filename, Problem *problem);
/**
 * Read solution from csv
 * @param problem
 * @param filename
 * @return
 */
Solution read_sol_from_csv(Problem *problem, char *filename);

#endif //TSCP_SOLVER_C_PARSE_CONFIG_H
