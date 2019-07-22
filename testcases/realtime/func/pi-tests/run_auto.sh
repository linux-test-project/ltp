#!/bin/sh

profile=${1:-default}

cd $(dirname $0) # Move to test directory
if [ ! $SCRIPTS_DIR ]; then
        # assume we're running standalone
        export SCRIPTS_DIR=../../scripts/
fi

. $SCRIPTS_DIR/setenv.sh

# Warning: tests args are now set in profiles
$SCRIPTS_DIR/run_c_files.sh $profile testpi-0

export LOG_FILE="$LOG_DIR/$LOG_FORMAT-testpi-1.log"
$SCRIPTS_DIR/run_c_files.sh $profile testpi-1
PYTHONPATH=../../  python3 parse-testpi1.py $LOG_FILE 2>&1 | tee -a $LOG_FILE

export LOG_FILE="$LOG_DIR/$LOG_FORMAT-testpi-2.log"
$SCRIPTS_DIR/run_c_files.sh $profile testpi-2
PYTHONPATH=../../  python3 parse-testpi2.py $LOG_FILE 2>&1 | tee -a $LOG_FILE

export LOG_FILE="$LOG_DIR/$LOG_FORMAT-testpi-4.log"
$SCRIPTS_DIR/run_c_files.sh $profile testpi-4
PYTHONPATH=../../  python3 parse-testpi1.py $LOG_FILE 2>&1 | tee -a $LOG_FILE


export LOG_FILE=""
$SCRIPTS_DIR/run_c_files.sh $profile testpi-5
$SCRIPTS_DIR/run_c_files.sh $profile testpi-6
$SCRIPTS_DIR/run_c_files.sh $profile sbrk_mutex
