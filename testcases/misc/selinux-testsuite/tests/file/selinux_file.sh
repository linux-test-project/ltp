#!/bin/sh
#
# This test performs access checks for a file.
#

setup()
{
        LTPTMP="/tmp/selinux"
        export TCID="setup"
        export TST_COUNT=0

        # Clean up from a previous run
	rm -f $LTPTMP/temp_file 2>&1
	rm -f $LTPTMP/temp_file2 2>&1
	rm -f $LTPTMP/temp_file3 2>&1

	#
	# Create the temp files
	#
	dd if=/dev/zero of=$LTPTMP/temp_file count=2 ibs=1024 2>&1 > /dev/null
	dd if=/dev/zero of=$LTPTMP/temp_file2 count=2 ibs=1024 2>&1 > /dev/null
	dd if=/dev/zero of=$LTPTMP/temp_file3 count=2 ibs=1024 2>&1 > /dev/null
	chmod 775 $LTPTMP/temp_file 2>&1 > /dev/null
	chmod 775 $LTPTMP/temp_file2 2>&1 > /dev/null

	#
	# Change the context for the file the good domain only has access to.
	#
	chcon -t fileop_file_t $LTPTMP/temp_file 2>&1 > /dev/null

	#
	# Change the context for the r/w file for the bad domain
	#
	chcon -t nofileop_rw_file_t $LTPTMP/temp_file2 2>&1 > /dev/null

	#
	# Change the context for the read-only access file for the bad domain
	#
	chcon -t nofileop_ra_file_t $LTPTMP/temp_file3 2>&1 > /dev/null

	# 
	# Change the context of the test executable
	#
	chcon -t fileop_exec_t $LTPBIN/selinux_wait_io 2>&1 > /dev/null

	#
	# Get the SID of the good file.
	#
	good_file_sid="system_u:object_r:fileop_file_t"

}

test01()
{
        TCID="test01"
        TST_COUNT=1
        RC=0

	#
	# Attempt to access a restricted file as the 'good' domain. 
	# The first test hits basic permissions, while the remaining 
	# tests hit specific hooks.
	#
	runcon -t test_fileop_t -- touch $LTPTMP/temp_file 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #1: file failed."
        else
                echo "Test #1: file passed."
        fi
        return $RC
}

test02() 
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	runcon -t test_fileop_t -- selinux_seek $LTPTMP/temp_file 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #2: file failed."
        else
                echo "Test #2: file passed."
        fi
        return $RC
}

test03()
{
        TCID="test03"
        TST_COUNT=3
        RC=0

	runcon -t test_fileop_t -- selinux_mmap $LTPTMP/temp_file $good_file_sid 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #3: file failed."
        else
                echo "Test #3: file passed."
        fi
        return $RC
}

test04()
{
        TCID="test04"
        TST_COUNT=4
        RC=0

	runcon -t test_fileop_t -- selinux_mprotect $LTPTMP/temp_file $good_file_sid 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #4: file failed."
        else
                echo "Test #4: file passed."
        fi
        return $RC
}

test05()
{
        TCID="test05"
        TST_COUNT=5
        RC=0

	runcon -t test_fileop_t -- selinux_lock $LTPTMP/temp_file $good_file_sid 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #5: file failed."
        else
                echo "Test #5: file passed."
        fi
        return $RC
}

test06()
{
        TCID="test06"
        TST_COUNT=6
        RC=0

	runcon -t test_fileop_t -- selinux_fcntl $LTPTMP/temp_file 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #6: file failed."
        else
                echo "Test #6: file passed."
        fi
        return $RC
}

test07()
{
        TCID="test07"
        TST_COUNT=7
        RC=0

	#
	# Attempt to create a SIGIO as the 'good' domain. 
	#

	SAVEPWD=${PWD}
	cd ${LTPROOT}/testcases/bin
	CURRENTDIR="."

	runcon -t test_fileop_t -- $CURRENTDIR/selinux_sigiotask 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #7: file failed."
        else
                echo "Test #7: file passed."
        fi
	
	cd ${PWD}
        return $RC
}

test08()
{
        TCID="test08"
        TST_COUNT=8
        RC=0

	#
	# Attempt to access the restricted file as the 'bad' domain.
	# The first test hits basic permissions, while the remaining 
	# tests hit specific hooks.
	#
	runcon -t test_nofileop_t -- touch $LTPTMP/temp_file 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #8: file passed."
		return 0
        else
                echo "Test #8: file failed."
		return 1
        fi
}

#
# Perform tests for the bad domain.
#

test09()
{
        TCID="test09"
        TST_COUNT=9
        RC=0

	# This seek test will succeed because the llseek hook only verifies
	# that the process has access to the file descriptor. In order to 
	# test llseek properly, a policy change would need to take effect 
	# between the time that the file was opened and the seek took place. 
	# So, for now, we just test the basic access which should succeed.

	runcon -t test_nofileop_t -- selinux_seek $LTPTMP/temp_file2 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #9: file failed."
        else
                echo "Test #9: file passed."
	fi
	return $RC
}

test10()
{
        TCID="test10"
        TST_COUNT=10
        RC=0

	runcon -t test_nofileop_t -- selinux_mmap $LTPTMP/temp_file2 $good_file_sid 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #10: file passed."
		return 0
        else
                echo "Test #10: file failed."
		return 1
	fi
}
test11()
{
        TCID="test11"
        TST_COUNT=11
        RC=0

	chcon -t nofileop_rw_file_t $LTPTMP/temp_file2 2>&1 > /dev/null
	runcon -t test_nofileop_t -- selinux_mprotect $LTPTMP/temp_file2 $good_file_sid 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #11: file passed."
		return 0
        else
                echo "Test #11: file failed."
		return 1
	fi
}

test12()
{
        TCID="test12"
        TST_COUNT=12
        RC=0

	chcon -t nofileop_rw_file_t $LTPTMP/temp_file2 2>&1 > /dev/null
	runcon -t test_nofileop_t -- selinux_lock $LTPTMP/temp_file2 $good_file_sid 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #12: file passed."
		return 0
        else
                echo "Test #12: file failed."
		return 1
	fi
}

test13()
{
        TCID="test13"
        TST_COUNT=13
        RC=0

	chcon -t nofileop_rw_file_t $LTPTMP/temp_file2 2>&1 > /dev/null

	#
	# Check the fcntl for the bad domain.
	# This uses the read-only accessable file.
	#
	runcon -t test_nofileop_t -- selinux_nofcntl $LTPTMP/temp_file3 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #13: file failed."
        else
                echo "Test #13: file passed."
	fi
	return $RC
}

test14() 
{
        TCID="test14"
        TST_COUNT=14
        RC=0

	#
	# Attempt to create a SIGIO as the 'bad' domain. 
	#
	runcon -t test_nofileop_t -- selinux_sigiotask 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #14: file passed."
		return 0
        else
                echo "Test #14: file failed."
		return 1
	fi
}

cleanup()
{
	#
	# Delete the temp files
	#
	rm -f $basedir/temp_file 2>&1
	rm -f $basedir/temp_file2 2>&1
	rm -f $basedir/temp_file3 2>&1
}

#
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
test06 || exit $RC
test07 || exit $RC
test08 || exit $RC
test09 || exit $RC
test10 || exit $RC
test11 || exit $RC
test12 || exit $RC
test13 || exit $RC
test14 || exit $RC
cleanup
exit 0
