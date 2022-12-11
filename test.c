//
// Created by Ivana Krumlová on 28.04.22.
//

#include <stdio.h>
#include "test.h"
#include "objective.h"

int test_graph_consistency(Problem *problem, Solution *solution){
    for (int i_node = 0; i_node < problem->num_inner_nodes; i_node++) {
        Node node = problem->nodes[i_node];
        for (int ts = 0; ts < problem->num_trainset_types; ts++) {
            int incoming = solution->edge_solution[node.in_waiting->id].num_trainsets[ts];
            if (node.in_subcon) {
                incoming += solution->edge_solution[node.in_subcon->id].num_trainsets[ts];
            }
            int outcoming = solution->edge_solution[node.out_waiting->id].num_trainsets[ts];
            if (node.out_subcon) {
                outcoming += solution->edge_solution[node.out_subcon->id].num_trainsets[ts];
            }

            if(incoming != outcoming) {
                fprintf(stderr, "CONSISTENCY TEST: unconsistent solution around node %d\n", i_node);
                //return 0;
            }
        }
    }
    return 1;
}

int test_start_end_station_consistency(Problem *problem, Solution *solution){
    for (int i_station = 0; i_station < problem->num_stations; i_station++) {
        Station station = problem->stations[i_station];
        for (int ts = 0; ts < problem->num_trainset_types; ts++) {
            int outcoming = solution->edge_solution[station.source_node->out_waiting->id].num_trainsets[ts];
            int incoming = solution->edge_solution[station.sink_node->in_waiting->id].num_trainsets[ts];

            if(incoming != outcoming) {
                fprintf(stderr, "START-END CONSISTENCY TEST: unconsistent solution in station %d\n", i_station);
                return 0;
            }
        }
    }
    return 1;
}

int test_consistency(Problem *problem, Solution *solution) {
    return test_graph_consistency(problem, solution) && test_start_end_station_consistency(problem, solution);
}

int test_objective(Problem *problem, Solution *solution) {
    int out = 1;
    Solution sol_copy;
    empty_solution(problem, &sol_copy);
    copy_solution(problem, solution, &sol_copy);
    recalculate_objective(&sol_copy, problem);
    if(solution->objective != sol_copy.objective) {
        fprintf(stderr, "OBJECTIVE TEST: itratively updated objective is not euqal to recalculated objective\n%lld\n%lld", solution->objective, sol_copy.objective);
        out = 0;
    }
    destroy_solution(problem, &sol_copy);
    return out;
}