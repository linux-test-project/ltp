#!/bin/bash

# Copyright (C) 2004 Dan Carpenter
# This software is released under the terms of the GPL

max_tests=25
test_percent=20

[[ $1 == "" ]] || max_tests=$1
[[ $2 == "" ]] || test_percent=$2

./waker.sh &

for i in `seq $max_tests` ; do 
	./run.sh $test_percent &
done

