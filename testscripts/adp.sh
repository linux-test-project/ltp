#!/bin/bash
#
# adp.sh -- run ADP's stress test on /proc/[0-9]*/cmdline
#
#

echo "Starting test, please wait.... Ctrl-C to exit"

for i in 1 2 3 4 5
do
	./adp_children.sh &
done

sleep 2

for i in 1 2 3 4 5
do
	./adp_test.sh &
done

top
