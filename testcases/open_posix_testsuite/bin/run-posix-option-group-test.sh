#! /bin/sh
# Copyright (c) Linux Test Project, 2010-2022
# Copyright (c) 2002, Intel Corporation. All rights reserved.
# Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.
#
# Use to build and run tests for a specific area

TESTPATH=""

BASEDIR="$(dirname "$0")/../${TESTPATH}/conformance/interfaces"

usage()
{
    cat <<EOF
usage: $(basename "$0") [AIO|MEM|MSG|SEM|SIG|THR|TMR|TPS]

Build and run the tests for POSIX area specified by the 3 letter tag
in the POSIX spec

EOF
}

run_option_group_tests()
{
	local list_of_tests

	list_of_tests=`find $1 -name '*.run-test' | sort`

	if [ -z "$list_of_tests" ]; then
		echo ".run-test files not found under $1, have been the tests compiled?"
		exit 1
	fi

	for test_script in $list_of_tests; do
		(cd "$(dirname "$test_script")" && ./$(basename "$test_script"))
	done
}

case $1 in
AIO)
	echo "Executing asynchronous I/O tests"
	run_option_group_tests "$BASEDIR/aio_*"
	run_option_group_tests "$BASEDIR/lio_listio"
	;;
SIG)
	echo "Executing signals tests"
	run_option_group_tests "$BASEDIR/sig*"
	run_option_group_tests $BASEDIR/raise
	run_option_group_tests $BASEDIR/kill
	run_option_group_tests $BASEDIR/killpg
	run_option_group_tests $BASEDIR/pthread_kill
	run_option_group_tests $BASEDIR/pthread_sigmask
	;;
SEM)
	echo "Executing semaphores tests"
	run_option_group_tests "$BASEDIR/sem*"
	;;
THR)
	echo "Executing threads tests"
	run_option_group_tests "$BASEDIR/pthread_*"
	;;
TMR)
	echo "Executing timers and clocks tests"
	run_option_group_tests "$BASEDIR/time*"
	run_option_group_tests "$BASEDIR/*time"
	run_option_group_tests "$BASEDIR/clock*"
	run_option_group_tests $BASEDIR/nanosleep
	;;
MSG)
	echo "Executing message queues tests"
	run_option_group_tests "$BASEDIR/mq_*"
	;;
TPS)
	echo "Executing process and thread scheduling tests"
	run_option_group_tests "$BASEDIR/*sched*"
	;;
MEM)
	echo "Executing mapped, process and shared memory tests"
	run_option_group_tests "$BASEDIR/m*lock*"
	run_option_group_tests "$BASEDIR/m*map"
	run_option_group_tests "$BASEDIR/shm_*"
	;;
*)
	usage
	exit 1
	;;
esac

echo "****Tests Complete****"
