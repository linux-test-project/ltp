#!/bin/sh

echo "unshare tests"
for i in `seq 1 5`; do
	echo "test $i (unshare)"
	utstest unshare $i
done
echo "clone tests"
for i in `seq 1 5`; do
	echo "test $i (clone)"
	utstest clone $i
done
