#!/bin/bash
PRIORITY=20
# PINUM stands for the test cases number of pi test
PINUM=6
num=0
chrt -p -f $PRIORITY $$
TIMEOUT=600
echo "Start watchdogtimer script..."
echo "wait 10 minutes, if pitest hangs, stop the case execution"
while [ $num -lt $PINUM ]
do
sleep $TIMEOUT
killall -9 -q pitest*
echo "Timeout, kill pi test case"
num=`expr $num + 1`
done

