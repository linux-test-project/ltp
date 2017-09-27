#!/bin/sh
#
# Automated tests for utimensat()
#
# Copyright (C) 2008, Linux Foundation
# Written by Michael Kerrisk <mtk.manpages@gmail.com>
# Licensed under GPLv2 or later
#
# Not (yet) included in this automated test set:
# * AT_SYMLINK_NOFOLLOW in flags: If pathname specifies a symbolic link,
#   then update the timestamps of the link, rather than the file to which
#   it refers.
# * Setting of nanosecond components of timestamps (support for
#   nanosecond timestamps is file-system-dependent)
# * "Updated file timestamps are set to the greatest value supported
#   by the file system that is not greater than the specified time."
#   (i.e., if we set timestamp to {0, 999999999}, then the setting
#   is rounded down, rather than up, to unit of timestamp resolution.
# * Privileged processes should be able to bypass permission checks.
#   (except when file is marked with the "Immutable" EFA).

#=====================================================================

export TCID=utimensat01
export TST_TOTAL=99
export TST_COUNT=0
. test.sh

if tst_kvcmp -lt "2.6.22"; then
	tst_brkm TCONF "System kernel version is less than 2.6.22,cannot execute test"
fi

# Starting with 4.8.0 operations on immutable files return EPERM instead of
# EACCES.
# This patch has also been merged to stable 4.4 with
# b3b4283 ("vfs: move permission checking into notify_change() for utimes(NULL)")
if tst_kvcmp -ge "4.4.27" -a -lt "4.5.0"; then
	imaccess=EPERM
elif tst_kvcmp -lt "4.4.27"; then
	imaccess=EACCES
else
	imaccess=EPERM
fi


RESULT_FILE=$TMPDIR/utimensat.result

TEST_DIR=$TMPDIR/utimensat_tests
FILE=$TEST_DIR/utimensat.test_file

TEST_PROG=utimensat01

if [ ! -f $LTPROOT/testcases/bin/$TEST_PROG ]; then
	tst_brkm TBROK "$LTPROOT/testcases/bin/$TEST_PROG is missing (please check install)"
fi

# Summary counters of all test results

test_num=0
failed_cnt=0
passed_cnt=0
failed_list=""

#=====================================================================

setup_file()
{
# $1 is test file pathname
# $2 is owner for test file (chown(1))
# $3 is permissions for test file (chmod(1))
# $4 is "ext2" extended file attributes for test file (chattr(1))

    FILE=$1

    # Make sure any old version of file is deleted

    if test -e $FILE; then
        sudo $s_arg chattr -ai $FILE || return $?
        sudo $s_arg rm -f $FILE || return $?
    fi

    # Create file and make atime and mtime zero.

    sudo $s_arg -u $test_user touch $FILE || return $?
    if ! $TEST_PROG -q $FILE 0 0 0 0 > $RESULT_FILE; then
        echo "Failed to set up test file $FILE" 1>&2
        exit 1
    fi

    read res atime mtime < $RESULT_FILE
    if test "X$res" != "XSUCCESS" ||
                test $atime -ne 0 || test $mtime != 0; then
        echo "Failed to set correct times on test file $FILE" 1>&2
        exit 1
    fi

    # Set owner, permissions, and EFAs for file.

    if test -n "$2"; then
        sudo $s_arg chown $2 $FILE || return $?
    fi

    sudo $s_arg chmod $3 $FILE || return $?

    if test -n "$4"; then
        sudo $s_arg chattr $4 $FILE || return $?
    fi

    # Display file setup, for visual verification

    ls -l $FILE | awk '{ printf "Owner=%s; perms=%s; ", $3, $1}'
    if ! sudo $s_arg lsattr -l $FILE | sed 's/, /,/g' | awk '{print "EFAs=" $2}'
    then
        return $?
    fi

}

test_failed()
{
    tst_resm TFAIL "FAILED test $test_num"

    failed_cnt=$(expr $failed_cnt + 1)
    failed_list="$failed_list $test_num"
}

