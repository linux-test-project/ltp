#!/bin/sh
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
	pushd /etc/selinux > /dev/null
	cp --preserve semanage.conf semanage.conf.orig
	echo "expand-check=0" >> semanage.conf
	popd > /dev/null
}

config_unset_expandcheck() {
	pushd /etc/selinux > /dev/null
	mv semanage.conf.orig semanage.conf
	popd > /dev/null
}

config_allow_domain_fd_use () {
	setval=$1
	if /usr/sbin/getsebool allow_domain_fd_use; then
		echo "allow_domain_fd_use exists setting"
		/usr/sbin/setsebool allow_domain_fd_use=$setval
	fi
}

# Must be root to run the selinux testsuite
if [ $(id -ru) -ne 0 ]; then
        echo "${0##*/} FAILED: Must be root to execute this script"
        exit 1
fi

# set the LTPROOT directory
LTPROOT=$(readlink -f "${LTPROOT:=${0%/*}}")
cd "$LTPROOT"
export TMP=${TMP:-/tmp}
# If we're in the testscripts directory, go up a dir..
LTPROOT_TMP=${LTPROOT%/testscripts}
if [ "x${LTPROOT_TMP}" != "x${LTPROOT}" ]
then
	cd ..
	LTPROOT=$LTPROOT_TMP
fi
export LTPROOT
unset LTPROOT_TMP

# set the PATH to include testcase/bin

export LTPBIN=$LTPROOT/testcases/bin
export PATH=$PATH:/usr/sbin:$LTPBIN

# We will store the logfiles in $LTPROOT/results, so make sure
# it exists.
test -d $LTPROOT/results || /bin/mkdir $LTPROOT/results

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

POLICYDIR="$LTPROOT/testcases/kernel/security/selinux-testsuite"

if [ -x $SEMODULE ]; then
	POLICYDIR="$POLICYDIR/refpolicy"
else
	POLICYDIR="$POLICYDIR/policy"
fi

config_set_expandcheck

config_allow_domain_fd_use 0

# install the test policy...
echo "Installing test_policy module..."
if ! semodule -i $POLICYDIR/test_policy.pp; then
	echo "Failed to install test_policy module, aborting test run."
	config_unset_expandcheck
	exit 1
else
	echo "Successfully loaded test_policy module."
fi

config_unset_expandcheck

echo "Running the SELinux testsuite..."

mkdir $TMP/selinux > /dev/null 2>&1
/usr/bin/chcon -R -t test_file_t $TMP/selinux
export SELINUXTMPDIR=$TMP/selinux

# The ../testcases/bin directory needs to have the test_file_t type.
# Save and restore later.
SAVEBINTYPE=`ls -Zd $LTPROOT/testcases/bin | awk '{ print $4 }' | awk -F: '{ print $3 }'`
/usr/bin/chcon -R -t test_file_t "$LTPROOT/testcases/bin"

$LTPROOT/bin/ltp-pan -S -a $LTPROOT/results/selinux -n ltp-selinux \
	-l $LTPROOT/results/selinux.logfile \
	-o $LTPROOT/results/selinux.outfile -p \
	-f $LTPROOT/runtest/selinux
# cleanup before exiting    

rm -rf $TMP/selinux

# Restore type of .../testcases/bin directory
/usr/bin/chcon -R -t $SAVEBINTYPE $LTPROOT/testcases/bin

echo "Removing test_policy module..."
if ! semodule -r test_policy; then
	echo "Failed to remove test_policy module."
	exit 1
fi

config_allow_domain_fd_use 1

echo "Done."
