//
// Created by Ivana Krumlová on 02.01.22.
//

#ifndef TSCP_SOLVER_C_DATATYPES_H
#define TSCP_SOLVER_C_DATATYPES_H

#include <time.h>

typedef enum edge_type EdgeType;
typedef struct Edge Edge;
typedef struct Node Node;
typedef struct Station Station;
typedef struct Trainset Trainset;
typedef struct Connection Connection;

enum edge_type {
    SUBCONNECTION,
    WAITING,
    SOURCE_EDGE,
    SINK_EDGE
};

struct Station {
    int id;
    Node *source_node;
    Node *sink_node;
    int num_nodes;
    int *node_ids;
};

struct Trainset {
    int id;

    int available; // availability of trainset
    int seats;  // number of seats
    int investment;  // purchase cost
    int el_cost; // cost electricity for 1 km
    int fe_cost; // cost for fee for using rail tracks for 1 km
    int re_cost; // cost for maintenance for 1 km
    int abroad_gain;  // gains for one km abroad
};

struct Edge {
    int id;

    Node *start_node;
    Node *end_node;

    int minimal_capacity;
    int distance;
    int distance_abroad;
    EdgeType type;
};

struct Node {
    int id;

    Edge *in_waiting;
    Edge *in_subcon;
    Edge *out_waiting;
    Edge *out_subcon;

    Station* station;
    int time; // order of the node between all nodes of the station
};

struct Problem {
    int num_stations; //number of stations
    int num_edges; // number of edges
    int num_inner_nodes; // number of edges
    int num_nodes; // number of nodes: num_inner_nodes + 2*num_stations
    int num_trainset_types; // number of available trainset types

    int max_trainset_types; //maximum number of different trainset types used in solution
    int max_len; // maximum number of trainset in one train
    int max_cap; // maximum capacity of a train

    int time_horizon; // Time horizon used for calculating operational cost and abroad gain
    double mod_cost; // Modernization coefficient used for calculating investment cost

    Trainset *trainset_types; //array of all avalable trainset types
    Node *nodes; // array of all nodes
    Station* stations; // array of all stations
    Edge* edges; //array of all subconections
};
typedef struct Problem Problem;

struct EdgeSolution {
    int* num_trainsets;
    int capacity;
};
typedef struct EdgeSolution EdgeSolution;

struct Solution {
    EdgeSolution *edge_solution;
    long long int objective;
    int *num_trainstes;
    int *cap_can_add;
    int *cap_can_remove;
};
typedef struct Solution Solution;

/**
 * Free all dynamic arrays of a problem
 * @param problem
 */
void destroy_problem(Problem *problem);

/**
 * Creates an ampty solution with no trainsets
 * @param problem
 * @param[out] sol
 * @return EXIT_SUCCESS if a solution is created, EXIT_FAILURE in case of allocation error
 */
int empty_solution(Problem *problem, Solution *sol);

/**
 * Copy solution from `orig_sol` to `dest_sol`
 * @param problem
 * @param orig_sol
 * @param dest_sol
 */
void copy_solution(Problem *problem, Solution *orig_sol, Solution *dest_sol);

/**
 * Free all dynamic arrays of a solution `sol`
 * @param problem
 * @param sol
 */
void destroy_solution(Problem *problem, Solution *sol);

#endif //TSCP_SOLVER_C_DATATYPES_H
