#ifndef LAN_TIMER_H
#define LAN_TIMER_H

#include "priority_queue.h"
#include "http_request.h"

#define LAN_TIMER_INFINITE -1
#define TIMEOUT_DEFAULT 10000     /* ms */

typedef int (*timer_handler_pt)(lan_http_request_t *rq);

typedef struct lan_timer_node_s{
    size_t key;
    int deleted;    /* if remote client close the socket first, set deleted to 1 */
    timer_handler_pt handler;
    lan_http_request_t *rq;
} lan_timer_node;

int lan_timer_init();
int lan_find_timer();
void lan_handle_expire_timers();

extern lan_pq_t lan_timer;
extern size_t lan_current_msec;

void lan_add_timer(lan_http_request_t *rq, size_t timeout, timer_handler_pt handler);
void lan_del_timer(lan_http_request_t *rq);

#endif
