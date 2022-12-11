//
// Created by Ivana Krumlová on 24.02.22.
//

#include "datatypes.h"
#include "stdio.h"

void print_problem(Problem* problem, Solution *sol, const char* output_filename, const char *title) {
    char colors[3][16] = {"red", "blue", "green"};
    FILE *file = fopen(output_filename, "w");
    fprintf(file, "digraph G {\n\tgraph [rankdir = LR]\n\tlabel = \"%s\"\n", title);

    for (int station_id = 0; station_id < problem->num_stations; station_id++) {
        fprintf(file, "\tsubgraph cluster_%d {\n\t\tstyle=filled;\n\t\tcolor=white;\n\t\tlabel = %d\n", station_id, station_id);
        for (int node_i = 0; node_i < problem->num_nodes; node_i++) {
            if (problem->nodes[node_i].station->id == station_id) {
                fprintf(file, "\t\t%d;\n", problem->nodes[node_i].id);
            }
        }
        fprintf(file, "\t}\n");
    }

    for(int i = 0; i<problem->num_edges; i++) {
        if(problem->edges[i].start_node == NULL || problem->edges[i].end_node == NULL) { //workaround for unused edges
            continue;
        }

        fprintf(file, "\t%d -> %d", problem->edges[i].start_node->id, problem->edges[i].end_node->id);
        if(problem->edges[i].type == SUBCONNECTION)
            fprintf(file, "[style=dotted]");
        fprintf(file, ";\n");

        int capacity = 0;
        for(int ts = 0; ts < problem->num_trainset_types; ts++) {
            for(int j = 0; j < sol->edge_solution[i].num_trainsets[ts]; j++) {
                fprintf(file, "\t%d -> %d [color=%s];\n", problem->edges[i].start_node->id, problem->edges[i].end_node->id, colors[ts]);
            }
            capacity += problem->trainset_types[ts].seats * sol->edge_solution[i].num_trainsets[ts];
        }
        if(capacity < problem->edges[i].minimal_capacity) {
            fprintf(file, "\t%d -> %d [penwidth=3];\n", problem->edges[i].start_node->id, problem->edges[i].end_node->id);
        }

        if(capacity > problem->edges[i].minimal_capacity + problem->trainset_types[1].seats - problem->trainset_types[0].seats) {
            fprintf(file, "\t%d -> %d [penwidth=3; style=dotted];\n", problem->edges[i].start_node->id, problem->edges[i].end_node->id);
        }
    }
    fprintf(file, "}");
    fclose(file);
}

