#! /bin/sh

#
# This is the scripts/ directory for rt-test.
# It exports variables used during the tests.
#
#export TESTS_DIR=$(readlink -f $SCRIPTS_DIR/..)

TESTSUITE_NAME=testcases/realtime
if [ -z "$PARENT" ]; then
    PARENT=${PWD%/$TESTSUITE_NAME*}
fi

export TESTS_DIR=$PARENT/$TESTSUITE_NAME
# TEST_REL_DIR is used as a unique id for a test dir
export TEST_REL_DIR=${PWD#$TESTS_DIR/}
export SCRIPTS_DIR=$TESTS_DIR/scripts
export PROFILES_DIR=$TESTS_DIR/profiles
export LOG_DIR=$TESTS_DIR/logs
export ARGUMENTS_INPUT_ERROR=25
export LOG_FORMAT="`hostname -s`-`uname -m`-`uname -r`-`date +%F`"