check_result()
{
    STATUS=$1                   # Exit status from test program
    EXPECTED_RESULT=$2          # SUCCESS / EACCES / EPERM / EINVAL
    EXPECT_ATIME_CHANGED=$3     # Should be 'y' or 'n' (only for SUCCESS)
    EXPECT_MTIME_CHANGED=$4     # Should be 'y' or 'n' (only for SUCCESS)

    test_num=$(expr $test_num + 1)

    # If our test setup failed, stop immediately

    if test $STATUS -gt 1; then
        echo "FAILED (bad test setup)"
        exit 1
    fi

    read res atime mtime < $RESULT_FILE

    echo "EXPECTED: $EXPECTED_RESULT $EXPECT_ATIME_CHANGED "\
         "$EXPECT_MTIME_CHANGED"
    echo "RESULT:   $res $atime $mtime"

    if test "$res" != "$EXPECTED_RESULT"; then
        test_failed
        return
    fi

    passed=1

    # If the test program exited successfully, then check that atime and
    # and mtime were updated / not updated, as expected.

    if test $EXPECTED_RESULT = "SUCCESS"; then
        if test $EXPECT_ATIME_CHANGED = "y"; then
            if test $atime -eq 0; then
                echo "atime should have changed, but did not"
                passed=0
            fi
        else
            if test $atime -ne 0; then
                echo "atime should not have changed, but did"
                passed=0
            fi
        fi

        if test $EXPECT_MTIME_CHANGED = "y"; then
            if test $mtime -eq 0; then
                echo "mtime should have changed, but did not"
                passed=0
            fi
        else
            if test $mtime -ne 0; then
                echo "mtime should not have changed, but did"
                passed=0
            fi
        fi

        if test $passed -eq 0; then
            test_failed
            return
        fi
    fi

    passed_cnt=$(expr $passed_cnt + 1)
    tst_resm TPASS "PASSED test $test_num"
}

run_test()
{
    # By default, we do three types of test:
    # a) pathname (pathname != NULL)
    # b) readable file descriptor (pathname == NULL, dirfd opened O_RDONLY)
    # c) writable file descriptor (pathname == NULL, dirfd opened O_RDWR).
    #    For this case we also include O_APPEND in open flags, since that
    #    is needed if testing with a file that has the Append-only
    #    attribute enabled.

    # -R says don't do tests with readable file descriptor
    # -W says don't do tests with writable file descriptor

    OPTIND=1

    do_read_fd_test=1
    do_write_fd_test=1
    while getopts "RW" opt; do
        case "$opt" in
        R) do_read_fd_test=0
           ;;
        W) do_write_fd_test=0
           ;;
        *) echo "run_test: bad usage"
           exit 1
           ;;
        esac
    done
    shift `expr $OPTIND - 1`

    echo "Pathname test"
    setup_file $FILE "$1" "$2" "$3"
    cp $LTPROOT/testcases/bin/$TEST_PROG ./
    CMD="./$TEST_PROG -q $FILE $4"
    echo "$CMD"
    sudo $s_arg -u $test_user $CMD > $RESULT_FILE
    check_result $? $5 $6 $7
    echo

    if test $do_read_fd_test -ne 0; then
        echo "Readable file descriptor (futimens(3)) test"
        setup_file $FILE "$1" "$2" "$3"
        CMD="./$TEST_PROG -q -d $FILE NULL $4"
        echo "$CMD"
        sudo $s_arg -u $test_user $CMD > $RESULT_FILE
        check_result $? $5 $6 $7
        echo
    fi

    # Can't do the writable file descriptor test for immutable files
    # (even root can't open an immutable file for writing)

    if test $do_write_fd_test -ne 0; then
        echo "Writable file descriptor (futimens(3)) test"
        setup_file $FILE "$1" "$2" "$3"
        CMD="./$TEST_PROG -q -w -d $FILE NULL $4"
        echo "$CMD"
        sudo $s_arg -u $test_user $CMD > $RESULT_FILE
        check_result $? $5 $6 $7
        echo
    fi

    sudo $s_arg chattr -ai $FILE
    sudo $s_arg rm -f $FILE
}

#=====================================================================

# Since some automated testing systems have no tty while testing,
# comment this line in /etc/sudoers to avoid the error message:
# `sudo: sorry, you must have a tty to run sudo'
# Use trap to restore this line after program terminates.
sudoers=/etc/sudoers
if [ ! -r $sudoers ]; then
	tst_brkm TBROK "can't read $sudoers"
fi
pattern="[[:space:]]*Defaults[[:space:]]*requiretty.*"
if grep -q "^${pattern}" $sudoers; then
	tst_resm TINFO "Comment requiretty in $sudoers for automated testing systems"
	if ! sed -r -i.$$ -e "s/^($pattern)/#\1/" $sudoers; then
		tst_brkm TBROK "failed to mangle $sudoers properly"
	fi
	trap 'trap "" EXIT; restore_sudoers' EXIT
