#!/bin/bash

# Copyright (C) 2004 Dan Carpenter
# This software is released under the terms of the GPL

progname=$0
dir=`dirname $progname`
[ $dir == "." ] && dir=`pwd`

if [[ "$1" == "-h" || "$1" == "-help" ]] ; then
    echo "$progname [user][max_tests][test_percent]"
    echo "        user => username to the scripts under"
    echo "   max_tests => maximun concurrent tests to run"
    echo "test_percent => percent of the syscalls to test"
    exit 0
fi

if [[ $1 == "" ]] ; then
    echo "Enter a user name to run the test under"
    read user
else
    user="$1"
fi

[[ "$2" == "" ]] || max_tests="$2"
[[ "$3" == "" ]] || test_percent="$3"

if [ ! -e test_list.txt ] ; then
    echo "Enter the path to the ltp scripts"
    read ltp_path
    echo "Creating test_list.txt"
    find $ltp_path -type f -name \*[0-9] > test_list.txt
    tmp=`cat test_list.txt | wc -l`
    echo "$tmp test scripts found"
fi

trap "echo \"CTRL-C Pressed.  Exiting\"
./slay $user
exit 0
" 2

while true ; do
	chmod +x $dir
	su $user -c ./test.sh
	sleep 8
	./slay $user
done
	
