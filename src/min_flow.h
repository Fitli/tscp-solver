//
// Created by Ivana Krumlová on 03.11.22.
//

#include "datatypes.h"

#ifndef TSCP_SOLVER_MIN_FLOW_H
#define TSCP_SOLVER_MIN_FLOW_H

/**
 * Finds optimal solution using only one trainset types
 * @param problem
 * @param ts trainset type
 * @return
 */
Solution min_flow(Problem *problem, Trainset *ts);

#endif //TSCP_SOLVER_MIN_FLOW_H
