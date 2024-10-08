#! /bin/bash

profile=${1:-default}

if [ ! $SCRIPTS_DIR ]; then
        # assume we're running standalone
        export SCRIPTS_DIR=../../scripts/
fi

source $SCRIPTS_DIR/setenv.sh


# This is a temporary workaround for previous
# loop support patch which seems to be having
# issues right now.

LOG_FILE="$LOG_DIR/$LOG_FORMAT-rdtsc-latency.log"
$SCRIPTS_DIR/run_c_files.sh $profile "rdtsc-latency"

LOG_FILE="$LOG_DIR/$LOG_FORMAT-preempt_timing.log"
$SCRIPTS_DIR/run_c_files.sh $profile "preempt_timing"
