#!/bin/sh
#
# This test performs ioctl access on a file.
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

	# Create a temporary file for testing
	rm -f $LTPTMP/temp_file 2>&1
	touch $LTPTMP/temp_file 2>&1
	chcon -t test_ioctl_file_t $LTPTMP/temp_file 2>&1
}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Attempt to perform the ioctls on the temproary
	# file as the good domain
	runcon -t test_ioctl_t -- selinux_ioctl $LTPTMP/temp_file 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                echo "Test #1: ioctl passed."
        else
                echo "Test #1: ioctl failed."
        fi
        return $RC
}


test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Attempt to perform the ioctls on the temproary file as the bad domain
	# The test program, test_noioctl.c, determines success/failure for the
	# individual calls, so we expect success always from that program.
	runcon -t test_noioctl_t -- selinux_noioctl $LTPTMP/temp_file 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                echo "Test #2: ioctl passed."
        else
                echo "Test #2: ioctl failed."
        fi
        return $RC
}

cleanup()
{
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

setup 
test01 || exit $RC
test02 || exit $RC
cleanup
exit 0