fi

restore_sudoers()
{
	tst_resm TINFO "Restore requiretty in $sudoers"
	mv /etc/sudoers.$$ /etc/sudoers
}

test_user=nobody
echo "test sudo for -n option, non-interactive"
if sudo -h | grep -q -- -n; then
	s_arg="-n"
	echo "sudo supports -n"
else
	s_arg=
	echo "sudo does not support -n"
fi

if ! sudo $s_arg true; then
	tst_brkm TBROK "sudo cannot be run by user non-interactively"
fi
if test ! -f $sudoers
then
	echo "root    ALL=(ALL)    ALL" > $sudoers || exit
	chmod 440 $sudoers
	trap 'trap "" EXIT; nuke_sudoers' EXIT
fi

nuke_sudoers()
{
	sudo rm -f $sudoers
}

sudo $s_arg -u $test_user mkdir -p $TEST_DIR

# Make sure chattr command is supported
touch $TEST_DIR/tmp_file
chattr +a $TEST_DIR/tmp_file
if [ $? -ne 0 ] ; then
	rm -rf $TEST_DIR
	tst_brkm TCONF "chattr not supported"
fi
chattr -a $TEST_DIR/tmp_file

cd $TEST_DIR
chown root $LTPROOT/testcases/bin/$TEST_PROG
chmod ugo+x,u+s $LTPROOT/testcases/bin/$TEST_PROG

#=====================================================================


echo "============================================================"

echo
echo "Testing read-only file, owned by self"
echo

echo "***** Testing times==NULL case *****"
run_test -W "" 400 "" "" SUCCESS y y

echo "***** Testing times=={ UTIME_NOW, UTIME_NOW } case *****"
run_test -W "" 400 "" "0 n 0 n" SUCCESS y y

echo "***** Testing times=={ UTIME_OMIT, UTIME_OMIT } case *****"
run_test -W "" 400 "" "0 o 0 o" SUCCESS n n

echo "***** Testing times=={ UTIME_NOW, UTIME_OMIT } case *****"
run_test -W "" 400 "" "0 n 0 o" SUCCESS y n

echo "***** Testing times=={ UTIME_OMIT, UTIME_NOW } case *****"
run_test -W "" 400 "" "0 o 0 n" SUCCESS n y

echo "***** Testing times=={ x, y } case *****"
run_test -W "" 400 "" "1 1 1 1" SUCCESS y y

echo "============================================================"

echo
echo "Testing read-only file, not owned by self"
echo

echo "***** Testing times==NULL case *****"
run_test -RW root 400 "" "" EACCES

echo "***** Testing times=={ UTIME_NOW, UTIME_NOW } case *****"
run_test -RW root 400 "" "0 n 0 n" EACCES

echo "***** Testing times=={ UTIME_OMIT, UTIME_OMIT } case *****"
run_test -RW root 400 "" "0 o 0 o" SUCCESS n n

echo "***** Testing times=={ UTIME_NOW, UTIME_OMIT } case *****"
run_test -RW root 400 "" "0 n 0 o" EPERM

echo "***** Testing times=={ UTIME_OMIT, UTIME_NOW } case *****"
run_test -RW root 400 "" "0 o 0 n" EPERM

echo "***** Testing times=={ x, y } case *****"
run_test -RW root 400 "" "1 1 1 1" EPERM

echo "============================================================"

echo
echo "Testing writable file, not owned by self"
echo

echo "***** Testing times==NULL case *****"
run_test root 666 "" "" SUCCESS y y

echo "***** Testing times=={ UTIME_NOW, UTIME_NOW } case *****"
run_test root 666 "" "0 n 0 n" SUCCESS y y

echo "***** Testing times=={ UTIME_OMIT, UTIME_OMIT } case *****"
run_test root 666 "" "0 o 0 o" SUCCESS n n

echo "***** Testing times=={ UTIME_NOW, UTIME_OMIT } case *****"
run_test root 666 "" "0 n 0 o" EPERM

echo "***** Testing times=={ UTIME_OMIT, UTIME_NOW } case *****"
run_test root 666 "" "0 o 0 n" EPERM

echo "***** Testing times=={ x, y } case *****"
run_test root 666 "" "1 1 1 1" EPERM

