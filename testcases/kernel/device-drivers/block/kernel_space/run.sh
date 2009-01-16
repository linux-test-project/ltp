#!/bin/sh

# Tell the system to flush write buffers in order to minimize data loss in
# case of a crash.
sync

echo Running Test Cases of "block" testsuite one by one.
echo The test results are printed to "dmesg".
echo

#
# Valid test cases (should run stable)
#
echo Test Case 1
insmod ./test_block.ko tc=1
rmmod test_block

echo Test Case 2
insmod ./test_block.ko tc=2
rmmod test_block

echo Test Case 5
insmod ./test_block.ko tc=5
rmmod test_block

#
# Invalid testcases (more probable to crash the module under test)
#
echo Test Case 3
insmod ./test_block.ko tc=3
rmmod test_block

echo Test Case 4
insmod ./test_block.ko tc=4
rmmod test_block

echo Test Case 6
insmod ./test_block.ko tc=6
rmmod test_block

echo Test Case 7
insmod ./test_block.ko tc=7
rmmod test_block

echo Test Case 10
insmod ./test_block.ko tc=10
rmmod test_block
