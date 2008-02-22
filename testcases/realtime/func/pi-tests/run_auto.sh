#! /bin/bash

if [ ! $SCRIPTS_DIR ]; then
        # assume we're running standalone
        export SCRIPTS_DIR=../../scripts/
fi

source $SCRIPTS_DIR/setenv.sh

$SCRIPTS_DIR/run_c_files.sh "testpi-0"
$SCRIPTS_DIR/run_c_files.sh "testpi-1"

LOG_FILE="$LOG_DIR/$LOG_FORMAT-testpi-1.log"
PYTHONPATH=../../  python parse-testpi1.py $LOG_FILE 2>&1 | tee -a $LOG_FILE

$SCRIPTS_DIR/run_c_files.sh "testpi-2"
LOG_FILE="$LOG_DIR/$LOG_FORMAT-testpi-2.log"
PYTHONPATH=../../  python parse-testpi2.py $LOG_FILE 2>&1 | tee -a $LOG_FILE

$SCRIPTS_DIR/run_c_files.sh "testpi-4"
LOG_FILE="$LOG_DIR/$LOG_FORMAT-testpi-4.log"
PYTHONPATH=../../  python parse-testpi1.py $LOG_FILE 2>&1 | tee -a $LOG_FILE 



$SCRIPTS_DIR/run_c_files.sh "testpi-5"
$SCRIPTS_DIR/run_c_files.sh "testpi-6"
$SCRIPTS_DIR/run_c_files.sh "sbrk_mutex"
