#! /bin/bash

if [ ! $SCRIPTS_DIR ]; then
	# assume we're running standalone
	export SCRIPTS_DIR=../../scripts/
fi

source $SCRIPTS_DIR/setenv.sh
LOG_FILE="$LOG_DIR/$LOG_FORMAT-pthread_cond_many.log"

echo "Logging to: " | tee -a $LOG_FILE
echo "$LOG_FILE " | tee -a $LOG_FILE
echo "and to local individual .out files " | tee -a $LOG_FILE

#
# make will eventually go away from here, as will the above echoes
#
make

#
# Test lots of threads.  Specify "--realtime" if you want the first
# process to run realtime.  The remainder of the processes (if any)
# will run non-realtime in any case.

nthread=5000
iter=400
nproc=5

echo "pthread_cond_many configuration:" | tee -a $LOG_FILE
echo "number of threads = $nthread " | tee -a $LOG_FILE
echo "number of iterations = $iter " | tee -a $LOG_FILE
echo "number of processes = $nproc " | tee -a $LOG_FILE

# Remove any existing local log files
rm -f $nthread.$iter.$nproc.*.out

i=0
./pthread_cond_many --realtime --broadcast -i $iter -n $nthread > $nthread.$iter.$nproc.$i.out &
i=1
while test $i -lt $nproc
do
        ./pthread_cond_many --broadcast -i $iter -n $nthread > $nthread.$iter.$nproc.$i.out &
        i=`expr $i + 1`
done
wait
