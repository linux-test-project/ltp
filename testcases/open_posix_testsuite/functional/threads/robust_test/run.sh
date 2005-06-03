#!/bin/bash

declare -i TOTAL=0
declare -i FAIL=0
declare -i PASS=0
Run()
{
        echo "TEST: " $1
        TOTAL=$TOTAL+1
        ./$1
        if [ $? == 0 ]; then
                PASS=$PASS+1
                echo -ne "\t\t\t***TEST PASSED***\n\n"
        else
                FAIL=$FAIL+1
                echo -ne "\t\t\t***TEST FAILED***\n\n"
        fi
}
TESTS="robust1-sun robust2-sun robust1-mode2 robust2-mode2 robust3-mode2"

for test in $TESTS; do
	Run $test
done

echo -ne "\t\t*****************\n"
echo -ne "\t\t*   TOTAL:   "  $TOTAL *"\n"
echo -ne "\t\t*   PASSED:  "  $PASS *"\n"
echo -ne "\t\t*   FAILED:  "  $FAIL *"\n"
echo -ne "\t\t*****************\n"


