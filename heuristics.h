//
// Created by fitli on 06.04.22.
//

#ifndef TSCP_SOLVER_HEURISTICS_H
#define TSCP_SOLVER_HEURISTICS_H

#include "datatypes.h"

int select_station_max_empty_subcons(Problem *problem, Solution *solution);
int select_station_first_empty_departure(Problem *problem, Solution *solution);

#endif //TSCP_SOLVER_HEURISTICS_H
