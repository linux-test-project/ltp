#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2001                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software	              ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File:        tar_test.sh
# 
# Description: Tests tar command. These tests test the basic functioanlity of
#              tape archive command. 
#
# Author:      Manoj Iyer, manjo@mail.utexas.edu
#
# History:     Dec 17 2002 - Created - Manoj Iyer.
#              Dec 18 2002 - Added code to read the LTPROOT and TMPBASE
#              variables to set LTPBIN and LTPTMP variables
#

export TST_TOTAL=1

if [ -z "$LTPTMP" -a -z "$TMPBASE" ]; then
    LTPTMP=/tmp
else
    LTPTMP=$TMPBASE
fi

if [ -z "$LTPBIN" -a -z "$LTPROOT" ]; then
    LTPBIN=./bin
else
    LTPBIN=$LTPROOT/testcases/bin
fi

# set return code RC variable to 0, it will be set with a non-zero return code
# in case of error. Set TFAILCNT to 0, increment if there occures a failure.
#

TFAILCNT=0     
RC=0                         
RC1=0
RC2=0
RC3=0

# Test #1
# Test if tar command can create a tar file 'tar cvf <tar filename> <list of
# files>'

export TCID=tar01
export TST_COUNT=1

$LTPBIN/tst_resm TINFO \
    "TEST #1: tar command with cvf options creates an archive file"

touch $LTPTMP/tar_tstf1 $LTPTMP/tar_tstf2 $LTPTMP/tar_tstf3

tar cvf $LTPTMP/tar_tstf.tar $LTPTMP/tar_tstf1 \
    $LTPTMP/tar_tstf2 $LTPTMP/tar_tstf3 > $LTPTMP/tar_tst.out 2>&1 || RC=$?

if [ $RC -eq 0 ]; then
    if [ -f $LTPTMP/tar_tstf.tar ]; then
	$LTPBIN/tst_resm TPASS "tar: cvf option created a tar file."
    else
	$LTPBIN/tst_res TFAIL $LTPTMP/tar_tst.out \
	    "tar: cvf option failed to create archive.  Reason"
	TFAILCNT=$(( $TFAILCNT+1 ))
    fi
else
    $LTPBIN/tst_brk TBROK $LTPTMP/file.out NULL \
	"tar: command failed. Reason:"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi


# Test #2
# Test if tar command with tvf option will list all the files in the archive.

export TCID=tar02
export TST_COUNT=2

$LTPBIN/tst_resm TINFO \
    "TEST #2: tar command with tvf options lists all files in an archive file"

if [ -f $LTPTMP/tar_tstf.tar ]; then
    echo "$LTPTMP/tar_tstf.tar exists" > /dev/null 2>&1
else
    touch $LTPTMP/tar_tstf1 $LTPTMP/tar_tstf2 $LTPTMP/tar_tstf3
    tar cvf $LTPTMP/tar_tstf.tar $LTPTMP/tar_tstf1 \
	$LTPTMP/tar_tstf2 $LTPTMP/tar_tstf3 > $LTPTMP/tar_tst.out 2>&1 || RC=$?
    
    if [ $RC -eq 0 ]; then
	if [ -f $LTPTMP/tar_tstf.tar ]; then
	    echo "tar file created" > /dev/null 2>&1
	else
	    $LTPBIN/tst_brk TBROK $LTPTMP/tar_tst.out NULL \
		"tar: cvf option failed to create archive.  Reason"
	    TFAILCNT=$(( $TFAILCNT+1 ))
	fi
    else
	$LTPBIN/tst_brk TBROK $LTPTMP/file.out NULL \
	    "tar: command failed. Reason:"
	TFAILCNT=$(( $TFAILCNT+1 ))
    fi
fi

tar -tvf $LTPTMP/tar_tstf.tar > /$LTPTMP/tar_tst.out 2>&1 || RC=$?

