#!/bin/bash

# Copyright (C) 2004 Dan Carpenter
# This software is released under the terms of the GPL

test_percent=20
[[ $1 == "" ]] || test_percent=$1

while true ; do
	RAND=$((RANDOM%$(cat test_list.txt | wc -l)))
	test=`cat test_list.txt | head -n $(($RAND + 1)) | tail -n 1`
	[ -f $test ] && ./strace -P $test_percent $test
	#[ -f $test ] && $test
done
