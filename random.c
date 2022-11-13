//
// Created by fitli on 22.09.22.
//

#include <stdlib.h>
#include "random.h"

/**
 * Return index of random member weighted by `weights`
 * @param weights
 * @param num_elems
 * @return
 */
int roulette_wheel(const int *weights, int num_elems) {
    int sum = 0;
    for (int i = 0; i < num_elems; ++i) {
        sum += weights[i];
    }
    if(sum == 0) {
        return 0;
    }
    int r = rand() % sum;
    for (int i = 0; i < num_elems; ++i) {
        r -= weights[i];
        if(r < 0) {
            return i;
        }
    }
    return 0;
}