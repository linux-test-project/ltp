#!/bin/bash
#
#   Copyright (c) International Business Machines  Corp., 2008
#   
#   This program is free software;  you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY;  without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#   the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program;  if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
#*******************************************************************************
# Readme_ROBind has more details on the tests running for ROBIND.
# TEST: 
#   NAME:       test_robind.sh
#   FUNCTIONALITY:  File system tests for normal mount, bind mount and RO mount
#
#   DESCRIPTION:    Performs filesystems tests for RO mount.
#     For filesystem's like ext2, ext3, reiserfs, jfs & xfs.
#     This test creates an image-file and
#        a)  mounts on dir1, 
#        b)  mount --bind dir2
#        c)  mount -o remount,ro 
#       It verifies the tests on a) and b) works correctly.
#     For the c) option it checks that the tests are not able to write into dir.
#     Then it executes the tests from flat-file  {LTPROOT}/testscripts/fs_ro_tests
#     Check the logs /tmp/fs$$/errs.log and /tmp/fs$$/pass.log for pass/failures.
#===============================================================================
#
# CHANGE HISTORY:
# DATE           AUTHOR                  REASON
# 09/06/2008     Veerendra Chandrappa    For Container, testing of RO-Bind mount
#                Dave Hansen
# This script is based on the Dave Hansen script for testing the robind.
#*******************************************************************************

#trace_logic=${trace_logic:-"set -x"}
$trace_logic

# The test case ID, the test case count and the total number of test case
TCID=${TCID:-test_robind.sh}
TST_TOTAL=1
TST_COUNT=1
export TCID
export TST_COUNT
export TST_TOTAL

usage()
{   
  cat << EOF
  usage: $0 [ext3,ext2,jfs,xfs,reiserfs,ramfs]

  This script verifies ReadOnly-filesystem, by mounting imagefile and 
  executing the filesystem tests.

  OPTIONS
    -h    display this message and exit
EOF
}

DIRS="dir1 dir2-bound dir3-ro"
TMPDIR=/tmp/fs$$
trap cleanup ERR
trap cleanup INT

#==============================================================================
# FUNCTION NAME:    cleanup
#
# FUNCTION DESCRIPTION: Unmounts dir, Removes dir's, files created by the tests.
#
# PARAMETERS:       The $fs_image .
#
# RETURNS:      None.
#==============================================================================
function cleanup
{
    umount ${TMPDIR}/dir3-ro 2> /dev/null > /dev/null
    umount ${TMPDIR}/dir2-bound 2> /dev/null 1> /dev/null 
    umount ${TMPDIR}/dir1 2> /dev/null 1> /dev/null
    if [ ! -z $1 ]; then {
        rm -rf $1 || true
    }
    fi
}

#===============================================================================
# FUNCTION NAME:    setup
#
# FUNCTION DESCRIPTION: Does the initailization
#
# PARAMETERS:   File_systems (if any )    
#
# RETURNS:      None.
#===============================================================================
function setup
{
    mkdir ${TMPDIR}
    FAILLOG="$TMPDIR/errs.log"
    PASSLOG="$TMPDIR/pass.log"

    for i in $DIRS; do
        rm -rf ${TMPDIR}/$i || true
        mkdir -p ${TMPDIR}/$i
    done;

    # Populating the default FS as ext3, if FS is not given
    if [ -z "$*" ]; then
        FSTYPES="ext3"
    else
        FSTYPES="$*"
    fi

    # set the LTPROOT directory
    cd `dirname $0`
    echo "${PWD}" | grep testscripts > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        cd ..
        export LTPROOT="${PWD}"
        export PATH="${PATH}:${LTPROOT}/testcases/bin"
    fi

    FS_Tests="${LTPROOT}/testscripts/fs_ro_tests"
    cd ${TMPDIR}
}

