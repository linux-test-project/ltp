#!/bin/sh

if [ -z "$1" ]; then
    echo "Usage: move_pages.sh <test-number>"
    exit 1
fi

testprog=move_pages${1}

if [ -f $LTPROOT/testcases/bin/${testprog} ]; then
    exec $testprog
else
    export TCID=$testprog
    export TST_TOTAL=1
    export TST_COUNT=0
    tst_resm TCONF "libnuma and NUMA support is required for this testcase"
    tst_exit
fi
