//
// Created by fitli on 31.10.22.
//

#include <stdlib.h>
#include "constructive_alg.h"
#include "datatypes.h"
#include "operations.h"

typedef enum phase Phase;

enum phase {
    REMOVE,
    ADD
};

void remove_waiting(Problem *problem, Solution *sol) {
    for (int s = 0; s < problem->num_stations; ++s) {
        for (int t = 0; t < problem->num_trainset_types; ++t) {
            while(oper_remove_waiting_train(sol, problem, s, t)) {}
        }
    }
}

/*void lower_edge(Solution *sol, Problem *problem, int edge_id) {
    EdgeSolution e_sol = sol->edge_solution[edge_id];
    int min_cap = problem->edges[edge_id].minimal_capacity;
    int num_ts = 0;
    for (int ts = 0; ts < problem->num_trainset_types; ++ts) {
        num_ts += e_sol.num_trainsets[ts];
    }

    // if the train is too long
    if(num_ts > problem->max_len) {
        // remove ts
        for (int ts = 0; ts < problem->num_trainset_types; ++ts) {
            if(e_sol.num_trainsets[ts] > 0 && e_sol.capacity - problem->trainset_types[ts].seats >= min_cap) {
                if(oper_remove_train_with_edge_dfs(sol, problem, edge_id, ts)) {
                    return;
                }
            }
        }
        for (int ts = 0; ts < problem->num_trainset_types; ++ts) {
            if(e_sol.num_trainsets[ts] > 0) {
                if(oper_remove_train_with_edge_dfs(sol, problem, edge_id, ts)) {
                    return;
                }
            }
        }
    }

    // change type
    for (int ts1 = 0; ts1 < problem->num_trainset_types; ++ts1) {
        if(e_sol.num_trainsets[ts1] > 0) {
            for (int ts2 = 0; ts2 < problem->num_trainset_types; ++ts2) {
                if(problem->trainset_types[ts1].seats > problem->trainset_types[ts2].seats &&
                e_sol.capacity - problem->trainset_types[ts1].seats + problem->trainset_types[ts2].seats >= min_cap) {
                    if(oper_change_train_capacity_dfs_with_edge(sol, problem, edge_id, ts1, ts2, 1, 1)) {
                        return;
                    }
                }
            }
        }
    }

    // remove
    for (int ts = 0; ts < problem->num_trainset_types; ++ts) {
        if(e_sol.num_trainsets[ts] > 0 && e_sol.capacity - problem->trainset_types[ts].seats >= min_cap) {
            if(oper_remove_train_with_edge_dfs(sol, problem, edge_id, ts)) {
                return;
            }
        }
    }
    for (int ts = 0; ts < problem->num_trainset_types; ++ts) {
        if(e_sol.num_trainsets[ts] > 0) {
            if(oper_remove_train_with_edge_dfs(sol, problem, edge_id, ts)) {
                return;
            }
        }
    }
}

void increase_edge(Solution *sol, Problem *problem, int edge_id) {
    EdgeSolution e_sol = sol->edge_solution[edge_id];
    int min_cap = problem->edges[edge_id].minimal_capacity;

    // change type
    for (int ts1 = 0; ts1 < problem->num_trainset_types; ++ts1) {
        if(e_sol.num_trainsets[ts1] > 0) {
            for (int ts2 = 0; ts2 < problem->num_trainset_types; ++ts2) {
                if (problem->trainset_types[ts1].seats < problem->trainset_types[ts2].seats &&
                    e_sol.capacity - problem->trainset_types[ts1].seats + problem->trainset_types[ts2].seats >= min_cap) {
                    oper_change_train_capacity_dfs_with_edge(sol, problem, edge_id, ts1, ts2, 1, 1);
                    return;
                }
            }
        }
    }

    // add
    for (int ts = 0; ts < problem->num_trainset_types; ++ts) {
        if(problem->trainset_types[ts].seats >= min_cap) {
            oper_add_train_with_edge_dfs(sol, problem, edge_id, ts);
            return;
        }
    }
    oper_add_train_with_edge_dfs(sol, problem, edge_id, problem->num_trainset_types - 1);
}

void constructive_alg(Solution *sol, Problem *problem, int num_iters) {
    int need_more[problem->num_edges];
    int num_need_more = 0;
    int need_less[problem->num_edges];
    int num_need_less = 0;
    int iter = 0;
    Phase phase = REMOVE;
    while (iter < num_iters){// || num_need_more > 0 || num_need_less > 0) {
        printf("%d %lld %d %d %d %d\n", iter, sol->objective, sol->num_trainstes[0], sol->num_trainstes[1], num_need_more, num_need_less);
        num_need_more = 0;
        num_need_less = 0;
        for (int i = 0; i < problem->num_edges; ++i) {
            if(problem->edges[i].type != SUBCONNECTION) {
                continue;
            }
            if(sol->edge_solution[i].capacity < problem->edges[i].minimal_capacity) {
                need_more[num_need_more] = i;
                num_need_more++;
                continue;
            }
            if(sol->edge_solution[i].capacity > problem->max_cap) {
                need_less[num_need_less] = i;
                num_need_less++;
                continue;
            }
            int num_ts = 0;
            for (int j = 0; j < problem->num_trainset_types; ++j) {
                num_ts += sol->edge_solution[i].num_trainsets[j];
            }
            if(num_ts == 0) {
                need_more[num_need_more] = i;
                num_need_more++;
                continue;
            }
            if(num_ts > problem->max_len) {
                need_less[num_need_less] = i;
                num_need_less++;
                continue;
            }
        }
        if(num_need_less == 0 && phase == REMOVE) {
            phase = ADD;
        }
        if(num_need_more == 0 && phase == ADD) {
            remove_waiting(problem, sol);
            phase = REMOVE;
        }

        if(phase == REMOVE) {
            lower_edge(sol, problem, need_less[rand() % num_need_less]);
        }

        if(phase == ADD) {
            increase_edge(sol, problem, need_more[rand() % num_need_more]);
        }
        iter++;
    }
}*/
