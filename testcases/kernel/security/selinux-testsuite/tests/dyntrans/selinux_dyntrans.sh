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
	export TCID="setup"
	export TST_COUNT=0
	export TST_TOTAL=2
}

test01()
{
        TCID="test01"
        TST_COUNT=1
        RC=0

	# Verify that notfromdomain cannot transition to todomain.
	# Should fail on the transition permission check.
	runcon -t test_dyntrans_notfromdomain_t -- "selinux_dyntrans_parent" test_dyntrans_todomain_t 2>&1
	RC=$?
	if [ $RC -ne 0 ]	# we expect this to fail
	then
		tst_resm TPASS "dyntrans passed."
		RC=0
	else
		tst_resm TFAIL "dynstrans failed."
		RC=1
	fi
	return $RC
}

test02()
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	# Verify that fromdomain can transition to todomain.
	runcon -t test_dyntrans_fromdomain_t -- "selinux_dyntrans_parent" test_dyntrans_todomain_t 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "dyntrans passed."
	else
		tst_resm TFAIL "dynstrans failed."
	fi
	return $RC
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
exit $EXIT_VAL 
