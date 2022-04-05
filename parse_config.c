//
// Created by fitli on 10.03.22.
//

#include "parse_config.h"
#include <libconfig.h>
#include <stdlib.h>

int set_array_field_from_config(config_t *cfg, Problem *problem,
                                const char * path, int expected_length,
                                void(*setter)(Problem *, int, int)) {
    config_setting_t *setting;
    setting = config_lookup(cfg, path);
    if (setting == NULL) {
        fprintf(stderr, "No %s setting in configuration file.\n", path);
        return EXIT_FAILURE;
    }

    if (config_setting_length(setting) != expected_length) {
        fprintf(stderr, "setting %s has incorect length.\n", path);
        return EXIT_FAILURE;
    }

    for(int i = 0; i < expected_length; i++) {
        setter(problem, i, config_setting_get_int_elem(setting, i));
    }
    return EXIT_SUCCESS;
}

/*
 * LOAD PROBLEM CONSTANTS
 */
int load_constants(config_t *cfg, Problem *problem) {
    if(!config_lookup_int(cfg, "max_trainset_types", &problem->max_trainset_types)) {
        fprintf(stderr, "No 'max_trainset_types' setting in configuration file.\n");
        return EXIT_FAILURE;
    }

    if(!config_lookup_int(cfg, "max_len", &problem->max_len)) {
        fprintf(stderr, "No 'max_len' setting in configuration file.\n");
        return EXIT_FAILURE;
    }

    if(!config_lookup_int(cfg, "max_cap", &problem->max_cap)) {
        fprintf(stderr, "No 'max_cap' setting in configuration file.\n");
        return EXIT_FAILURE;
    }

    if(!config_lookup_int(cfg, "time_horizon", &problem->time_horizon)) {
        fprintf(stderr, "No 'time_horizon' setting in configuration file.\n");
        return EXIT_FAILURE;
    }

    if(!config_lookup_float(cfg, "mod_cost", &problem->mod_cost)) {
        fprintf(stderr, "No 'mod_cost' setting in configuration file.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/*
 * LOADING TRAINSETS
 */
void set_ts_available(Problem *problem, int ts_idx, int value) {
    problem->trainset_types[ts_idx].available = value;
}

void set_ts_seats(Problem *problem, int ts_idx, int value) {
    problem->trainset_types[ts_idx].seats = value;
}

void set_ts_investment(Problem *problem, int ts_idx, int value) {
    problem->trainset_types[ts_idx].investment = value;
}

void set_ts_el_cost(Problem *problem, int ts_idx, int value) {
    problem->trainset_types[ts_idx].el_cost = value;
}

void set_ts_fe_cost(Problem *problem, int ts_idx, int value) {
    problem->trainset_types[ts_idx].fe_cost = value;
}

void set_ts_re_cost(Problem *problem, int ts_idx, int value) {
    problem->trainset_types[ts_idx].re_cost = value;
}

void set_ts_abroad_gain(Problem *problem, int ts_idx, int value) {
    problem->trainset_types[ts_idx].abroad_gain = value;
}

int create_trainsets(config_t *cfg, Problem *problem) {
    //load nuber of trainset types
    if(!config_lookup_int(cfg, "n_trainset_types", &problem->num_trainset_types)) {
        fprintf(stderr, "No 'n_trainset_types' setting in configuration file.\n");
        return EXIT_FAILURE;
    }

    // alocate space for trainset types
    problem->trainset_types = malloc(problem->num_trainset_types * sizeof(Trainset));
    if (problem->trainset_types == NULL) {
        fprintf(stderr, "Allocation error.\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < problem->num_trainset_types; i++) {
        problem->trainset_types[i].id = i;
    }

    // Set fields for each trainset
    if(set_array_field_from_config(cfg, problem, "t_available", problem->num_trainset_types, set_ts_available) != EXIT_SUCCESS ||
       set_array_field_from_config(cfg, problem, "t_seats", problem->num_trainset_types, set_ts_seats) != EXIT_SUCCESS ||
       set_array_field_from_config(cfg, problem, "t_investment", problem->num_trainset_types, set_ts_investment) != EXIT_SUCCESS ||
       set_array_field_from_config(cfg, problem, "el_cost", problem->num_trainset_types, set_ts_el_cost) != EXIT_SUCCESS ||
       set_array_field_from_config(cfg, problem, "fe_cost", problem->num_trainset_types, set_ts_fe_cost) != EXIT_SUCCESS ||
       set_array_field_from_config(cfg, problem, "re_cost", problem->num_trainset_types, set_ts_re_cost) != EXIT_SUCCESS ||
       set_array_field_from_config(cfg, problem, "abroad_gain", problem->num_trainset_types, set_ts_abroad_gain) != EXIT_SUCCESS
       ) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/*
 * LOADING EDGES
*/
void set_edge_passengers(Problem *problem, int edge_idx, int value) {
    problem->edges[edge_idx].minimal_capacity = value;
}

void set_edge_distance(Problem *problem, int edge_idx, int value) {
    problem->edges[edge_idx].distance = value;
}

void set_edge_abroad(Problem *problem, int edge_idx, int value) {
    problem->edges[edge_idx].distance_abroad = value;
}

int create_edges(config_t *cfg, Problem *problem) {
    // Load numbers of edges
    int n_subconn;
    if(!config_lookup_int(cfg, "n_subconn", &n_subconn)) {
        fprintf(stderr, "No 'n_subconn' setting in configuration file.\n");
        return EXIT_FAILURE;
    }
    if(!config_lookup_int(cfg, "n_edges", &problem->num_edges)) {
        fprintf(stderr, "No 'n_edges' setting in configuration file.\n");
        return EXIT_FAILURE;
    }
    if(n_subconn > problem->num_edges) {
        fprintf(stderr, "Nuber of subconnections is bigger then number of edges.\n");
        return EXIT_FAILURE;
    }

    // alocate space for edges
    problem->edges = malloc(problem->num_edges * sizeof(Edge));
    if (problem->edges == NULL) {
        fprintf(stderr, "Allocation error.\n");
        return EXIT_FAILURE;
    }

    //set id
    for (int i = 0; i < problem->num_edges; i++) {
        problem->edges[i].id = i;
    }

    // set subconnection edges type
    for (int i = 0; i<n_subconn; i++) {
        problem->edges[i].type = SUBCONNECTION;
    }
    // set the rest as waiting for now
    for (int i = n_subconn; i<problem->num_edges; i++) {
        problem->edges[i].type = WAITING;
        problem->edges[i].minimal_capacity = 0;
    }

    if(set_array_field_from_config(cfg, problem, "pass_subconn", n_subconn, set_edge_passengers) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if(set_array_field_from_config(cfg, problem, "distances_subconn", n_subconn, set_edge_distance) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if(set_array_field_from_config(cfg, problem, "distance_abroad", n_subconn, set_edge_abroad) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/*
 * LOADING NODES
*/
int create_nodes(config_t *cfg, Problem *problem) {
    config_setting_t *in_nodes;
    in_nodes = config_lookup(cfg, "in_nodes");
    if (in_nodes == NULL) {
        fprintf(stderr, "No 'in_nodes' setting in configuration file.\n");
        return EXIT_FAILURE;
    }

    config_setting_t *out_nodes;
    out_nodes = config_lookup(cfg, "out_nodes");
    if (out_nodes == NULL) {
        fprintf(stderr, "No 'out_nodes' setting in configuration file.\n");
        return EXIT_FAILURE;
    }

    problem->num_nodes = config_setting_length(in_nodes) + config_setting_length(out_nodes);

    // alocate space for edges
    problem->nodes = malloc(problem->num_nodes * sizeof(Node));
    if (problem->nodes == NULL) {
        fprintf(stderr, "Allocation error.\n");
        return EXIT_FAILURE;
    }

    // set id
    for (int i = 0; i < problem->num_nodes; i++) {
        problem->nodes[i].id = i;
    }

    // input nodes
    int num_in = config_setting_length(in_nodes);
    for (int i = 0; i<num_in; i++) {
        config_setting_t *in_node = config_setting_get_elem(in_nodes, i);
        if (config_setting_length(in_node) != 3) {
            fprintf(stderr, "Node in 'in_nodes' has incorrect length.\n");
            return EXIT_FAILURE;
        }
        int win = config_setting_get_int_elem(in_node, 0) - 1;
        int wout = config_setting_get_int_elem(in_node, 1) - 1;
        int sin = config_setting_get_int_elem(in_node, 2) - 1;

        problem->nodes[i].in_waiting = &problem->edges[win];
        problem->edges[win].end_node = &problem->nodes[i];

        problem->nodes[i].out_waiting = &problem->edges[wout];
        problem->edges[wout].start_node = &problem->nodes[i];

        problem->nodes[i].in_subcon = &problem->edges[sin];
        problem->edges[sin].end_node = &problem->nodes[i];

        problem->nodes[i].out_subcon = NULL;
    }
    // out nodes
    int idx = num_in;
    for (int i = 0; i<config_setting_length(out_nodes); i++) {
        config_setting_t *out_node = config_setting_get_elem(out_nodes, i);
        if (config_setting_length(out_node) != 3) {
            fprintf(stderr, "Node in 'out_nodes' has incorrect length.\n");
            return EXIT_FAILURE;
        }

        int win = config_setting_get_int_elem(out_node, 0) - 1;
        int wout = config_setting_get_int_elem(out_node, 1) - 1;
        int sout = config_setting_get_int_elem(out_node, 2) - 1;

        problem->nodes[idx].in_waiting = &problem->edges[win];
        problem->edges[win].end_node = &problem->nodes[idx];

        problem->nodes[idx].out_waiting = &problem->edges[wout];
        problem->edges[wout].start_node = &problem->nodes[idx];

        problem->nodes[idx].in_subcon = NULL;

        problem->nodes[idx].out_subcon = &problem->edges[sout];
        problem->edges[sout].start_node = &problem->nodes[idx];

        idx++;
    }
    //TODO: přidat časy?

    return EXIT_SUCCESS;
}

int assign_stations(Problem *problem) {
    for(int i = 0; i < problem->num_stations; i++) {
        fprintf(stderr,"stanice %d:\n", i);
        Station *station = &(problem->stations[i]);
        Node *node = station->source_edge->end_node;
        time_t time = 0;
        do {
            fprintf(stderr,"n%d ", node->id);
            node->station = station;
            node->time = time;
            time++;
            fprintf(stderr,"e%d ", node->out_waiting->id);
            node = node->out_waiting->end_node;
        }
        while (node->out_waiting->end_node != NULL && node->out_waiting->type != SINK_EDGE);
        fprintf(stderr,"\n");
        if (node->out_waiting != station->sink_edge) {
            fprintf(stderr, "Unconsistent waiting nodes.\n");
            return EXIT_FAILURE;
        }
        node->station = station;
        node->time = time;
    }
    return EXIT_SUCCESS;
}

/*
 * LOADING STATIONS
*/
int create_stations(config_t *cfg, Problem *problem) {
    if(!config_lookup_int(cfg, "n_stations", &problem->num_stations)) {
        fprintf(stderr, "No 'n_stations' setting in configuration file.\n");
        return EXIT_FAILURE;
    }

    //alocate stations
    problem->stations = malloc(problem->num_stations * sizeof(Station));
    if (problem->stations == NULL) {
        fprintf(stderr, "Allocation error.\n");
        return EXIT_FAILURE;
    }

    //set id
    for(int i = 0; i < problem->num_stations; i++) {
        problem->stations[i].id = i;
    }
    //TODO: přidat názvy stanic??
    return EXIT_SUCCESS;
}

void set_source_edge(Problem *problem, int st_idx, int edge_idx) {
    problem->stations[st_idx].source_edge = &problem->edges[edge_idx - 1];
    problem->edges[edge_idx-1].type = SOURCE_EDGE;
    problem->edges[edge_idx-1].start_node = NULL;
}

void set_sink_edge(Problem *problem, int st_idx, int edge_idx) {
    problem->stations[st_idx].sink_edge = &problem->edges[edge_idx - 1];
    problem->edges[edge_idx-1].type = SINK_EDGE;
    problem->edges[edge_idx-1].end_node = NULL;
}

int add_source_sink_edges(config_t *cfg, Problem *problem) {
    if (set_array_field_from_config(cfg, problem, "source_edges", problem->num_stations, set_source_edge) == EXIT_FAILURE ||
        set_array_field_from_config(cfg, problem, "sink_edges", problem->num_stations, set_sink_edge) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


int parse(const char* file, Problem *problem) {
    config_t cfg;

    config_init(&cfg);

    if(! config_read_file(&cfg, file))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return(EXIT_FAILURE);
    }
    fprintf(stderr, "Config file loaded\n");

    if(load_constants(&cfg, problem) != EXIT_SUCCESS ||
       create_stations(&cfg, problem) != EXIT_SUCCESS ||
       create_trainsets(&cfg, problem) != EXIT_SUCCESS ||
       create_edges(&cfg, problem) != EXIT_SUCCESS ||
       create_nodes(&cfg, problem) != EXIT_SUCCESS ||
       add_source_sink_edges(&cfg, problem) != EXIT_SUCCESS ||
       assign_stations(problem) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    };

    config_destroy(&cfg);
    fprintf(stderr, "Config successfully parsed\n");

    return(EXIT_SUCCESS);
}