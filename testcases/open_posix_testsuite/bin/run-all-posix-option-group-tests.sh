#!/bin/sh
#
# A simple wrapper for executing all of the tests.
#
# See COPYING for licensing details.
#
# Ngie Cooper, July 2010
#

FAILED=0
PROG_SCRIPT="$(dirname "$0")/run-posix-option-group-test.sh"

for option_group in AIO MEM MSG SEM SIG THR TMR TPS; do
	if ! $PROG_SCRIPT $option_group; then
		FAILED=1
	fi
done

exit $FAILED