if [ $RC -eq 0 ]; then
    grep  "tar_tstf1" $LTPTMP/tar_tst.out > $LTPTMP/tar_tst2.out 2>&1 || RC1=$?
    grep  "tar_tstf2" $LTPTMP/tar_tst.out 2>&1 1>>$LTPTMP/tar_tst2.out || RC2=$?
    grep  "tar_tstf3" $LTPTMP/tar_tst.out 2>&1 1>>$LTPTMP/tar_tst2.out || RC3=$?
    
    if [ $RC1 -eq 0 -a $RC2 -eq 0 -a $RC3 -eq 0 ]; then
	$LTPBIN/tst_resm TPASS 	"tar: tvf option listed all its contents"
    else
	$LTPBIN/tst_res TFAIL $LTPTMP/tar_tst.out \
	    "tar: failed to list all the files in the archive. Reason:"
	TFAILCNT=$(( $TFAILCNT+1 ))
    fi
fi


# Test #3
# Test if tar command can create a compressed tar file 'tar cvf <tar filename> 
# <list of files>'

export TCID=tar03
export TST_COUNT=3

$LTPBIN/tst_resm TINFO \
    "TEST #3: tar command with zcvf options creates an compressed archive file"

touch $LTPTMP/tar_tstf1 $LTPTMP/tar_tstf2 $LTPTMP/tar_tstf3

tar zcvf $LTPTMP/tar_tstf.tgz $LTPTMP/tar_tstf1 \
    $LTPTMP/tar_tstf2 $LTPTMP/tar_tstf3 > $LTPTMP/tar_tst.out 2>&1 || RC=$?

if [ $RC -eq 0 ]; then
    if [ -f $LTPTMP/tar_tstf.tgz ]; then
	file $LTPTMP/tar_tstf.tgz | grep "gzip compressed data" \
            > $LTPTMP/tar_tst.out 2>&1 || RC=$?
	if [ $RC -eq 0 ]; then
	    $LTPBIN/tst_resm TPASS \
		"tar: zcvf option created a compressed tar file."
	else
	    $LTPBIN/tst_res TFAIL $LTPTMP/tar_tst.out \
		"tar: zcvf option failed to create a compressed tar file. Reason:"
	    TFAILCNT=$(( $TFAILCNT+1 ))
	fi
    else
	$LTPBIN/tst_brk TBROK $LTPTMP/tar_tst.out NULL \
	    "tar: cvf option failed to create compressed archive.  Reason"
	TFAILCNT=$(( $TFAILCNT+1 ))
    fi
else
    $LTPBIN/tst_brk TBROK $LTPTMP/file.out NULL \
	"tar: command failed while creating compressed archive. Reason:"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi


# Test #4
# Test if tar command with xvf can untar an archive file created by tar.

export TCID=tar04
export TST_COUNT=4

$LTPBIN/tst_resm TINFO \
    "TEST #4: tar command with xvf options extracts files from an archive file"

if [ -f $LTPTMP/tar_tstf.tar ]; then
    echo "tar file exists" > /dev/null 2>&1
else
    touch $LTPTMP/tar_tstf1 $LTPTMP/tar_tstf2 $LTPTMP/tar_tstf3
    
    tar cvf $LTPTMP/tar_tstf.tar $LTPTMP/tar_tstf1 \
	$LTPTMP/tar_tstf2 $LTPTMP/tar_tstf3 > $LTPTMP/tar_tst.out 2>&1 || RC=$?
    
    if [ $RC -eq 0 ]; then
	if [ -f $LTPTMP/tar_tstf.tar ];	then
	    $LTPBIN/tst_resm TINFO "tar: cvf option created a tar file."
	else
	    $LTPBIN/tst_brk TBROK $LTPTMP/tar_tst.out NULL \
		"tar: cvf option failed to create archive.  Reason"
	    TFAILCNT=$(( $TFAILCNT+1 ))
	fi
    else
	$LTPBIN/tst_brk TBROK $LTPTMP/file.out NULL \
	    "tar: command failed. Reason:"
	TFAILCNT=$(( $TFAILCNT+1 ))
    fi
