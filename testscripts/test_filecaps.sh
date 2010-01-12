#!/bin/bash
#
# Copyright 2007 IBM
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# test_filecaps.sh - Run the file capabilities test suite.

# Must be root to run the containers testsuite
if [ $UID != 0 ]
then
        echo "FAILED: Must be root to execute this script"
        exit 1
fi

# set the LTPROOT directory
cd `dirname $0`
LTPROOT=${PWD}
echo $LTPROOT | grep testscripts > /dev/null 2>&1
if [ $? -eq 0 ]
then
	cd ..
	LTPROOT=${PWD}
fi

# set the PATH to include testcase/bin

export PATH=$PATH:/usr/sbin:$LTPROOT/testcases/bin
export LTPBIN=$LTPROOT/testcases/bin

# We will store the logfiles in $LTPROOT/results, so make sure
# it exists.
if [ ! -d $LTPROOT/results ]
then
	mkdir $LTPROOT/results
fi

# Check the role and mode testsuite is being executed under.
echo "Running the file capabilities testsuite..."

$LTPROOT/bin/ltp-pan -S -a $LTPROOT/results/filecaps -n ltp-filecaps -l $LTPROOT/results/filecaps.logfile -o $LTPROOT/results/filecaps.outfile -p -f $LTPROOT/runtest/filecaps

echo "Done."
exit 0
