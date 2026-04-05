#ifndef TIME_UTILS_H

#include <sys/time.h>

double clock_timespec_real(struct timespec *t);
void clock_timespec_get(struct timespec *t);

#define TIME_UTILS_H
#endif /* TIME_UTILS_H */

