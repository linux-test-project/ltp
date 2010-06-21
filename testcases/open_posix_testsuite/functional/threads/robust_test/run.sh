#!/bin/sh

TOTAL=0
FAIL=0
PASS=0
Run()
{
	echo "TEST: " $1
	: $(( TOTAL += 1 ))
	./$1
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
TESTS="robust1-sun robust2-sun robust1-mode2 robust2-mode2 robust3-mode2"

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
