#!/bin/sh
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

TOTAL=0
FAIL=0
PASS=0

Run()
{
	echo "TEST: " $1
	: $(( TOTAL += 1 ))
	./$1 > output.$1
	if [ $? -eq 0 ]; then
		: $(( PASS += 1 ))
		echo "		***TEST PASSED***"
		echo ""
	else
		: $(( FAIL += 1 ))
		echo "		***TEST FAILED***"
		echo ""
	fi
}

TESTS="pitest-1 pitest-2 pitest-3 pitest-4 pitest-5 pitest-6"

for test in $TESTS; do
	Run $test
done

cat <<EOF
		*****************
		*   TOTAL:  $TOTAL *
		*   PASSED: $PASS *
		*   FAILED: $FAIL *
		*****************
EOF
