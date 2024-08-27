#!/bin/sh

testdir=$(realpath $(dirname $0))
export PATH="$PATH:$testdir:$testdir/tests/"

for i in `seq -w 01 06`; do
	echo
	echo "*** Running shell_test$i ***"
	echo
	./tests/shell_test$i
done
