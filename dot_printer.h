//
// Created by fitli on 25.02.22.
//

#ifndef TSCP_SOLVER_C_DOT_PRINTER_H
#define TSCP_SOLVER_C_DOT_PRINTER_H

#include "datatypes.h"

/**
 * Print solution in .dot format
 * @param problem
 * @param sol
 * @param output_filename
 * @param title title printed in the output
 */
void print_problem(Problem* problem, Solution *sol, const char* output_filename, const char *title);

#endif //TSCP_SOLVER_C_DOT_PRINTER_H
