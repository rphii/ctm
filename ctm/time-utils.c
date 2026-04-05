#include "time-utils.h"
#include <time.h>

double clock_timespec_real(struct timespec *t) {
    double result = t->tv_sec + t->tv_nsec / 1e9;
    return result;
}

void clock_timespec_get(struct timespec *t) {
    clock_gettime(CLOCK_MONOTONIC, t);
}


