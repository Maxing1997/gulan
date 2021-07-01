# perf  stat
```
root@ubuntu:/home/m/Hack/network/gulan# perf stat ./run.sh
^C./run.sh: Interrupt

 Performance counter stats for './run.sh':

         19,602.17 msec task-clock                #    0.347 CPUs utilized
            68,537      context-switches          #    0.003 M/sec
               122      cpu-migrations            #    0.006 K/sec
           196,625      page-faults               #    0.010 M/sec
   <not supported>      cycles
   <not supported>      instructions
   <not supported>      branches
   <not supported>      branch-misses

      56.471707960 seconds time elapsed

       4.064676000 seconds user
      15.407108000 seconds sys
```
task-clock是指程序运行期间占用了xx的任务时钟周期，该值高，说明程序的多数时间花费在 CPU 计算上而非 IO

context-switches是指程序运行期间发生了xx次上下文切换，记录了程序运行过程中发生了多少次进程切换，频繁的进程切换是应该避免的。（有进程进程间频繁切换，或者内核态与用户态频繁切换）

757 cpu-migrations 是指程序运行期间发生了xx次CPU迁移，即用户程序原本在一个CPU上运行，后来迁移到另一个CPU

16,368 page-faults 是指程序发生了xx次页错误
