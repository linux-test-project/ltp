#!/bin/sh
#
# Copyright (c) 2008 NEC Corporation
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
	export TST_TOTAL=4

	if ! SELINUXTMPDIR=$(mktemp -d); then
		tst_resm TCONF "mktemp -d failed"
	elif ! chcon -t test_file_t $SELINUXTMPDIR; then
		tst_resm TCONF "chcon -t test_file_t $SELINUXTMPDIR failed"
	else

		# Create test files
		dd if=/dev/zero of=$SELINUXTMPDIR/bounds_file      count=1
		dd if=/dev/zero of=$SELINUXTMPDIR/bounds_file_red  count=1
		dd if=/dev/zero of=$SELINUXTMPDIR/bounds_file_blue count=1

		chcon -t test_bounds_file_t      $SELINUXTMPDIR/bounds_file
		chcon -t test_bounds_file_red_t  $SELINUXTMPDIR/bounds_file_red
		chcon -t test_bounds_file_blue_t $SELINUXTMPDIR/bounds_file_blue

	fi

}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	runcon -t test_bounds_parent_t \
	       -- selinux_bounds_thread test_bounds_child_t 2>&1
	RC=$?
	if [ $RC -eq 0 ];
	then
		tst_resm TPASS "thread dyntrans passed."
	else
		tst_resm TFAIL "thread dynstrans failed."
	fi
	return $RC
}

test02()
{
	TCID="test02"
	TST_COUND=2
	RC=0

	runcon -t test_bounds_parent_t \
	       -- selinux_bounds_thread test_bounds_unbound_t 2>&1
	RC=$?
	if [ $RC -ne 0 ];	# we expect this to fail
	then
		tst_resm TPASS "thread dyntrans to unbound domain failed."
		RC=0
	else
		tst_resm TFAIL "thread dyntrans to unbound domain succeeded."
		RC=1
	fi
	return $RC
}

test03()
{
	TCID="test03"
	TST_COUND=3
	RC=0

	runcon -t test_bounds_child_t \
		-- dd if=$SELINUXTMPDIR/bounds_file of=/dev/null count=1
	RC=$?
	if [ $RC -eq 0 ];
	then
		tst_resm TPASS "unbounded action to be allowed."
	else
		tst_resm TFAIL "unbounded action to be allowed."
	fi
	return $RC
}

test04()
{
	TCID="test04"
	TST_COUNT=4
	RC=0

	runcon -t test_bounds_child_t \
		-- dd if=/dev/zero of=$SELINUXTMPDIR/bounds_file count=1
	RC=$?
	if [ $RC -ne 0 ];	# we expect this to fail
	then
		tst_resm TPASS "bounded action to be denied."
		RC=0
	else
		tst_resm TFAIL "bounded action to be denied."
		RC=1
	fi
	return $RC
}

test05()
{
	TCID="test05"
	TST_COUNT=5
	RC=0

	runcon -t test_bounds_parent_t \
	       -- dd if=/dev/zero of=$SELINUXTMPDIR/bounds_file_red count=1
	RC=$?
	if [ $RC -ne 0 ];	# we expect this to fail
	then
		tst_resm TPASS "actions to bounded type to be denied."
		RC=0
	else
		tst_resm TFAIL "actions to bounded type to be denied."
		RC=1
	fi
	return $RC
}

test06()
{
	TCID="test06"
	TST_COUNT=6
	RC=0

	runcon -t test_bounds_child_t -- chmod 0777 $SELINUXTMPDIR/bounds_file_blue
	RC=$?
	if [ $RC -eq 0 ];
	then
		tst_resm TPASS "bounds of subject can setattr bounds of target"
	else
		tst_resm TFAIL "bounds of subject can setattr bounds of target"
	fi
	return $RC
}

cleanup()
{
	rm -rf $SELINUXTMPDIR
}

# Function:	main
#
# Description:	- Execute all tests, exit with test status.
#
# Exit:		- zero on success
#		- non-zero on failure.
#
RC=0	# Return value from setup, and test functions.
EXIT_VAL=0

if setup ; then
	i=1
	while [ $i -le 6 ] ; do
		if ! eval "test0$i"; then
			 : $(( EXIT_VAL |= $RC ))
		fi
		: $(( i += 1 ))
	done
	cleanup
else
	tst_resm TCONF "setup failed"
	EXIT_VAL=1
fi
exit $EXIT_VAL
