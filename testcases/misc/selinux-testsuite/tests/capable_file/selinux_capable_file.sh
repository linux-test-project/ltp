#!/bin/sh
#
# This test performs capability tests for file operations.
#

setup()
{
	LTPTMP="/tmp/selinux"
	export TCID="setup"
	export TST_COUNT=0

	# Clean up from a previous run
	rm -f $LTPTMP/temp_file 2>&1
	rm -f $LTPTMP/temp_file2 2>&1
}

#
# Tests for the good domains.
#

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# CAP_CHOWN
	touch $LTPTMP/temp_file 2>&1
	runcon -t test_fcap_t -- chown daemon $LTPTMP/temp_file 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		echo "Test #1: capable_file passed."
	else
		echo "Test #1: capable_file failed."
	fi
	return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# CAP_FOWNER
	chown daemon.tty $LTPTMP/temp_file 2>&1
	runcon -t test_fcap_t -- chmod 0400 $LTPTMP/temp_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #2: capable_file failed."
	else
		echo  "Test #2: capable_file passed."
	fi
	return $RC
}

test03()
{

	TCID="test03"
	TST_COUNT=3
	RC=0

	# CAP_FSETID
	MODE_BEFORE=`stat --format %a $LTPTMP/temp_file` 
	runcon -t test_fcap_t -- chmod g+rs $LTPTMP/temp_file 2>&1
	MODE_AFTER=`stat --format %a $LTPTMP/temp_file`

	# prior mode should not be same as current mode
	if [ $MODE_BEFORE -eq $MODE_AFTER ] 
	then
		echo "Test #3: capable_file failed."
		return 1
	else
		echo "Test #3: capable_file passed."
		return 0 
	fi
}

test04()
{
	TCID="test04"
	TST_COUNT=4
	RC=0

	# CAP_LEASE
	runcon -t test_fcap_t --  selinux_lease $LTPTMP/temp_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #4: capable_file failed."
	else
		echo "Test #4: capable_file passed."
	fi
	return $RC
}

test05()
{

	TCID="test05"
	TST_COUNT=5
	RC=0

	# CAP_MKNOD
	runcon -t test_fcap_t -- mknod $LTPTMP/temp_file2 c 5 5 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #5: capable_file failed."
	else
		echo "Test #5: capable_file passed."
	fi
	return $RC
}

#
# Tests for the bad domain.
#
test06()
{
	TCID="test06"
	TST_COUNT=6
	RC=0

	# CAP_CHOWN
	touch $LTPTMP/temp_file 2>&1
	runcon -t test_nofcap_t -- chown daemon $LTPTMP/temp_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #6: capable_file passed."
		return 0
	else
		echo "Test #6: capable_file failed."
		return 1
	fi
}
	
test07()
{
	TCID="test07"
	TST_COUNT=7
	RC=0

	# CAP_FOWNER
	chown daemon.tty $LTPTMP/temp_file 2>&1
	runcon -t test_nofcap_t -- chmod 0400 $LTPTMP/temp_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #7: capable_file passed."
		return 0
	else
		echo "Test #7: capable_file failed."
		return 1
	fi
}

test08()
{
	TCID="test08"
	TST_COUNT=8
	RC=0

	# CAP_FSETID - Domain needs CAP_FOWNER
	MODE_BEFORE=`stat --format %a $LTPTMP/temp_file`
	runcon -t test_resfcap_t -- chmod g+rs $LTPTMP/temp_file 2>&1

	MODE_AFTER=`stat --format %a $LTPTMP/temp_file`
	# prior mode should be same as current mode
	if [ $MODE_BEFORE -eq $MODE_AFTER ] 
	then
		echo "Test #8: capable_file passed."
		return 0
	else
		echo "Test #8: capable_file failed."
	 	return 1	
	fi 
}

test09()
{
	TCID="test09"
	TST_COUNT=9
	RC=0

	# CAP_LEASE - Needs DAC_OVERRIDE
	runcon -t test_resfcap_t --  selinux_lease $LTPTMP/temp_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #9: capable_file passed."
		return 0
	else
		echo "Test #9: capable_file failed."
		return 1
	fi
}

test10()
{
	TCID="test10"
	TST_COUNT=10
	RC=0

	# CAP_MKNOD - Domain needs CAP_DAC_OVERRIDE
	runcon -t test_resfcap_t -- mknod $LTPTMP/temp_file2 c 5 5 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #10: capable_file passed."
		return 0
	else
		echo "Test #10: capable_file failed."
		return 1
	fi
}

cleanup()
{
	rm -f $LTPTMP/temp_file 2>&1
	rm -f $LTPTMP/temp_file2 2>&1
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
cleanup
test06 || exit $RC
test07 || exit $RC
test08 || exit $RC
test09 || exit $RC
test10 || exit $RC
cleanup
exit 0
