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

global_setup()
{
	# Must be root to run the selinux testsuite
	if [ $UID != 0 ]
	then
        	echo "FAILED: Must be root to execute this script"
        	exit 1
	fi

	# set the testcases/bin directory
	cd ../../../../bin
	export LTPBIN=${PWD}
	cd -
	export PATH=$PATH:/$LTPBIN


	# Check the role and mode testsuite is being executed under.
	SELINUX_CONTEXT=`id | sed 's/.* //'`

	echo "Running with security $SELINUX_CONTEXT"

	SELINUX_MODE=`getenforce`
	if [ $SELINUX_MODE != Enforcing ] && [ $SELINUX_MODE != enforcing ]
	then
		echo "These tests are intended to be run in enforcing mode only."
		echo "Run 'setenforce 1' to switch to enforcing mode."
		exit
	fi

	# Save and later restore /tmp's type.
	# We need to change it's type to work within test domain
	SAVETMPTYPE=`ls -Zd /tmp | awk '{ print $4 }' | awk -F: '{ print $3 }'`
	chcon -t test_file_t /tmp

	mkdir /tmp/selinux > /dev/null 2>&1
	chcon -t test_file_t /tmp/selinux
	export SELINUXTMPDIR=/tmp/selinux
	
	# It seems LTP wants executables to reside in the
	# $LTPROOT/testcases/bin directory. However, this directory
	# needs test_file_t type in order to run selinux test executables.
	# Save original type, change to test_file_t and restore 
	# the original type later after tests have been run.
	SAVEBINTYPE=`ls -Zd $LTPBIN | awk '{ print $4 }' | awk -F: '{ print $3 }'`
	chcon -t test_file_t $LTPBIN
}


# cleanup before exiting
global_cleanup()
{

	# Restore original type of /tmp
	chcon -t $SAVETMPTYPE /tmp
	rm -rf /tmp/selinux

	# Restore original type of .../testcases/bin directory
	chcon -t $SAVEBINTYPE $LTPBIN
	echo "Done."
	exit 0
}

global_setup
./$1/selinux_$1.sh
global_cleanup
