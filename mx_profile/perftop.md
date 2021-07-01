# perf top
```
ps -ef|grep 'gulan'
```
get PID of gulan

and then
```
perf top -p 16791
```

we get 
```
  13.62%  [kernel]            [k] finish_task_switch
  10.26%  [kernel]            [k] __lock_text_start
   2.76%  [kernel]            [k] do_syscall_64
   2.29%  libc-2.27.so        [.] vfprintf
   2.02%  libc-2.27.so        [.] __GI___libc_write
   1.16%  [kernel]            [k] kmem_cache_free
   1.08%  [kernel]            [k] __softirqentry_text_start
   1.06%  libpthread-2.27.so  [.] __libc_write
   1.04%  [kernel]            [k] clear_page_orig
   0.94%  [kernel]            [k] kmem_cache_alloc
   0.90%  [kernel]            [k] __fget_light
   0.86%  [kernel]            [k] crc32c_pcl_intel_update
   0.85%  libc-2.27.so        [.] _IO_default_xsputn
   0.82%  libc-2.27.so        [.] _IO_file_write@@GLIBC_2.2.5
   0.74%  libc-2.27.so        [.] __strchrnul_avx2
   0.73%  [kernel]            [k] rcu_all_qs
   0.71%  [kernel]            [k] ext4_do_update_inode
   0.71%  libc-2.27.so        [.] buffered_vfprintf
   0.70%  gulan               [.] serve_static
   0.60%  gulan               [.] rio_writen
   0.58%  [kernel]            [k] common_file_perm
   0.57%  [kernel]            [k] memset_orig
   0.56%  [kernel]            [k] _cond_resched
   0.56%  [kernel]            [k] copy_user_generic_unrolled
   0.55%  gulan               [.] do_request
   0.54%  libpthread-2.27.so  [.] __open64
   0.50%  libpthread-2.27.so  [.] __close
   0.46%  libc-2.27.so        [.] __fprintf_chk
   0.46%  [kernel]            [k] __tcp_transmit_skb
```
