#!/bin/sh
#
# This test performs checks for network-related capabilties.
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
        export TST_COUNT=0

        # Clean up from a previous run
        rm -f $LTPTMP/temp_file 2>&1
}

#
# Tests for the good domain.
#
test01()
{
        TCID="test01"
        TST_COUNT=1
        RC=0

	# CAP_SYS_RAWIO
	touch $LTPTMP/temp_file 2>&1
	runcon -t test_scap_t -- selinux_rawio $LTPTMP/temp_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #1: capable_sys failed."
	else
		echo "Test #1: capable_sys passed."
	fi
	return $RC
}

test02()
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	# CAP_SYS_CHROOT
	runcon -t test_scap_t -- selinux_chroot $LTPTMP/ 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #2: capable_sys failed."
	else
		echo "Test #2: capable_sys passed."
	fi
	return $RC
}

# CAP_SYS_PTRACE - Not done here.
# CAP_SYS_PACCT - Not done; needs support built into the kernel

test03()
{
        TCID="test03"
        TST_COUNT=3
        RC=0

	# CAP_SYS_ADMIN 
	runcon -t test_scap_t -- selinux_hostname 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #3: capable_sys failed."
	else
		echo "Test #3: capable_sys passed."
	fi
	return $RC
}

# CAP_SYS_BOOT - Not done; too dangerous

# CAP_SYS_NICE
test04()
{
        TCID="test04"
        TST_COUNT=4
        RC=0

	runcon -t test_scap_t -- selinux_nice 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #4: capable_sys failed."
	else
		echo "Test #4: capable_sys passed."
	fi
	return $RC
}

# CAP_SYS_RESOURCE - Not done.

# CAP_SYS_TIME - Not done.

# CAP_SYS_TTY_CONFIG - Not done; can result in a terminal hangup.

#
# Tests for the bad domain.
#

test05()
{
        TCID="test05"
        TST_COUNT=5
        RC=0

	# CAP_SYS_RAWIO
	touch $LTPTMP/temp_file 2>&1
	runcon -t test_noscap_t -- selinux_rawio $LTPTMP/temp_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #5: capable_sys passed."
		return 0
	else
		echo "Test #5: capable_sys failed."
		return 1
	fi
}

test06()
{
        TCID="test06"
        TST_COUNT=6
        RC=0

	# CAP_SYS_CHROOT
	runcon -t test_noscap_t -- selinux_chroot $LTPTMP/ 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #6: capable_sys passed."
		return 0
	else
		echo "Test #6: capable_sys failed."
		return 1
	fi
}

test07()
{
        TCID="test07"
        TST_COUNT=7
        RC=0

	# CAP_SYS_ADMIN 
	runcon -t test_noscap_t -- selinux_hostname 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #7: capable_sys passed."
		return 0
	else
		echo "Test #7: capable_sys failed."
		return 1
	fi
}

test08()
{
        TCID="test08"
        TST_COUNT=8
        RC=0

	# CAP_SYS_NICE
	runcon -t test_noscap_t -- selinux_nice 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #8: capable_sys passed."
		return 0
	else
		echo "Test #8: capable_sys failed."
		return 1
	fi
}

cleanup()
{
	# Remove files
	rm -f $LTPTMP/temp_file 2>&1
}

# Function:     main
#
# Description:  - Execute all tests, exit with test status.
#
# Exit:         - zero on success
#               - non-zero on failure.
#
RC=0    # Return value from setup, and test functions.

setup  || exit $RC
test01 || exit $RC
test02 || exit $RC
test03 || exit $RC
test04 || exit $RC
cleanup
test05 || exit $RC
test06 || exit $RC
test07 || exit $RC
test08 || exit $RC
cleanup
exit 0
