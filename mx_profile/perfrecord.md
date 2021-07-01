-g选项是告诉perf record额外记录函数的调用关系，因为原本perf record记录大都是库函数，直接看库函数，大多数情况下，你的代码肯定没有标准库的性能好对吧？除非是针对产品进行特定优化，所以就需要知道是哪些函数频繁调用这些库函数，通过减少不必要的调用次数来提升性能

-e cpu-clock 指perf record监控的指标为cpu周期

程序运行完之后，perf record会生成一个名为perf.data的文件（缺省值），如果之前已有，那么之前的perf.data文件会变为perf.data.old文件

[.]代表该调用属于用户态，若自己监控的进程为用户态进程，那么这些即主要为用户态的cpu-clock占用的数值，[k]代表属于内核态的调用。

```
root@ubuntu:/home/m/Hack/network/gulan# perf record -e cpu-clock -g ./run.sh
^C[ perf record: Woken up 35 times to write data ]
[ perf record: Captured and wrote 8.742 MB perf.data (45860 samples) ]
Terminated
root@ubuntu:/home/m/Hack/network/gulan# perf report -i perf.data
```
the main time spend on write
```
   13.12%    13.09%  gulan    [kernel.kallsyms]   [k] finish_task_switch                                                                               ▒
   - 7.71% 0xd4b4f2030303220                                                                                                                            ▒
      - 7.71% __libc_write                                                                                                                              ▒
         - 7.69% entry_SYSCALL_64_after_hwframe                                                                                                         ▒
            - do_syscall_64                                                                                                                             ▒
               - 7.67% __x64_sys_write                                                                                                                  ▒
                    ksys_write                                                                                                                          ▒
                    vfs_write                                                                                                                           ▒
                    __vfs_write                                                                                                                         ▒
                    new_sync_write                                                                                                                      ▒
                    sock_write_iter                                                                                                                     ▒
                  + sock_sendmsg                                                                                                                        ▒
   - 2.03% 0x74656e2f6b636148                                                                                                                           ▒
      - 1.77% __close                                                                                                                                   ▒
         - 1.77% entry_SYSCALL_64_after_hwframe                                                                                                         ▒
              do_syscall_64                                                                                                                             ▒
            - exit_to_usermode_loop                                                                                                                     ▒
               - 1.75% task_work_run                                                                                                                    ▒
                  - 1.75% ____fput                                                                                                                      ▒
                     - __fput                                                                                                                           ▒
                        - 1.71% dput                                                                                                                    ▒
                           - 1.69% _cond_resched                                                                                                        ▒
                                __schedule                                                                                                              ▒
                                finish_task_switch                                                                                                      ▒
   - 1.76% 0xa3e6c6d74682045                                                                                                                            ▒
      + 1.75% __libc_write                                                                                                                              ▒
   + 0.66% epoll_wait

-   19.33%    19.33%  gulan    [kernel.kallsyms]   [k] __lock_text_start                                                                                ▒
   - 17.99% 0xd4b4f2030303220                                                                                                                           ▒
        __libc_write                                                                                                                                    ▒
        entry_SYSCALL_64_after_hwframe                                                                                                                  ▒
        do_syscall_64                                                                                                                                   ▒
        __x64_sys_write                                                                                                                                 ▒
        ksys_write                                                                                                                                      ▒
        vfs_write                                                                                                                                       ▒
        __vfs_write                                                                                                                                     ▒
        new_sync_write                                                                                                                                  ▒
        sock_write_iter                                                                                                                                 ▒
        sock_sendmsg                                                                                                                                    ▒
        inet_sendmsg                                                                                                                                    ▒
        tcp_sendmsg                                                                                                                                     ▒
        tcp_sendmsg_locked                                                                                                                              ▒
        tcp_push                                                                                                                                        ▒
        __tcp_push_pending_frames                                                                                                                       ▒
      + tcp_write_xmit                                                                                                                                  ▒
   + 0.70% 0x74656e2f6b636148                                                                                                                           ▒
```


