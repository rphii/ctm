#ifndef CTM_EVENT_H

#include <pthread.h>
#include <stddef.h>

typedef struct Ctm_Event {
    pthread_mutex_t mtx;
    size_t thumb_loaded;
} Ctm_Event;

#define CTM_EVENT_H
#endif /* CTM_EVENT_H */

