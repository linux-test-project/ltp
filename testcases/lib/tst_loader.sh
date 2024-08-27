#!/bin/sh
#
# This is a loader for shell tests that use the C test library.
#

if [ -z "$LTP_IPC_PATH" ]; then
	tst_run_shell $(basename "$0") "$@"
	exit $?
else
	. tst_env.sh
fi
