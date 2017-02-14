#!/bin/sh
#
# A simple wrapper for pre- and post-execution activities for any given
# openposix test.
#
# run_test contains logic moved out of Makefile.
#
# Ngie Cooper, June 2010
#

LOGFILE=${LOGFILE:=logfile}

NUM_FAIL=0
NUM_PASS=0
NUM_TESTS=0

run_test_loop() {

	for t in $*; do

		if run_test "$t"; then
			NUM_PASS=$(expr $NUM_PASS + 1)
		else
			NUM_FAIL=$(expr $NUM_FAIL + 1)
		fi
		NUM_TESTS=$(expr $NUM_TESTS + 1)

	done

	cat <<EOF
*******************
Testing $(basename $PWD)
*******************
$(printf "PASS\t\t%3d" $NUM_PASS)
$(printf "FAIL\t\t%3d" $NUM_FAIL)
*******************
$(printf "TOTAL\t\t%3d" $NUM_TESTS)
*******************
EOF

}

run_test() {

	testname="$TEST_PATH/${1%.*}"

	complog=$(basename $testname).log.$$

	sh -c "$SCRIPT_DIR/t0 $TIMEOUT_VAL ./$1 $(cat ./$(echo "$1" | sed 's,\.[^\.]*,,').args 2>/dev/null)" > $complog 2>&1

	ret_code=$?

	if [ "$ret_code" = "0" ]; then
		echo "$testname: execution: PASS" >> "${LOGFILE}"
	elif [ -f "$1" ]; then
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
			if [ $ret_code -gt 128 ]; then
				msg="SIGNALED"
			else
				msg="EXITED ABNORMALLY"
			fi
		esac
		echo "$testname: execution: $msg: Output: " >> "${LOGFILE}"
		cat $complog >> "${LOGFILE}"
		echo "$testname: execution: $msg "
	else
		echo "$testname: execution: SKIPPED (test not present)"
	fi

	rm -f $complog

	return $ret_code

}

# SETUP
if [ -w "$LOGFILE" ] || echo "" > "$LOGFILE"; then
	:
else
	echo >&2 "ERROR: $LOGFILE not writable"
	exit 1
fi

SCRIPT_DIR=$(dirname "$0")
TEST_PATH=$1; shift
T0=$SCRIPT_DIR/t0
T0_VAL=$SCRIPT_DIR/t0.val

if [ ! -x $T0 ]; then
	echo >&2 "ERROR: $T0 doesn't exist / isn't executable"
	exit 1
fi

if [ ! -f "$T0_VAL" ]; then
	$SCRIPT_DIR/t0 0 >/dev/null 2>&1
	echo $? > "$T0_VAL"
fi
if TIMEOUT_RET=$(cat "$T0_VAL"); then

	TIMEOUT_VAL=${TIMEOUT_VAL:=300}
	if [ -f test_defs ] ; then
		. ./test_defs || exit $?
	fi
	trap '' INT

	# RUN
	run_test_loop $*
	exit $NUM_FAIL

else
	exit $?
fi
