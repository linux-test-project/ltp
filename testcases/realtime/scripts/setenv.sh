#! /bin/bash

#
# This is the scripts/ directory for rt-test.
# It exports variables used during the tests.
#

TESTSUITE_NAME=testcases/realtime
if [ -z "$PARENT" ]; then
    PARENT=${PWD%/$TESTSUITE_NAME*}
fi
export TESTS_DIR=$PARENT/$TESTSUITE_NAME

export SCRIPTS_DIR=$TESTS_DIR/scripts
export LOG_DIR=$TESTS_DIR/logs
export ARGUMENTS_INPUT_ERROR=25
export LOG_FORMAT="`hostname --short`-`uname -m`-`uname -r`-`date +%F`"
