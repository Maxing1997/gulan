# debug
- [x] webbench 3000 segment fault

## noEMFILElog
can observe from log 
```
[INFO] (/home/m/Hack/network/gulan/src/gulan.c:173) new connection fd 1023
[ERROR] (/home/m/Hack/network/gulan/src/gulan.c:166: errno: Too many open files) accept
[ERROR] (/home/m/Hack/network/gulan/src/http.c:264: errno: Too many open files) open error
[ERROR] (/home/m/Hack/network/gulan/src/http.c:267: errno: Bad file descriptor) mmap error
[ERROR] (/home/m/Hack/network/gulan/src/rio.c:46: errno: Bad address) errno == 14
```

the output of `ulimit -n` is 1024, which is the max open file numbers

when accept new connection but can't open a new fd, the problem shows, but why in 3000. 2000 is right.

count the 1023 
```
m@ubuntu:~/Hack/network/gulan$ cat log|grep 1023 | sed ':a;N;$!ba;s/\n/ /g'|awk 'BEGIN{FS=" "}{for(i=1;i<=NF;i++){a[$i]++}}END{for(i in a){print i,a[i]}}' |sort -n -t " " -k 2 -r
new 242
[INFO] 242
fd 242
1023 242
(/home/m/Hack/network/gulan/src/gulan.c:198) 121
(/home/m/Hack/network/gulan/src/gulan.c:173) 121
from 121
data 121
connection 121
```

`242*1024`= 123904 times


## EMFILElog
add EMFILE error handle find the following pattern

webebench get 1121 succeed https request, and watch the log find 53+60+62+301+684=1160, is matching.

and count the EMFILE 
```
m@ubuntu:~/Hack/network/gulan$ grep -o 'EMFILE' 1.log |wc -l
566306
```

we can see: may be after many times the kernel decide to giv up this program. Maybe too many errnos and exceed the program memory space which caused to segement fualt

## idelfd

tried, starting from fd 6, but still can't exceed 1024, and finally dump
# TIMEOUT

## 10msTimeout 

dump faster

## 1000ms timeout
`121*1023`

## 4000ms timeout
198
## 10000ms timeout

can support 10s webbench or 30s 

320

# Timer question!!!
```
double free(!prev)
``` 
happens in log, and `ulimit -c unlimited` to print bt

we get 
```
#6  0x000055c616e2e144 in lan_http_close_conn (r=r@entry=0x55c6189fbe20) at /home/m/Hack/network/gulan/src/http_request.c:88
#7  0x000055c616e2cedd in do_request (ptr=0x55c6189fbe20) at /home/m/Hack/network/gulan/src/http.c:155
```
in fact, `lan_http_close_conn` is timer's callback. One of control flow is 
```c
        if (!out->keep_alive) {
            log_info("no keep_alive! ready to close");
            free(out);
            goto close;
        }
```
thus why we see `no keep_alive! ready to close`, this is normal case.

the other control flow is 
```c
                    lan_add_timer(request, TIMEOUT_DEFAULT, lan_http_close_conn);
```

now if the timeout is too short, some fd is not ok to handle request, they are blocking, and blocked so long to be kicked off as timer has expired. That exactly happens before the normal close.

open debug log
```
[INFO] (/home/m/Hack/network/gulan/src/gulan.c:217) new data from fd 1713527968
DEBUG /home/m/Hack/network/gulan/src/timer.c:114: in lan_del_timer
DEBUG /home/m/Hack/network/gulan/src/timer.c:23: in lan_time_update, time = 1622606412186
```
we know` do_request` calls `lan_del_timer`, and one reason is access 
```
#0  lan_del_timer (rq=rq@entry=0x557aaa0fb6b0) at /home/m/Hack/network/gulan/src/timer.c:119
119	    timer_node->deleted = 1;
(gdb) bt
#0  lan_del_timer (rq=rq@entry=0x557aaa0fb6b0) at /home/m/Hack/network/gulan/src/timer.c:119
#1  0x0000557aa9a0791c in do_request (ptr=0x557aaa0fb6b0) at /home/m/Hack/network/gulan/src/http.c:53
#2  0x0000557aa9a06bd1 in main (argc=<optimized out>, argv=<optimized out>) at /home/m/Hack/network/gulan/src/gulan.c:221
(gdb) p timer_node
$1 = (lan_timer_node *) 0x8200
```
 0x8200 belongs to kernel. The DEFINITIVE log
 ```
[INFO] (/home/m/Hack/network/gulan/src/gulan.c:192) new connection fd 519
DEBUG /home/m/Hack/network/gulan/src/timer.c:23: in lan_time_update, time = 1622607380881
DEBUG /home/m/Hack/network/gulan/src/timer.c:104: in lan_add_timer, key = 1622607381281

[INFO] (/home/m/Hack/network/gulan/src/gulan.c:217) new data from fd 519
DEBUG /home/m/Hack/network/gulan/src/timer.c:114: in lan_del_timer
DEBUG /home/m/Hack/network/gulan/src/timer.c:23: in lan_time_update, time = 1622607381281
```
timeout设为400，正要接受数据，结果到期了。去删除timer,but 已经是野指针，就导致了这个问题。

!Note
```c
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
```
真实原因
```
DEBUG /home/m/Hack/network/gulan/src/timer.c:57: in lan_find_timer, key = 1622607381281, cur = 1622607381280
DEBUG /home/m/Hack/network/gulan/src/gulan.c:152: wait time = 1
DEBUG /home/m/Hack/network/gulan/src/timer.c:66: in lan_handle_expire_timers
DEBUG /home/m/Hack/network/gulan/src/timer.c:71: lan_handle_expire_timers, size = 2488
DEBUG /home/m/Hack/network/gulan/src/timer.c:23: in lan_time_update, time = 1622607381281
DEBUG /home/m/Hack/network/gulan/src/timer.c:71: lan_handle_expire_timers, size = 2487
DEBUG /home/m/Hack/network/gulan/src/timer.c:23: in lan_time_update, time = 1622607381281
```
可以看出在这个时刻很多定时器到期了，所以开始清理，清理之后头号收到数据的就是519，也许刚好519不幸不清理，这样free(timer_node),timer_node就变成了野指针。就导致后面标记删除519定时器的时候发生错误。

事实上任意一个被清理的fd来数据，都会发生core dump。原因就是预先清理了连接但是长久不活动的timer，事实上是被挤占了，但是有没有close(fd)，导致出现问题。

thus when facing high contetntion, wait too long, fix the TIMEOUT sufficient.