#=============================================================================
# FUNCTION NAME:    testdir
#
# FUNCTION DESCRIPTION: The core function where it runs the tests
#
# PARAMETERS:   dir_name, file_systems, Read_only flag = [true|false]
#
# RETURNS:      None.
#=============================================================================
function testdir
{
    dir=$1
    fs=$2
    RO=$3
    pushd $dir
    testnums=`wc -l $FS_Tests | cut -f1 -d" "`
    status=0

    echo "---------------------------------------------------" >> $FAILLOG ;
    echo "Running RO-FileSystem Tests for $dir $fs filesystem" >> $FAILLOG ;
    echo "---------------------------------------------------" >> $FAILLOG ;

    echo "---------------------------------------------------" >> $PASSLOG ;
    echo "Running RO-FileSystem Tests for $dir $fs filesystem" >> $PASSLOG ;
    echo "---------------------------------------------------" >> $PASSLOG ;

    export TDIRECTORY=$PWD ;
    echo TDIR is $TDIRECTORY;
    if [ $RO == false ] ; then                          # Testing Read-Write dir
        for tests in `seq $testnums` ; do
            cmd=`cat $FS_Tests | head -$tests | tail -n 1`
#            eval $cmd 2>&1 /dev/null
            eval $cmd 2> /dev/null 1> /dev/null 
            if [ $? -eq 0 ]; then
                echo "$tests. '$cmd' PASS" >> $PASSLOG 
            else
                echo "$tests. '$cmd' FAIL " >> $FAILLOG 
                echo "TDIR is $TDIRECTORY" >> $FAILLOG;
                status=1 
            fi
        done

    else                                                # Testing Read-Only dir
        for tests in `seq $testnums` ; do
            cmd=`cat $FS_Tests | head -$tests | tail -n 1`
            eval $cmd 2> /dev/null 1> /dev/null 
            if [ $? -ne 0 ]; then
                echo "$tests. '$cmd' PASS " >> $PASSLOG 
            else
                 echo "$tests. '$cmd' FAIL" >> $FAILLOG 
                 status=1 
            fi
        done
    fi
    if [ $status == 1 ] ; then
        echo "RO-FileSystem Tests FAILED for $dir $fs filesystem" >> $FAILLOG
        echo >> $FAILLOG
        retcode=$status
    else
        echo "RO-FileSystem Tests PASSed for $dir $fs filesystem" >> $PASSLOG
        echo >> $PASSLOG
    fi
    # Remove all the temp-files created.
    eval rm -rf ${TMPDIR}/${dir}/* > /dev/null 2>&1 /dev/null || true
    unset TDIRECTORY
    popd
}

#=============================================================================
# MAIN 
#     See the description, purpose, and design of this test under TEST 
#     in this test's prolog.
#=============================================================================
retcode=0
while getopts h: OPTION; do
  case $OPTION in
    h)
      usage
      exit 1
      ;;
    ?)
      usage
      exit 1
      ;;
  esac
done
# Does the initial setups
oldpwd=${PWD}
setup $*

# Executes the tests for differnt FS's
# Creates an image file of 500 MB and mounts it.
for fstype in $FSTYPES; do
    image=$fstype.img
    dd if=/dev/zero of=$image bs=$((1<<20)) count=500 2> /dev/null 1> /dev/null
    if [ $? -ne 0 ] ; then
        tst_resm, TFAIL "Unable to create image "
        tst_resm, TFAIL "Free Disk space of 512MB is required in /tmp fs"
        tst_resm, TFAIL "Please free it and rerun thank you.."
        rm -f $image
        exit -1
    fi
    
    OPTS="-F"
    if [ "$fstype" == "reiserfs" ]; then
    OPTS="-f --journal-size 513 -q"
    elif [ "$fstype" == "jfs" ]; then
    OPTS="-f"
    elif [ "$fstype" == "xfs" ]; then
    OPTS=""
    fi

    if [ "$fstype" != "ramfs" ] ; then
        mkfs.$fstype $OPTS $image 2> /dev/null 1> /dev/null
    fi

    mount -t $fstype -o loop $image dir1
    mount --bind dir1 dir2-bound || exit -1
    mount --bind dir1 dir3-ro    || exit -1
    mount -o remount,ro dir3-ro  || exit -1

    testdir dir1 $fstype false
    testdir dir2-bound $fstype false
    testdir dir3-ro $fstype true
    cleanup $image
done

    for i in $DIRS; do
        rm -rf ./$i || true
    done;
    cd $oldpwd || true
    exit $retcode 

