#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include "dbg.h"
#include "error.h"

#define LAN_PQ_DEFAULT_SIZE 10

typedef int (*lan_pq_comparator_pt)(void *pi, void *pj);

typedef struct {
    void **pq;
    size_t nalloc;
    size_t size;
    lan_pq_comparator_pt comp;
} lan_pq_t;

int lan_pq_init(lan_pq_t *lan_pq, lan_pq_comparator_pt comp, size_t size);
int lan_pq_is_empty(lan_pq_t *lan_pq);
size_t lan_pq_size(lan_pq_t *lan_pq);
void *lan_pq_min(lan_pq_t *lan_pq);
int lan_pq_delmin(lan_pq_t *lan_pq);
int lan_pq_insert(lan_pq_t *lan_pq, void *item);

int lan_pq_sink(lan_pq_t *lan_pq, size_t i);
#endif 