echo "============================================================"

echo
echo "Testing append-only file, owned by self"
echo

echo "***** Testing times==NULL case *****"
run_test "" 600 "+a" "" SUCCESS y y

echo "***** Testing times=={ UTIME_NOW, UTIME_NOW } case *****"
run_test "" 600 "+a" "0 n 0 n" SUCCESS y y

echo "***** Testing times=={ UTIME_OMIT, UTIME_OMIT } case *****"
run_test "" 600 "+a" "0 o 0 o" SUCCESS n n

echo "***** Testing times=={ UTIME_NOW, UTIME_OMIT } case *****"
run_test "" 600 "+a" "0 n 0 o" EPERM

echo "***** Testing times=={ UTIME_OMIT, UTIME_NOW } case *****"
run_test "" 600 "+a" "0 o 0 n" EPERM

echo "***** Testing times=={ x, y } case *****"
run_test "" 600 "+a" "1 1 1 1" EPERM

echo "============================================================"

echo
echo "Testing immutable file, owned by self"
echo

echo "***** Testing times==NULL case *****"
run_test -W "" 600 "+i" "" $imaccess

echo "***** Testing times=={ UTIME_NOW, UTIME_NOW } case *****"
run_test -W "" 600 "+i" "0 n 0 n" $imaccess

echo "***** Testing times=={ UTIME_OMIT, UTIME_OMIT } case *****"
run_test -W "" 600 "+i" "0 o 0 o" SUCCESS n n

echo "***** Testing times=={ UTIME_NOW, UTIME_OMIT } case *****"
run_test -W "" 600 "+i" "0 n 0 o" EPERM

echo "***** Testing times=={ UTIME_OMIT, UTIME_NOW } case *****"
run_test -W "" 600 "+i" "0 o 0 n" EPERM

echo "***** Testing times=={ x, y } case *****"
run_test -W "" 600 "+i" "1 1 1 1" EPERM

echo "============================================================"

# Immutable+append-only should have same results as immutable

echo
echo "Testing immutable append-only file, owned by self"
echo

echo "***** Testing times==NULL case *****"
run_test -W "" 600 "+ai" "" $imaccess

echo "***** Testing times=={ UTIME_NOW, UTIME_NOW } case *****"
run_test -W "" 600 "+ai" "0 n 0 n" $imaccess

echo "***** Testing times=={ UTIME_OMIT, UTIME_OMIT } case *****"
run_test -W "" 600 "+ai" "0 o 0 o" SUCCESS n n

echo "***** Testing times=={ UTIME_NOW, UTIME_OMIT } case *****"
run_test -W "" 600 "+ai" "0 n 0 o" EPERM

echo "***** Testing times=={ UTIME_OMIT, UTIME_NOW } case *****"
run_test -W "" 600 "+ai" "0 o 0 n" EPERM

echo "***** Testing times=={ x, y } case *****"
run_test -W "" 600 "+ai" "1 1 1 1" EPERM

echo "============================================================"

echo

# EINVAL should result, if pathname is NULL, dirfd is not
# AT_FDCWD, and flags contains AT_SYMLINK_NOFOLLOW.

echo "***** Testing pathname==NULL, dirfd!=AT_FDCWD, flags has" \
     "AT_SYMLINK_NOFOLLOW *****"
setup_file $FILE "" 600 ""
CMD="$TEST_PROG -q -n -d $FILE NULL $4"
echo "$CMD"
$CMD > $RESULT_FILE
check_result $? EINVAL
echo

echo "============================================================"

echo

# If UTIME_NOW / UTIME_OMIT in tv_nsec field, the tv_sec should
# be ignored.

echo "tv_sec should be ignored if tv_nsec is UTIME_OMIT or UTIME_NOW"

echo "***** Testing times=={ UTIME_NOW, UTIME_NOW } case *****"
run_test -RW "" 600 "" "1 n 1 n" SUCCESS y y

echo "***** Testing times=={ UTIME_OMIT, UTIME_OMIT } case *****"
run_test -RW "" 600 "" "1 o 1 o" SUCCESS n n

echo "============================================================"

echo

rm -rf "$TEST_DIR"
uname -a
date
echo "Total tests: $test_num; passed: $passed_cnt; failed: $failed_cnt"
if test $failed_cnt -gt 0; then
    echo "Failed tests: $failed_list"
fi

tst_exit
