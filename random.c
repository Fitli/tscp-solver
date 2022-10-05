//
// Created by fitli on 22.09.22.
//

#include <stdlib.h>
#include "random.h"

int random_1_over_i(int number) {
    int weight_sum = 0;
    for (int i = 2; i <= number; ++i) {
        weight_sum += number/i;
    }
    int random = rand() % weight_sum;
    for (int i = 2; i <= number; ++i) {
        if(random < number/i) {
            return i-2;
        }
        random -= number/i;
    }
}

int random_1_over_square(int number) {
    for (int i = 0; i < number; ++i) {
        if(rand() % 2) {
            return i;
        }
    }
    return 0;
}