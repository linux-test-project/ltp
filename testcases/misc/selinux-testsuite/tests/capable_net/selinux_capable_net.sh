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
}

#
# Tests for the good domain.
#

test01()
{
        TCID="test01"
        TST_COUNT=1
        RC=0

	# CAP_NET_ADMIN
	runcon -t test_ncap_t -- /sbin/ifconfig lo -promisc 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #1: capable_net failed."
        else
                echo "Test #1: capable_net passed."
        fi
        return $RC
}	

test02()
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	# CAP_NET_BIND_SERVICE
	runcon -t test_ncap_t -- selinux_bind 2>&1
	RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #2: capable_net failed."
        else
                echo "Test #2: capable_net passed."
        fi
        return $RC
}

# CAP_NET_BROADCAST - Not done. Kernel does not check this capability yet.

test03()
{
	
        TCID="test03"
        TST_COUNT=3
        RC=0

	# CAP_NET_RAW
	runcon -t test_ncap_t -- selinux_bind 2>&1
	RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #3: capable_net failed."
        else
                echo "Test #3: capable_net passed."
        fi
        return $RC
}

#
# Tests for the bad domain.
#

test04()
{
        TCID="test04"
        TST_COUNT=4
        RC=0

	# CAP_NET_ADMIN
	runcon -t test_resncap_t -- /sbin/ifconfig lo -promisc 2>&1
	RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #4: capable_net passed."
		return 0
        else
                echo "Test #4: capable_net failed."
		return 1
        fi
}

# CAP_NET_BIND_SERVICE; included in can_network by fedora policy

test05()
{
	
        TCID="test05"
        TST_COUNT=5
        RC=0

	# CAP_NET_RAW - Domain requires rawip_socket create permission
	runcon -t test_resncap_t -- selinux_raw 2>&1
	RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #5: capable_net passed."
		return 0
        else
                echo "Test #5: capable_net failed."
		return 1
        fi
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
test05 || exit $RC
exit 0