fi

tar xvf $LTPTMP/tar_tstf.tar > $LTPTMP/tar_tst.out 2>&1 || RC=$?

if [ $? -eq 0 ]; then
   if [ -d $LTPTMP -a -f $LTPTMP/tar_tstf1 -a -f $LTPTMP/tar_tstf2 -a -f $LTPTMP/tar_tstf3 ]; then
       $LTPBIN/tst_resm TPASS "tar: xvf option extracted the archive file."
   else
       $LTPBIN/tst_res TFAIL $LTPTMP/tar_tst.out \
	   "tar: xvf option failed to extract. Reason:"
       TFAILCNT=$(( $TFAILCNT+1 ))
   fi
else
    $LTPBIN/tst_brk TBROK $LTPTMP/tar_tst.out NULL \
	"tar: command failed while extracting files. Reason"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi

rm -f $LTPTMP/tar_tst*

# Test #5
# Test if tar command can extract a compressed tar file 'tar zxvf 
# <tar filename> <list of files>'

export TCID=tar05
export TST_COUNT=5

$LTPBIN/tst_resm TINFO \
	"TEST #5: tar command with zxvf options extracts a compressed archive file"

if [ -f tar_tstf.tgz ]; then
    echo "compressed archive file already exists" > /dev/null 2>&1
else

    touch $LTPTMP/tar_tstf1 $LTPTMP/tar_tstf2 $LTPTMP/tar_tstf3
    tar zcvf $LTPTMP/tar_tstf.tgz $LTPTMP/tar_tstf1 \
	$LTPTMP/tar_tstf2 $LTPTMP/tar_tstf3 > $LTPTMP/tar_tst.out 2>&1 || RC=$?
    
	if [ $RC -eq 0 ]; then
	    if [ -f $LTPTMP/tar_tstf.tgz ]; then
		file $LTPTMP/tar_tstf.tgz | grep "gzip compressed data" \
		    > $LTPTMP/tar_tst.out 2>&1 || RC=$?
		if [ $RC -eq 0 ]; then
		    $LTPBIN/tst_resm TINFO  \
			"tar: zcvf option created a compressed tar file."
		else
		    $LTPBIN/tst_brk TBROK $LTPTMP/tar_tst.out NULL \
			"tar: zcvf option failed to create a compressed tar file. Reason:"
		    TFAILCNT=$(( $TFAILCNT+1 ))
		fi
	    else
		$LTPBIN/tst_brk TBROK $LTPTMP/tar_tst.out NULL \
		    "tar: cvf option failed to create compressed archive.  Reason"
		TFAILCNT=$(( $TFAILCNT+1 ))
	    fi
	else
	    $LTPBIN/tst_brk TBROK $LTPTMP/tar_tst.out NULL \
		"tar: command failed while creating compressed archive. Reason:"
	    TFAILCNT=$(( $TFAILCNT+1 ))
	fi
fi

tar zxvf $LTPTMP/tar_tstf.tgz > $LTPTMP/tar_tst.out 2>&1 || RC=$?

if [ $? -eq 0 ]; then
    if [ -d $LTPTMP -a -f $LTPTMP/tar_tstf1 -a -f $LTPTMP/tar_tstf2 -a -f $LTPTMP/tar_tstf3 ]; then
	$LTPBIN/tst_resm TPASS \
	    "tar: zxvf option extracted the compressed archive file."
    else
	$LTPBIN/tst_res TFAIL $LTPTMP/tar_tst.out \
	    "tar: zxvf option failed to extract compressed archive. Reason:"
	TFAILCNT=$(( $TFAILCNT+1 ))
    fi
else
    $LTPBIN/tst_brk TBROK $LTPTMP/tar_tst.out NULL \
	"tar: command failed while extracting compressed archive files. Reason"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi

rm -f $LTPTMP/tar_tst*

exit $TFAILCNT
