//
// Created by Ivana Krumlová on 16.10.22.
//

#ifndef TSCP_SOLVER_OUTPUT_H
#define TSCP_SOLVER_OUTPUT_H

#include "datatypes.h"

/**
 * Prints information about the solution
 * @param sol
 * @param problem
 */
void analyze_solution(Solution *sol, Problem *problem);

void sol_to_csv(Solution *sol, Problem *problem, const char* filename);

#endif //TSCP_SOLVER_OUTPUT_H
