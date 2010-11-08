#!/bin/sh

echo "testing keepcaps"
check_keepcaps 1
tmp=$?
if [ $tmp -ne 0 ]; then
	exit_code=$tmp
fi
check_keepcaps 2
tmp=$?
if [ $tmp -ne 0 ]; then
	exit_code=$tmp
fi
check_keepcaps 3
tmp=$?
if [ $tmp -ne 0 ]; then
	exit_code=$tmp
fi

exit $exit_code
