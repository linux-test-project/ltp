#!/bin/bash

CURRENT_DIR=$(pwd)
TEST_DIRS="robust_test pi_test"

for test_dir in $TEST_DIRS; do

echo ""
echo "Run $test_dir tests"
echo "=============================="
echo ""
        cd $CURRENT_DIR/$test_dir;
	./run.sh
done

