#!/bin/bash
#    Filename: mixed_load.sh
#      Author: Darren Hart <dvhltc@us.ibm.com>
# Description: Run multiple copies of periodic threads and compare their
#              runtime average.
#
# Use "-j" to enable the jvm simulator.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# Copyright (C) IBM Corporation, 2007
#
# 2007-Aug-29:	Initial version by Darren Hart <dvhltc@us.ibm.com>


# Run multiple copies of periodic threads and compare their runtime average

MASTER_PRIO=90 # just above highest prio of jvmsim
ITERATIONS=1000

T1_PRIO=73
T1_PERIOD=20
T1_LOOPS=750

T2_PRIO=63
T2_PERIOD=40
T2_LOOPS=1500

T3_PRIO=53
T3_PERIOD=80
T3_LOOPS=3000

T4_PRIO=43
T4_PERIOD=160
T4_LOOPS=6000

CRITERIA=10

# Remove logs laying around
rm -f pcl*.log > /dev/null

# Ensure we can spawn all the threads without being preempted
chrt -p -f $MASTER_PRIO $$

# Run $1 copies of the thread group, only run one copy of the jvm simulater per group of threads
function run_thread_groups() {
    CON=$1 # Concurrency
    for ((i=0; i<$CON; i++)); do
        ./periodic_cpu_load_single -i $ITERATIONS -r $T1_PRIO -t $T1_PERIOD -c $T1_LOOPS -f pcl-${CON}x-t1_$i > pcl-${CON}x-t1-$i.log &
        ./periodic_cpu_load_single -i $ITERATIONS -r $T2_PRIO -t $T2_PERIOD -c $T2_LOOPS -f pcl-${CON}x-t2_$i > pcl-${CON}x-t2-$i.log &
        ./periodic_cpu_load_single -i $ITERATIONS -r $T3_PRIO -t $T3_PERIOD -c $T3_LOOPS -f pcl-${CON}x-t3-$i > pcl-${CON}x-t3-$i.log &
        ./periodic_cpu_load_single $JVM_ARG -i $ITERATIONS -r $T4_PRIO -t $T4_PERIOD -c $T4_LOOPS -f pcl-${CON}x-t4-$i > pcl-${CON}x-t4-$i.log &
    done
    wait
}

# grab the average from a log file
function parse_avg() {
    grep Avg $1 | sed "s/Avg: \(.*\) us/\1/"
}
# grab composite averages from a glob of log files
function parse_avg_avg() {
    SUM=0
    COUNT=$#
    for LOG in $@; do
        A=`parse_avg $LOG`
        SUM=`echo "scale=2; $SUM + $A" | bc`
    done
    echo "scale=2; $SUM / $COUNT" | bc
}
# determine the min of the floating point averages
function min_float() {
    MIN=$1
    shift
    for VAL in $@; do
        LT=`echo "$VAL < $MIN" | bc`
        if [ $LT -eq 1 ]; then
            MIN=$VAL
        fi
        shift
    done
    echo $MIN
}
# determine the max of the floating point averages
function max_float() {
    MAX=$1
    shift
    for VAL in $@; do
        GT=`echo "$VAL > $MIN" | bc`
        if [ $GT -eq 1 ]; then
            MAX=$VAL
        fi
        shift
    done
    echo $MAX
}
# calculate (max-min)/min of the args passed in
function percent_error() {
    MIN=`min_float $@`
    MAX=`max_float $@`
    echo "scale=2; 100*($MAX - $MIN)/$MIN" | bc
}

# return 1 if all args are less than CRITERIA
function pass_criteria() {
    for ERR in $@; do
        LT=`echo "$ERR < $CRITERIA" | bc`
        if [ $LT -eq 0 ]; then
            return 0
        fi
    done
    return 1
}


# Parse args
JVM_ARG=""
JVM_STATUS="Disabled"
if [ "$1" == "-j" ]; then
    JVM_ARG="-j"
    JVM_STATUS="Enabled"
fi

cat <<EOL

