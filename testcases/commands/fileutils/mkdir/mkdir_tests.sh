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
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :        mkdir_test.sh
#
# Description:  Test basic functionality of mkdir command
#				- Test #1:  mkdir -p can make parent directories as needed
#               
# Author:       Manoj Iyer, manjo@mail.utexas.edu
#
# History:      Feb 03 2003 - Created - Manoj Iyer.
#
# Function:		init
#
# Description:	- Check if command mkdir is available.
#               - Create temprary directory, and temporary files.
#               - Initialize environment variables.
#
# Return		- zero on success
#               - non zero on failure. return value from commands ($RC)
init()
{

	RC=0				# Return code from commands.
	export TST_TOTAL=1	# total numner of tests in this file.
	export TCID=mkdir	# this is the init function.
	export TST_COUNT=0	# init identifier,

	if [ -z "$LTPTMP" ] && [ -z "$TMPBASE" ]
	then
		LTPTMP=/tmp
	else
		LTPTMP=$TMPBASE
	fi
	if [ -z "$LTPBIN" ] && [ -z "$LTPROOT" ]
	then
		LTPBIN=./
	else
		LTPBIN=$LTPROOT/testcases/bin
	fi

	
	$LTPBIN/tst_resm TINFO "INIT: Inititalizing tests."

	which mkdir > $LTPTMP/tst_mkdir.err 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		$LTPBIN/tst_brk TBROK $LTPTMP/tst_mkdir.err NULL \
			"Test #1: mkdir command does not exist. Reason:"
		return $RC
	fi

	mkdir -p $LTPTMP/tst_mkdir.tmp > $LTPTMP/tst_mkdir.err 2>&1 || RC=$? 
	if [ $RC -ne 0 ]
	then
		$LTPBIN/tst_brk TBROK $LTPTMP/tst_mkdir.err NULL \
			"Test #1: failed creating temp directory. Reason:"
		return $RC
	fi
	return $RC
}


# Function:		creat_expout
#
# Description:	- create expected output
#
# Input:		$1 - number of directories to create
#				$2 - number of file to create in each directory
#				$3 - name of the base directory
#
# Return		- zero on success
#               - non zero on failure. return value ($RC) from commands
creat_expout()
{
	numdir=$1	# number of directories to create
	numfile=$2  # number of file to create in each directory
	dirname=$3  # name of the base directory
    dircnt=0    # index into dir created in loop
    fcnt=0      # index into files created in loop
	RC=0        # return code from commands 
	
	echo "$dirname:"  1>>$LTPTMP/tst_mkdir.exp
	echo "d.$dircnt"  1>>$LTPTMP/tst_mkdir.exp
	while [ $dircnt -lt $numdirs ]
	do
		dirname=$dirname/d.$dircnt
		dircnt=$(($dircnt+1))
		echo "$dirname:"  1>>$LTPTMP/tst_mkdir.exp
		if [ $dircnt -lt $numdirs ]
		then
			echo "d.$dircnt"  1>>$LTPTMP/tst_mkdir.exp
		fi
		fcnt=0
        while [ $fcnt -lt $numfiles ]
        do
			echo "f.$fcnt " 1>>$LTPTMP/tst_mkdir.exp
			fcnt=$(($fcnt+1))
		done
		printf "\n\n" 1>>$LTPTMP/tst_mkdir.exp
	done
}

# Function:		test01
#
# Description	- Test #1: Test that mkdir -p creates parent directories as
#                 needed
#               - create N directories and fill each with M files.
#               - mkdir -p dir 
#               - list contents of dir and save it to file - actual output
#               - create expected output
#               - compare expected output with actual output.
#
# Return		- zero on success
#               - non zero on failure. return value from commands ($RC)

test01()
{
	RC=0				# Return value from commands.
	export TCID=mkdir01	# Name of the test case.
	export TST_COUNT=1	# Test number.
	numdirs=10
	numfiles=10
	dircnt=0
    fcnt=0

	$LTPBIN/tst_resm TINFO \
		"Test #1: mkdir -p will recursively mkdir contents of directory"

	dirname=$LTPTMP/tst_mkdir.tmp
	$LTPBIN/tst_resm TINFO "Test #1: Creating $numdirs directories."
	$LTPBIN/tst_resm TINFO "Test #1: filling each dir with $numfiles files".
	while [ $dircnt -lt $numdirs ]
	do
		dirname=$dirname/d.$dircnt
        mkdir -p $dirname  > $LTPTMP/tst_mkdir.err 2>&1 || RC=$?
		if [ $RC -ne 0 ]
		then
			$LTPBIN/tst_res TFAIL $LTPTMP/tst_mkdir.err \
				"Test #1: mkdir -p $dirname failed. Reason:"
			return $RC
		fi
		fcnt=0
        while [ $fcnt -lt $numfiles ]
        do
			touch $dirname/f.$fcnt
			if [ $RC -ne 0 ]
			then
				$LTPBIN/tst_brk TBROK $LTPTMP/tst_mkdir.err NULL \
				"Test #1: while creating $numdirs dirs.  Reason"
				return $RC
			fi
			fcnt=$(($fcnt+1))
		done
		dircnt=$(($dircnt+1))
	done

	$LTPBIN/tst_resm TINFO "Test #1: creating output file"
	ls -R $LTPTMP/tst_mkdir.tmp > $LTPTMP/tst_mkdir.out 2>&1

	$LTPBIN/tst_resm TINFO "Test #1: creating expected output file"
	creat_expout $numdirs $numfiles $LTPTMP/tst_mkdir.tmp

	$LTPBIN/tst_resm TINFO \
	    "Test #1: comparing expected out and actual output file"
	diff -w -B -q $LTPTMP/tst_mkdir.out $LTPTMP/tst_mkdir.exp \
		> $LTPTMP/tst_mkdir.err 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		$LTPBIN/tst_res TFAIL $LTPTMP/tst_mkdir.err \
			"Test #1: mkdir -p failed. Reason:"
	else
		$LTPBIN/tst_resm TINFO "Test #1: expected same as actual"
		$LTPBIN/tst_resm TPASS "Test #1: mkdir -R success"
	fi
	return $RC
}


# Function:		main
#
# Description:	- Execute all tests, report results.
#               
# Exit:			- zero on success
# 				- non-zero on failure.


TFAILCNT=0			# Set TFAILCNT to 0, increment on failure.
RC=0				# Return code from tests.

init || return $RC	# Exit if initializing testcases fails.	

test01 || RC=$?
if [ $RC -ne 0 ]
then
	TFAILCNT=$(($TFAILCNT+1))
fi


rm -fr $LTPTMP/tst_mkdir.*

exit $TFAILCNT
