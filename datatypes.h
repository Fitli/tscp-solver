//
// Created by fitli on 02.01.22.
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
    char name[256];
    int id;
    Edge *source_edge;
    Edge *sink_edge;
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

struct Connection {
    Station* start_station;
    Station* end_station;

    time_t start_time;
    time_t end_time;

    struct Edge** subconections;
};

struct Node {
    int id;

    Edge *in_waiting;
    Edge *in_subcon;
    Edge *out_waiting;
    Edge *out_subcon;

    Station* station;
    time_t time;
};

struct Problem {
    time_t cycle_length;
    int num_stations; //number of stations - without source and sink
    int num_edges;
    int num_nodes;
    int num_conections;
    int num_trainset_types;

    int max_trainset_types;
    int max_len;
    int max_cap;

    int time_horizon;
    double mod_cost;

    Trainset *trainset_types;
    Node *nodes;
    Station* stations; // array of all stations [0] is source, [1]-[num_stations] regular stations, [num_stations + 1] is sink
    Edge* edges; //array of all subconections
    Connection* connections; // array of all connections
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
    long long int hard_objective;
};
typedef struct Solution Solution;

void destroy_problem(Problem *problem);
int empty_solution(Problem *problem, Solution *sol);
void destroy_solution(Problem *problem, Solution *sol);

#endif //TSCP_SOLVER_C_DATATYPES_H
