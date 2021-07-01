# find the PID and emit signal for profile
kill -USR1 `ps -ef|grep './build/gulan'| awk 'NR==1'|awk '{print $2}'`
gprof  build/gulan gmon.out > analysis.log

