#!/bin/sh
#
# A simple wrapper for pre- and post-execution activities for any given
# openposix test.
#
# run_test contains logic moved out of Makefile.
#
# Garrett Cooper, June 2010

run_test_loop() {
	for i in 
}

run_test() {

	testname=`echo "$1" | sed -e 's,.run-test$,,'`

	complog=$testname.log.$$

	"$(dirname "$0")/t0" ./$1 > $complog 2>&1

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
if [ "x$1" = x ]; then
	LOGFILE=/dev/stdout
elif echo > "$1"; then
	echo >&2 "ERROR: $1 not readable"
else
	LOGFILE=$1
fi
TIMEOUT_RET=$(cat "$(dirname "$0")/t0.val")
TIMEOUT_VAL=${TIMEOUT_VAL:=240}
if [ -f test_defs ] ; then
	. ./test_defs || exit $?
fi

# RUN
run_test "$@"
