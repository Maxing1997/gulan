#ifndef THREADPOOL_H
#define THREADPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include "dbg.h"

#define THREAD_NUM 8

typedef struct lan_task_s {
    void (*func)(void *);
    void *arg;
    struct lan_task_s *next;
} lan_task_t;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    pthread_t *threads;
    lan_task_t *head;
    int thread_count;
    int queue_size;
    int shutdown;
    int started;
} lan_threadpool_t;

typedef enum {
    lan_tp_invalid   = -1,
    lan_tp_lock_fail = -2,
    lan_tp_already_shutdown  = -3,
    lan_tp_cond_broadcast    = -4,
    lan_tp_thread_fail       = -5,
    
} lan_threadpool_error_t;

lan_threadpool_t *threadpool_init(int thread_num);

int threadpool_add(lan_threadpool_t *pool, void (*func)(void *), void *arg);

int threadpool_destroy(lan_threadpool_t *pool, int gracegul);

#ifdef __cplusplus
}
#endif

#endif
