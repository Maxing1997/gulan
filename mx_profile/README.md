# TOOL
- [x] perf
- [x] gprof

# performance

don't use cache 
```
m@ubuntu:~/Hack/network/gulan$ ./webbench.sh
Webbench - Simple Web Benchmark 1.5
Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.

Request:
GET / HTTP/1.0
User-Agent: WebBench 1.5
Host: 127.0.0.1


Runing info: 6000 clients, running 30 sec.

Speed=664684 pages/min, 6060481 bytes/sec.
Requests: 332342 susceed, 0 failed.
```
QPS: REQUESTS/SEONDS

 about 10000QPS

## the best have ever
```
Runing info: 5000 clients, running 20 sec.

Speed=2417880 pages/min, 22079468 bytes/sec.
Requests: 805960 susceed, 0 failed.
```
40000 QPS
