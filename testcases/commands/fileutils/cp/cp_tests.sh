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
# File :        cpio_test.sh
#
# Description:  Test basic functionality of cp command
#				- Test #1:  cpio -o can create an archive.
#               
# Author:       Manoj Iyer, manjo@mail.utexas.edu
#
# History:      Jan 30 2003 - Created - Manoj Iyer.
#
#! /bin/sh


# Function:		init
#
# Description:	- Check if command cp is available.
#               - Create temprary directory, and temporary files.
#               - Initialize environment variables.
#
# Return		- zero on success
#               - non zero on failure. return value from commands ($RC)
init()
{

	RC=0				# Return code from commands.
	export TST_TOTAL=1	# total numner of tests in this file.
	export TCID=cpio	# this is the init function.
	export TST_COUNT=0	# init identifier,

	if [[ -z $LTPTMP && -z $TMPBASE ]]
	then
		LTPTMP=/tmp
	else
		LTPTMP=$TMPBASE
	fi
	if [[ -z $LTPBIN && -z $LTPROOT ]]
	then
		LTPBIN=./
	else
		LTPBIN=$LTPROOT/testcases/bin
	fi

	
	$LTPBIN/tst_resm TINFO "INIT: Inititalizing tests."

	which cp &> $LTPTMP/tst_cpio.err || RC=$?
	if [ $RC -ne 0 ]
	then
		$LTPBIN/tst_brk TBROK $LTPTMP/tst_cpio.err NULL \
			"Test #1: cp command does not exist. Reason:"
		exit $RC
	fi

	mkdir -p $LTPTMP/tst_cp.tmp &> $LTPTMP/tst_cpio.err || RC=$? 
	if [ $RC -ne 0 ]
	then
		$LTPBIN/tst_brk TBROK $LTPTMP/tst_cpio.err NULL \
			"Test #1: failed creating temp directory. Reason:"
		exit $RC
	fi
}


# Function:		test01
#
# Description	- Test #1: Test that cp -R will copy will copy directories 
#                 recursively.
#
# Return		- zero on success
#               - non zero on failure. return value from commands ($RC)

test01()
{
	RC=0				# Return value from commands.
	export TCID=cp01	# Name of the test case.
	export TST_COUNT=1	# Test number.

	$LTPBIN/tst_resm TINFO "Test #1: cpio -o will create an archive."

	find  $LTPTMP/tst_cpio.tmp/ -type f | cpio -o > $LTPTMP/tst_cpio.out \
		2>$LTPTMP/tst_cpio.err || RC=$?
	if [ $RC -ne 0 ]
	then
		 $LTPBIN/tst_res TFAIL $LTPTMP/tst_cpio.err \
			"Test #1: creating cpio archive failed. Reason:"
		exit $RC
	else
		if [ -f $LTPTMP/tst_cpio.out ]
		then
			file $LTPTMP/tst_cpio.out &>$LTPTMP/tst_cpio.err || RC=$?
			if [ $? -ne 0 ]
			then
				$LTPBIN/tst_res TFAIL $LTPTMP/tst_cpio.err	\
				"Test #1: bad output, not cpio format. Reason:"
				exit $RC
			fi
		else
			 $LTPBIN/tst_resm TFAIL "Test #1: did not create cpio file."
			 exit $RC
		fi
	fi
}


# Function:		main
#
# Description:	- Execute all tests, report results.
#               
# Exit:			- zero on success
# 				- non-zero on failure.


TFAILCNT=0			# Set TFAILCNT to 0, increment on failure.
RC=0				# Return code from tests.

init || exit $RC	# Exit if initializing testcases fails.	

test01 || RC=$?		# Test #1
if [ $RC -eq 0 ]
then
	$LTPBIN/tst_resm TPASS "Test #1: cpio created an archive"
else
	TFAILCNT=$((TFAILCNT+1))
fi

exit $TFAILCNT
