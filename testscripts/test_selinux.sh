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
export LTPROOT=${PWD}
echo $LTPROOT | grep testscripts > /dev/null 2>&1
if [ $? -eq 0 ] 
then
	cd ..
	export LTPROOT=${PWD}
fi

# set the PATH to include testcase/bin

export PATH=$PATH:$LTPROOT/testcases/bin
export LTPBIN=$LTPROOT/testcases/bin

# We will store the logfiles in $LTPROOT/results, so make sure
# it exists.
if [ ! -d $LTPROOT/results ]
then 
	mkdir $LTPROOT/results
fi
	
# Check the role and mode testsuite is being executed under.

SELINUX_CONTEXT=`id | sed 's/.* //'`
SELINUX_ROLE=`id | sed 's/.* //' | awk -F: '{ print $2 }'`

echo "Running with security $SELINUX_CONTEXT"

if [ $SELINUX_ROLE != sysadm_r ] && [ $SELINUX_ROLE != system_r ]
then
	echo "These tests are intended to be run in the sysadm role."
	exit
fi

SELINUX_MODE=`getenforce`
if [ $SELINUX_MODE != Enforcing ] && [ $SELINUX_MODE != enforcing ]
then
	echo "These tests are intended to be run in enforcing mode only."
	echo "Run 'setenforce 1' to switch to enforcing mode."
	exit
fi

# build and install the test policy...
echo "building and installing test policy..."

cd $LTPROOT/testcases/misc/selinux-testsuite/policy
make load > /dev/null
if [ $? != 0 ]; then
  echo "Failed to build test policy, therefore aborting test run."
  exit 1
fi

#echo "Successfully built test policy."

# go back to test's root directory
cd $LTPROOT

echo "Running the SELinux testsuite..."

# /tmp/selinux directory is used by this testsuite. However, 
# it needs to have the test_file_t for testcases to run successfully. 
# Each component of path needs this type. 
# Save and restore /tmp's type.
SAVETMPTYPE=`ls -Zd /tmp | awk -F: '{ print $3 }'`
chcon -t test_file_t /tmp

mkdir /tmp/selinux > /dev/null 2>&1
chcon -t test_file_t /tmp/selinux

# The ../testcases/bin directory needs to have the test_file_t type.
chcon -R -t test_file_t $LTPROOT/testcases/bin

$LTPROOT/pan/pan -S -a $LTPROOT/results/selinux -n ltp-selinux -l $LTPROOT/results/selinux.logfile -o $LTPROOT/results/selinux.outfile -p -f $LTPROOT/runtest/selinux  

# cleanup before exiting    

# Restore type of /tmp
chcon -t $SAVETMPTYPE /tmp
rm -rf /tmp/selinux

echo "Removing test policy and reloading original policy..."
cd $LTPROOT/testcases/misc/selinux-testsuite/policy
make cleanup > /dev/null
if [ $? != 0 ]; then
  echo "Failed to reload original policy."
  exit 1
fi
cd $LTPROOT
echo "Done."
exit 0
