#!/bin/bash
PRIORITY=20
# If users run pi tests from sshd, they need to improve the priority
# sshd using the following command. 

# chrt -p -f $PRIORITY $PPID

# If users run pi test from console, they need to add the prioirity 
# of the shell using the following command.

# chrt -p -f $PRIORITY $$

killall -9 watchdogtimer.sh
rm -rf output.*
chrt -f $PRIORITY ../tools/watchdogtimer.sh &

declare -i TOTAL=0
declare -i FAIL=0
declare -i PASS=0
Run()
{
        echo "TEST: " $1
        TOTAL=$TOTAL+1
        ./$1 > output.$1
        if [ $? == 0 ]; then
                PASS=$PASS+1
                echo -ne "\t\t\t***TEST PASSED***\n\n"
        else
                FAIL=$FAIL+1
                echo -ne "\t\t\t***TEST FAILED***\n\n"
        fi
}

TESTS="pitest-1 pitest-2 pitest-3 pitest-4 pitest-5 pitest-6"

for test in $TESTS; do
	Run $test
done

echo -ne "\t\t*****************\n"
echo -ne "\t\t*   TOTAL:   "  $TOTAL *"\n"
echo -ne "\t\t*   PASSED:  "  $PASS *"\n"
echo -ne "\t\t*   FAILED:  "  $FAIL *"\n"
echo -ne "\t\t*****************\n"


