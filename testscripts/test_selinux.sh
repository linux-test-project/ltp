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

config_set_expandcheck() {
	pushd /etc/selinux
	cp --preserve semanage.conf semanage.conf.orig
	echo "expand-check=0" >> semanage.conf
	popd
}

config_unset_expandcheck() {
	pushd /etc/selinux
	mv semanage.conf.orig semanage.conf
	popd
}

config_allow_domain_fd_use () {
    setval=$1
    /usr/sbin/getsebool allow_domain_fd_use
    getseRC=$?
    if [ "$getseRC" -eq "0" ]; then
	echo "allow_domain_fd_use exists setting"
	/usr/sbin/setsebool allow_domain_fd_use=$setval
    fi
}

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

echo "Running with security $SELINUX_CONTEXT"

SELINUX_MODE=`/usr/sbin/getenforce`
if [ $SELINUX_MODE != Enforcing ] && [ $SELINUX_MODE != enforcing ]
then
	echo "These tests are intended to be run in enforcing mode only."
	echo "Run 'setenforce 1' to switch to enforcing mode."
	exit
fi

SEMODULE="/usr/sbin/semodule"

if [ -f $SEMODULE ]; then
    POLICYDIR="$LTPROOT/testcases/kernel/security/selinux-testsuite/refpolicy"
else
    POLICYDIR="$LTPROOT/testcases/kernel/security/selinux-testsuite/policy"
fi

# Update test policy if needed
pushd $LTPROOT/testcases/kernel/security/selinux-testsuite/misc
sh ./update_refpolicy.sh
popd

config_set_expandcheck

config_allow_domain_fd_use 0

# build and install the test policy...
echo "building and installing test_policy module..."
cd $POLICYDIR
make load
if [ $? != 0 ]; then
	echo "Failed to build and load test_policy module, aborting test run."
	config_unset_expandcheck
	exit 1
else
	echo "Successfully built and loaded test_policy module."
fi

config_unset_expandcheck

# go back to test's root directory
cd $LTPROOT

echo "Running the SELinux testsuite..."

mkdir /tmp/selinux > /dev/null 2>&1
/usr/bin/chcon -t test_file_t /tmp/selinux
export SELINUXTMPDIR=/tmp/selinux

# The ../testcases/bin directory needs to have the test_file_t type.
# Save and restore later.
SAVEBINTYPE=`ls -Zd $LTPROOT/testcases/bin | awk '{ print $4 }' | awk -F: '{ print $3 }'`
/usr/bin/chcon -t test_file_t $LTPROOT/testcases/bin

$LTPROOT/pan/pan -S -a $LTPROOT/results/selinux -n ltp-selinux -l $LTPROOT/results/selinux.logfile -o $LTPROOT/results/selinux.outfile -p -f $LTPROOT/runtest/selinux  

# cleanup before exiting    

rm -rf /tmp/selinux

# Restore type of .../testcases/bin directory
/usr/bin/chcon -t $SAVEBINTYPE $LTPROOT/testcases/bin

echo "Removing test_policy module..."
cd $POLICYDIR
make cleanup 2>&1
if [ $? != 0 ]; then
	echo "Failed to remove test_policy module."
	exit 1
fi

config_allow_domain_fd_use 1

cd $LTPROOT
echo "Done."
exit 0
