 the main bottleneck is `__lock_text_start` and `finish_task_switch`, I guess the reason is too many tcp write operations and caused to intensive contention, which asks lock for tcp buffer and schduel to switch context.

# the perf
the rop user function is `do_request`, so I decided to optimize it
```
-    9.19%     0.29%  gulan    gulan               [.] do_request                                                                                       ▒
   - 8.91% do_request                                                                                                                                   ▒
      - 4.59% __open64                                                                                                                                  ▒
         - 4.16% entry_SYSCALL_64_after_hwframe                                                                                                         ▒
            - 4.12% do_syscall_64                                                                                                                       ▒
               - 3.84% __x64_sys_openat                                                                                                                 ▒
                  - 3.78% do_sys_open                                                                                                                   ▒
                     - 2.73% do_filp_open                                                                                                               ▒
                        - 2.56% path_openat                                                                                                             ▒
                           - 0.89% vfs_open                                                                                                             ▒
                                0.72% do_dentry_open                                                                                                    ▒
                           - 0.68% alloc_empty_file                                                                                                     ▒
                                0.59% __alloc_file                                                                                                      ▒
                     - 0.58% getname                                                                                                                    ▒
                          0.55% getname_flags                                                                                                           ▒
      - 2.80% __munmap                                                                                                                                  ▒
         - 2.50% entry_SYSCALL_64_after_hwframe                                                                                                         ▒
            - 2.49% do_syscall_64                                                                                                                       ▒
               - 1.49% __x64_sys_munmap                                                                                                                 ▒
                  - 1.43% __vm_munmap                                                                                                                   ▒
                     - 1.36% __do_munmap                                                                                                                ▒
                          0.90% unmap_region                                                                                                            ▒
               - 0.84% exit_to_usermode_loop                                                                                                            ▒
                  - 0.75% task_work_run                                                                                                                 ▒
                     - 0.68% ____fput                                                                                                                   
                          0.58% __fput                                                                                                                  ▒
        0.57% __close                                                                                                                                   ▒
-    0.55%  gulan    libpthread-2.27.so  [.] __close                                                                                                    ▒
   - 8.38% __close                                                                                                                                      ▒
      - 8.32% entry_SYSCALL_64_after_hwframe                                                                                                            ▒
         - 8.22% do_syscall_64                                                                                                                          ▒
            - 7.51% exit_to_usermode_loop                                                                                                               ▒
               - 7.42% task_work_run                                                                                                                    ▒
                  - 7.37% ____fput                                                                                                                      ▒
                     - 7.32% __fput                                                                                                                     ▒
                        - 4.94% sock_close                                                                                                              ▒
                           - 4.88% __sock_release                                                                                                       ▒
                              - 4.65% inet_release                                                                                                      ▒
                                 + 4.51% tcp_close                                                                                                      ▒
                        + 1.24% dput                                                                                                                    ▒
                        + 0.75% eventpoll_release_file                                                                                                  ▒
```

the code of open/munmap
```
    int srcfd = open(filename, O_RDONLY, 0);
    check(srcfd > 2, "open error");
    // can use sendfile
    char *srcaddr = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    check(srcaddr != (void *) -1, "mmap error");
    close(srcfd);

    n = rio_writen(fd, srcaddr, filesize);
    // check(n == filesize, "rio_writen error");

    munmap(srcaddr, filesize);
```

## use sendfile
```
Webbench - Simple Web Benchmark 1.5
Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.

Request:
GET / HTTP/1.0
User-Agent: WebBench 1.5
Host: 127.0.0.1


Runing info: 5000 clients, running 20 sec.

Speed=180591 pages/min, 379152 bytes/sec.
Requests: 60197 susceed, 0 failed.
``` 
so bad, down to 3000QPS

## use index cache
```
Runing info: 5000 clients, running 10 sec.

Speed=1428480 pages/min, 13043442 bytes/sec.
Requests: 238080 susceed, 0 failed.
```

20000QPS, double up 

## remove timer
```
Runing info: 5000 clients, running 20 sec.

Speed=717645 pages/min, 1506796 bytes/sec.
Requests: 239215 susceed, 0 failed.
```
seems no different

## add threadpool 
```
Runing info: 5000 clients, running 10 sec.

Speed=21750 pages/min, 198650 bytes/sec.
Requests: 3625 susceed, 0 failed.
```
300qps, THE AWEFUL IDEA
