# gulan

一个简单的http服务器

special thanks to [zaver](https://github.com/zyearn/zaver)

## to do

- [ ] LRU cache
- [ ] memory pool
- [ ] improved threadpool
- [ ] rbtree timer

## hardwareeffect profile to do
- [ ] cahce miss pruning
- [ ] critical path improving

## to learn
- [ ] the assembly
- [ ] the perf/profile usage
- [ ] the ISA
- [ ] the kernel path
- [ ] the datasturcture, maybe lock-free,since now threadpool is bad

## compile and run (for now only support Linux2.6+)

please make sure you have [cmake](https://cmake.org/) installed.
### AUTO
you can just run the fllowing script to build and run and benchtest
```
./build.sh
./run.sh
```
new terminal
```
./webbench.sh
```

### MANUAL
```
mkdir build && cd build
cmake .. && make
cd .. 
./build/gulan -c gulan.conf
```
then open your browser input
```
127.0.0.1:3000
```
OR USE webbench

```
./build/tests/webbench -c 6000 -t 30 http://127.0.0.1:3000/
```
由于测试机器文件描述符的原因,暂时只能测试6000个客户端。

