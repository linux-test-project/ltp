#!/bin/sh

exit_code=0
echo "unshare tests"
for i in `seq 1 5`; do
	echo "test $i (unshare)"
	./utstest_noltp unshare $i
	if [ $? -ne 0 ]; then
		exit_code=$?
	fi
done
echo "clone tests"
for i in `seq 1 5`; do
	echo "test $i (clone)"
	./utstest_noltp clone $i
	if [ $? -ne 0 ]; then
		exit_code=$?
	fi
done
exit $exit_code
