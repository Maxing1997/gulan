#include <sys/time.h>
#include "timer.h"

static int timer_comp(void *ti, void *tj) {
    lan_timer_node *timeri = (lan_timer_node *)ti;
    lan_timer_node *timerj = (lan_timer_node *)tj;

    return (timeri->key < timerj->key)? 1: 0;
}

lan_pq_t lan_timer;
size_t lan_current_msec;

static void lan_time_update() {
    // there is only one thread calling lan_time_update, no need to lock?
    struct timeval tv;
    int rc;

    rc = gettimeofday(&tv, NULL);
    check(rc == 0, "lan_time_update: gettimeofday error");

    lan_current_msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    debug("in lan_time_update, time = %zu", lan_current_msec);
}


int lan_timer_init() {
    int rc;
    rc = lan_pq_init(&lan_timer, timer_comp, LAN_PQ_DEFAULT_SIZE);
    check(rc == LAN_OK, "lan_pq_init error");
   
    lan_time_update();
    return LAN_OK;
}

int lan_find_timer() {
    lan_timer_node *timer_node;
    int time = LAN_TIMER_INFINITE;
    int rc;

    while (!lan_pq_is_empty(&lan_timer)) {
        debug("lan_find_timer");
        lan_time_update();
        timer_node = (lan_timer_node *)lan_pq_min(&lan_timer);
        check(timer_node != NULL, "lan_pq_min error");

        if (timer_node->deleted) {
            rc = lan_pq_delmin(&lan_timer); 
            check(rc == 0, "lan_pq_delmin");
            free(timer_node);
            continue;
        }
             
        time = (int) (timer_node->key - lan_current_msec);
        debug("in lan_find_timer, key = %zu, cur = %zu",
                timer_node->key,
                lan_current_msec);
        time = (time > 0? time: 0);
        break;
    }
    
    return time;
}

void lan_handle_expire_timers() {
    debug("in lan_handle_expire_timers");
    lan_timer_node *timer_node;
    int rc;

    while (!lan_pq_is_empty(&lan_timer)) {
        debug("lan_handle_expire_timers, size = %zu", lan_pq_size(&lan_timer));
        lan_time_update();
        timer_node = (lan_timer_node *)lan_pq_min(&lan_timer);
        check(timer_node != NULL, "lan_pq_min error");

        if (timer_node->deleted) {
            rc = lan_pq_delmin(&lan_timer); 
            check(rc == 0, "lan_handle_expire_timers: lan_pq_delmin error");
            free(timer_node);
            continue;
        }
        
        if (timer_node->key > lan_current_msec) {
            return;
        }

        if (timer_node->handler) {
            timer_node->handler(timer_node->rq);
        }
        rc = lan_pq_delmin(&lan_timer); 
        check(rc == 0, "lan_handle_expire_timers: lan_pq_delmin error");
        free(timer_node);
    }
}

void lan_add_timer(lan_http_request_t *rq, size_t timeout, timer_handler_pt handler) {
    int rc;
    lan_timer_node *timer_node = (lan_timer_node *)malloc(sizeof(lan_timer_node));
    check(timer_node != NULL, "lan_add_timer: malloc error");

    lan_time_update();
    rq->timer = timer_node;
    timer_node->key = lan_current_msec + timeout;
    debug("in lan_add_timer, key = %zu", timer_node->key);
    timer_node->deleted = 0;
    timer_node->handler = handler;
    timer_node->rq = rq;

    rc = lan_pq_insert(&lan_timer, timer_node);
    check(rc == 0, "lan_add_timer: lan_pq_insert error");
}

void lan_del_timer(lan_http_request_t *rq) {
    debug("in lan_del_timer");
    lan_time_update();
    lan_timer_node *timer_node = rq->timer;
    check(timer_node != NULL, "lan_del_timer: rq->timer is NULL");

    timer_node->deleted = 1;
}
