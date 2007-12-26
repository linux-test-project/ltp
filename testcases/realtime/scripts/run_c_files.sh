#! /bin/bash

#
# This script is called from the directory where the test cases are.
# Not all the test cases use this.
#
# Usage: $0 test-name
#
# For the moment, this does not take multiple arguments.
# Call this script separately for each test.
#

echo -e "SCRIPTS_DIR = $SCRIPTS_DIR"
source $SCRIPTS_DIR/setenv.sh

# Compile the test cases to support stand alone runs.
make

IFS=:

# Run the test case
for file in $*
do
	cmd=`echo $file | cut -d ' ' -f 1`
	if [ `echo $file | wc -w` -gt 1 ]; then
		param=`echo $file | cut -d ' ' -f 2-`
	fi
	LOG_FILE="$LOG_DIR/$LOG_FORMAT-${cmd}${param// /}.log"
	echo -e "--- Running testcase $cmd $param --- \n" | tee -a $LOG_FILE
	date | tee -a $LOG_FILE
	echo "Logging to $LOG_FILE" | tee -a $LOG_FILE
        eval ./$cmd $param | tee -a $LOG_FILE
	echo "" | tee -a $LOG_FILE
	date | tee -a $LOG_FILE
	echo -e "The $cmd test appears to have completed. \n" | tee -a $LOG_FILE
	cmd=""
	param=""
done
