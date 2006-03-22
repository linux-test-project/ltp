#!/bin/bash
#
# Copyright (c) International Business Machines  Corp., 2005
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# test_selinux.sh - Run the selinux test suite.

# Must be root to run the selinux testsuite
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
	/bin/mkdir $LTPROOT/results
fi
	
# Check the role and mode testsuite is being executed under.

SELINUX_CONTEXT=`/usr/bin/id | sed 's/.* //'`
SELINUX_ROLE=`/usr/bin/id | sed 's/.* //' | awk -F: '{ print $2 }'`

echo "Running with security $SELINUX_CONTEXT"

if [ $SELINUX_ROLE != sysadm_r ] && [ $SELINUX_ROLE != system_r ]
then
	echo "These tests are intended to be run in the sysadm role."
	exit
fi

SELINUX_MODE=`/usr/sbin/getenforce`
if [ $SELINUX_MODE != Enforcing ] && [ $SELINUX_MODE != enforcing ]
then
	echo "These tests are intended to be run in enforcing mode only."
	echo "Run 'setenforce 1' to switch to enforcing mode."
	exit
fi

# build and install the test policy...
echo "building and installing test_policy module..."
cd $LTPROOT/testcases/kernel/security/selinux-testsuite/refpolicy
make load
if [ $? != 0 ]; then
	echo "Failed to build and load test_policy module, aborting test run."
	exit 1
else
	echo "Successfully built and loaded test_policy module."
fi

# go back to test's root directory
cd $LTPROOT

echo "Running the SELinux testsuite..."

# Save and later restore /tmp's type.
SAVETMPTYPE=`ls -Zd /tmp | awk '{ print $4 }' | awk -F: '{ print $3 }'`
/usr/bin/chcon -t test_file_t /tmp

mkdir /tmp/selinux > /dev/null 2>&1
/usr/bin/chcon -t test_file_t /tmp/selinux
export SELINUXTMPDIR=/tmp/selinux

# The ../testcases/bin directory needs to have the test_file_t type.
# Save and restore later.
SAVEBINTYPE=`ls -Zd $LTPROOT/testcases/bin | awk '{ print $4 }' | awk -F: '{ print $3 }'`
/usr/bin/chcon -t test_file_t $LTPROOT/testcases/bin

$LTPROOT/pan/pan -S -a $LTPROOT/results/selinux -n ltp-selinux -l $LTPROOT/results/selinux.logfile -o $LTPROOT/results/selinux.outfile -p -f $LTPROOT/runtest/selinux  

# cleanup before exiting    

# Restore type of /tmp
/usr/bin/chcon -t $SAVETMPTYPE /tmp
rm -rf /tmp/selinux

# Restore type of .../testcases/bin directory
/usr/bin/chcon -t $SAVEBINTYPE $LTPROOT/testcases/bin

echo "Removing test_policy module..."
cd $LTPROOT/testcases/kernel/security/selinux-testsuite/refpolicy
make cleanup 2>&1
if [ $? != 0 ]; then
	echo "Failed to remove test_policy module."
	exit 1
fi
cd $LTPROOT
echo "Done."
exit 0
