//
// Created by fitli on 28.04.22.
//

#include "datatypes.h"

#ifndef TSCP_SOLVER_TEST_H
#define TSCP_SOLVER_TEST_H

/**
 * Tests if the flow and ballance in the network are valid
 * @param problem
 * @param solution
 * @return
 */
int test_consistency(Problem *problem, Solution *solution);
/**
 * Tests if current objective is equal to a recalculated one
 * @param problem
 * @param solution
 * @return
 */
int test_objective(Problem *problem, Solution *solution);

#endif //TSCP_SOLVER_TEST_H
