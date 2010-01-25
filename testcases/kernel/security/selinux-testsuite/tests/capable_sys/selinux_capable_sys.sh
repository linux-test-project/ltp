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
        export TCID="setup"
        export TST_COUNT=0
	export TST_TOTAL=8

	SELINUXTMPDIR=$(mktemp -d)
	chcon -t test_file_t $SELINUXTMPDIR
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
	touch $SELINUXTMPDIR/temp_file 2>&1
	runcon -t test_scap_t -- selinux_rawio $SELINUXTMPDIR/temp_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TFAIL "capable_sys failed."
	else
		tst_resm TPASS "capable_sys passed."
	fi
	return $RC
}

test02()
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	# CAP_SYS_CHROOT
	runcon -t test_scap_t -- selinux_chroot $SELINUXTMPDIR/ 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TFAIL "capable_sys failed."
	else
		tst_resm TPASS "capable_sys passed."
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
		tst_resm TFAIL "capable_sys failed."
	else
		tst_resm TPASS "capable_sys passed."
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
		tst_resm TFAIL "capable_sys failed."
	else
		tst_resm TPASS "capable_sys passed."
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
	touch $SELINUXTMPDIR/temp_file 2>&1
	runcon -t test_noscap_t -- selinux_rawio $SELINUXTMPDIR/temp_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "capable_sys passed."
		RC=0
	else
		tst_resm TFAIL "capable_sys failed."
		RC=1
	fi
	return $RC
}

test06()
{
        TCID="test06"
        TST_COUNT=6
        RC=0

	# CAP_SYS_CHROOT
	runcon -t test_noscap_t -- selinux_chroot $SELINUXTMPDIR/ 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "capable_sys passed."
		RC=0
	else
		tst_resm TFAIL "capable_sys failed."
		RC=1
	fi
	return $RC
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
		tst_resm TPASS "capable_sys passed."
		RC=0
	else
		tst_resm TFAIL "capable_sys failed."
		RC=1
	fi
	return $RC
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
		tst_resm TPASS "capable_sys passed."
		RC=0
	else
		tst_resm TFAIL "capable_sys failed."
		RC=1
	fi
	return $RC
}

cleanup()
{
	rm -rf $SELINUXTMPDIR
}

# Function:     main
#
# Description:  - Execute all tests, exit with test status.
#
# Exit:         - zero on success
#               - non-zero on failure.
#
RC=0    # Return value from setup, and test functions.
EXIT_VAL=0

setup  
test01 || EXIT_VAL=$RC
test02 || EXIT_VAL=$RC
test03 || EXIT_VAL=$RC
test04 || EXIT_VAL=$RC
cleanup
setup
test05 || EXIT_VAL=$RC
test06 || EXIT_VAL=$RC
test07 || EXIT_VAL=$RC
test08 || EXIT_VAL=$RC
cleanup
exit $EXIT_VAL 
