#!/bin/bash 

# Copyright (C) 2004 Dan Carpenter
# This software is released under the terms of the GPL

SLEEP_SECS=1
SEGV_SECS=4

if ps | grep -q tty ; then
	delim='t'
	tty="tty"
else
	delim='p'
	tty="pts"
fi

secs=0
while true ; do 
        # fixme (hack) assumes tests in test/ dir
        # assumes ltp naming scheme with a number on the
        # end of each test script
	for i in `ps x | \
                  grep test | \
                  grep [0-9]$ | \
                  cut -d $delim -f 1` ; do
		if [ $secs -eq $SEGV_SECS ] ; then
			kill -SEGV $i
		else
			kill -CONT $i
		fi
	done
	if [ $secs -ge $SEGV_SECS ] ; then
		secs=0
	else
		secs=$(($secs + 1))
	fi

	sleep $SLEEP_SECS
done
