#!/bin/sh
#
# Copyright (c) 2002 Network Associates Technology, Inc.
# Copyright (c) International Business Machines  Corp., 2005
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#

setup()
{
	LTPTMP="/tmp/selinux"
	export TCID="setup"
	export T_COUNT=0

	sysctl="kernel.modprobe"
	oldval=`/sbin/sysctl -n $sysctl`
}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	runcon -t test_sysctl_t -- /sbin/sysctl -n $sysctl 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		echo "Test #1: sysctl passed."
	else
		echo "Test #1: sysctl failed."
	fi
	return $RC
}

test02()
{

	TCID="test02"
	TST_COUNT=2
	RC=0

	runcon -t test_sysctl_t -- /sbin/sysctl -w $sysctl=$oldval 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		echo "Test #2: sysctl passed."
	else
		echo "Test #2: sysctl failed."
	fi
	return $RC
}

test03()
{
	TCID="test03"
	TST_COUNT=3
	RC=0

	runcon -t test_nosysctl_t -- /sbin/sysctl -n $sysctl 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #3: sysctl passed."
		RC=0
	else
		echo "Test #3: sysctl failed."
		RC=1
	fi
	return $RC
}

test04()
{
	TCID="test04"
	TST_COUNT=4
	RC=0

	runcon -t test_nosysctl_t -- /sbin/sysctl -w $sysctl=foobar 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #4: sysctl passed."
		RC=0
	else
		echo "Test #4: sysctl failed."
		RC=1
	fi
	return $RC
}

#
# Description:  - Execute all tests, exit with test status.
#
# Exit:         - zero on success
#               - non-zero on failure.
#
RC=0    # Return value from setup, and test functions.

setup
test01 || exit $RC
test02 || exit $RC
test03 || exit $RC
test04 || exit $RC
exit 0