------------------------------------------------
Periodic CPU Load: Average Runtime Percent Error
------------------------------------------------

Running $ITERATIONS iterations per thread
JVM Simulator: $JVM_STATUS

`printf "%8s%6s%10s%10s" Thread Prio Period Loops`
`printf "%8s%6s%10s%10s" ------ ---- ------ -----`
`printf "%8s%6s%10s%10s" T1 $T1_PRIO $T1_PERIOD $T1_LOOPS`
`printf "%8s%6s%10s%10s" T2 $T2_PRIO $T2_PERIOD $T2_LOOPS`
`printf "%8s%6s%10s%10s" T3 $T3_PRIO $T3_PERIOD $T3_LOOPS`
`printf "%8s%6s%10s%10s" T4 $T4_PRIO $T4_PERIOD $T4_LOOPS`
EOL

# Determine the average run time for a single instance of each thread
run_thread_groups 1
T1_1X_AVG=`parse_avg_avg pcl-1x-t1-0.log`
T2_1X_AVG=`parse_avg_avg pcl-1x-t2-0.log`
T3_1X_AVG=`parse_avg_avg pcl-1x-t3-0.log`
T4_1X_AVG=`parse_avg_avg pcl-1x-t4-0.log`

cat <<EOL

Single Instance Averages
------------------------
  T1: $T1_1X_AVG us
  T2: $T2_1X_AVG us
  T3: $T3_1X_AVG us
  T4: $T4_1X_AVG us
EOL
# allow console to update
sleep 1

run_thread_groups 2
T1_2X_AVG=`parse_avg_avg pcl-2x-t1-[01].log`
T2_2X_AVG=`parse_avg_avg pcl-2x-t2-[01].log`
T3_2X_AVG=`parse_avg_avg pcl-2x-t3-[01].log`
T4_2X_AVG=`parse_avg_avg pcl-2x-t4-[01].log`

cat <<EOL

2x Concurrent Run Averages
--------------------------
  T1: $T1_2X_AVG us
  T2: $T2_2X_AVG us
  T3: $T3_2X_AVG us
  T3: $T4_2X_AVG us
EOL
# allow console to update
sleep 1

run_thread_groups 4
T1_4X_AVG=`parse_avg_avg pcl-4x-t1-[0-3].log`
T2_4X_AVG=`parse_avg_avg pcl-4x-t2-[0-3].log`
T3_4X_AVG=`parse_avg_avg pcl-4x-t3-[0-3].log`
T4_4X_AVG=`parse_avg_avg pcl-4x-t4-[0-3].log`

cat <<EOL

4x Concurrent Run Averages
--------------------------
  T1: $T1_4X_AVG us
  T2: $T2_4X_AVG us
  T3: $T3_4X_AVG us
  T3: $T4_4X_AVG us
EOL

# Calculate the percent error
# %err = 100*(max-min)/min
T1_ERR=`percent_error $T1_1X_AVG $T1_2X_AVG $T1_4X_AVG`
T2_ERR=`percent_error $T2_1X_AVG $T2_2X_AVG $T2_4X_AVG`
T3_ERR=`percent_error $T3_1X_AVG $T3_2X_AVG $T3_4X_AVG`
T4_ERR=`percent_error $T4_1X_AVG $T4_2X_AVG $T4_4X_AVG`

cat <<EOL

Thread Runtime Percent Differences
----------------------------------
  T1: $T1_ERR%
  T2: $T2_ERR%
  T3: $T3_ERR%
  T4: $T4_ERR%
EOL

pass_criteria $T1_ERR $T2_ERR $T3_ERR $T4_ERR
PASS=$?
if [ $PASS -eq 1 ]; then
    RESULT="PASSED"
    RET=0
else
    RESULT="FAILED"
    RET=1
fi

echo -e "\nTest complete, see logs pcl-\$CONCURRENTx-t\$THREAD-\$NUM.log for detailed results."
echo "Criteria: < 10% Difference in average runs"
echo "Result: $RESULT"

exit $RET
