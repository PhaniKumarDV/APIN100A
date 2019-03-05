1. To collect RPM logs please do the following
		cat /sys/kernel/debug/rpm_log > debug.log

2. To stop log collection hit ctl+C 

3. To decode rpm logs please use the following command 
		python rpm_log.py -t 8064 -f debug.log > rpm_logs.txt
