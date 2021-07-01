#include "priority_queue.h"

int lan_pq_init(lan_pq_t *lan_pq, lan_pq_comparator_pt comp, size_t size) {
    lan_pq->pq = (void **)malloc(sizeof(void *) * (size+1));
    if (!lan_pq->pq) {
        log_err("lan_pq_init: malloc failed");
        return -1;
    }
    
    lan_pq->nalloc = 0;
    lan_pq->size = size + 1;
    lan_pq->comp = comp;
    
    return LAN_OK;
}

int lan_pq_is_empty(lan_pq_t *lan_pq) {
    return (lan_pq->nalloc == 0)? 1: 0;
}

size_t lan_pq_size(lan_pq_t *lan_pq) {
    return lan_pq->nalloc;
}

void *lan_pq_min(lan_pq_t *lan_pq) {
    if (lan_pq_is_empty(lan_pq)) {
        return NULL;
    }

    return lan_pq->pq[1];
}

static int resize(lan_pq_t *lan_pq, size_t new_size) {
    if (new_size <= lan_pq->nalloc) {
        log_err("resize: new_size to small");
        return -1;
    }

    void **new_ptr = (void **)malloc(sizeof(void *) * new_size);
    if (!new_ptr) {
        log_err("resize: malloc failed");
        return -1;
    }

    memcpy(new_ptr, lan_pq->pq, sizeof(void *) * (lan_pq->nalloc + 1));
    free(lan_pq->pq);
    lan_pq->pq = new_ptr;
    lan_pq->size = new_size;
    return LAN_OK;
}

static void exch(lan_pq_t *lan_pq, size_t i, size_t j) {
    void *tmp = lan_pq->pq[i];
    lan_pq->pq[i] = lan_pq->pq[j];
    lan_pq->pq[j] = tmp;
}

static void swim(lan_pq_t *lan_pq, size_t k) {
    while (k > 1 && lan_pq->comp(lan_pq->pq[k], lan_pq->pq[k/2])) {
        exch(lan_pq, k, k/2);
        k /= 2;
    }
}

static size_t sink(lan_pq_t *lan_pq, size_t k) {
    size_t j;
    size_t nalloc = lan_pq->nalloc;

    while (2*k <= nalloc) {
        j = 2*k;
        if (j < nalloc && lan_pq->comp(lan_pq->pq[j+1], lan_pq->pq[j])) j++;
        if (!lan_pq->comp(lan_pq->pq[j], lan_pq->pq[k])) break;
        exch(lan_pq, j, k);
        k = j;
    }
    
    return k;
}

int lan_pq_delmin(lan_pq_t *lan_pq) {
    if (lan_pq_is_empty(lan_pq)) {
        return LAN_OK;
    }

    exch(lan_pq, 1, lan_pq->nalloc);
    lan_pq->nalloc--;
    sink(lan_pq, 1);
    if (lan_pq->nalloc > 0 && lan_pq->nalloc <= (lan_pq->size - 1)/4) {
        if (resize(lan_pq, lan_pq->size / 2) < 0) {
            return -1;
        }
    }

    return LAN_OK;
}

int lan_pq_insert(lan_pq_t *lan_pq, void *item) {
    if (lan_pq->nalloc + 1 == lan_pq->size) {
        if (resize(lan_pq, lan_pq->size * 2) < 0) {
            return -1;
        }
    }

    lan_pq->pq[++lan_pq->nalloc] = item;
    swim(lan_pq, lan_pq->nalloc);

    return LAN_OK;
}

int lan_pq_sink(lan_pq_t *lan_pq, size_t i) {
    return sink(lan_pq, i);
}
