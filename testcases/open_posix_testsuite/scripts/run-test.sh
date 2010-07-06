#!/bin/sh
#
# A simple wrapper for pre- and post-execution activities for any given
# openposix test.
#
# run_test contains logic moved out of Makefile.
#
# Garrett Cooper, June 2010
#

LOGFILE=${LOGFILE:=logfile}

NUM_FAIL=0
NUM_PASS=0
NUM_TESTS=0

run_test_loop() {

	for test in $*; do

		if run_test $test; then
			: $(( NUM_PASS += 1 ))
		else
			: $(( NUM_FAIL += 1 ))
		fi
		: $(( NUM_TESTS += 1 ))

	done

	cat <<EOF
*****************
SUMMARY
*****************
PASS		$NUM_PASS
FAIL		$NUM_FAIL
*****************
TOTAL		$NUM_TESTS
*****************
EOF

}

run_test() {

	testname=`echo "$1" | sed -e 's,.run-test$,,'`

	complog=$testname.log.$$

	$SHELL -c "'$(dirname "$0")/t0' ./$1" > $complog 2>&1

	ret_code=$?

	if [ "$ret_code" = "0" ]; then
		echo "$testname: execution: PASS" >> $(LOGFILE)
	else
		case "$ret_code" in
		1)
			msg="FAILED"
			;;
		2)
			msg="UNRESOLVED"
			;;
		4)
			msg="UNSUPPORTED"
			;;
		5)
			msg="UNTESTED"
			;;
		$TIMEOUT_RET)
			msg="HUNG"
			;;
		*)
			msg="SIGNALED"
		esac
		(echo "$testname: execution: $msg: Output: "; cat $complog) >> \
		 ${LOGFILE}
		echo "$testname: execution: $msg "
	fi

	rm -f $complog

	return $ret_code

}

# SETUP
if echo > "$LOGFILE"; then
	:
else
	echo >&2 "ERROR: $LOGFILE not writable"
	exit 1
fi
if TIMEOUT_RET=$(cat "$(dirname "$0")/t0.val"); then
	TIMEOUT_VAL=${TIMEOUT_VAL:=240}
	if [ -f test_defs ] ; then
		. ./test_defs || exit $?
	fi
	trap '' INT

	# RUN
	run_test_loop $@
	exit $NUM_FAIL
else
	exit $? 
fi
