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
# File :         unzip_tests.sh
#
# Description:   Test Basic functionality of unzip command.
#                Test #1: Test that unzip -f <file.conf> rotates the logfile
#                as per the specifications in the conf file. Create a file
#                tst_logfile in /var/log/. Create a conf file such that this
#                logfile is set for rotation every week. Execute the command
#                unzip -f <file.conf>, check to see if it forced rotation.
#                Test #2: Check if unzip running as a cronjob will rotate a
#                logfile when it exceeds a specific size. Create two cronjobs 
#                1. runs a command to log a string to a logfile. 2. runs
#                unzip <file.conf> every minute. The conf file specifies
#                that the rotation happen only if the log file exceeds 2k file
#                size. 
#
# Author:        Manoj Iyer, manjo@mail.utexas.edu
#
# History:       Mar 03 2003 - Created - Manoj Iyer.
#
#! /bin/sh



# Function: 	chk_ifexists
#
# Description:  - Check if command required for this test exits.
#
# Input:        - $1 - calling test case.
#               - $2 - command that needs to be checked.
# 
# Return:		- zero on success.
# 				- non-zero on failure.
chk_ifexists()
{
	RC=0

	which $2 &>$LTPTMP/tst_unzip.err || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brkm TBROK NULL "$1: command $2 not found."
	fi
	return $RC
}


# Function: 	cleanup
#
# Description:  - remove temporaty files and directories. 
#
# Return:		- zero on success.
# 				- non-zero on failure.
cleanup()
{
	# remove all the temporary files created by this test.
	tst_resm TINFO "CLEAN: removing $LTPTMP"
	rm -fr $LTPTMP
	tst_resm TINFO "CLEAN: removing $PWD/tmp"
	rm -fr $PWD/tmp
}


# Function: init
#
# Description:  - Check if command required for this test exits.
#               - Create temporary directories required for this test. 
#               - Initialize global variables.
# 
# Return:		- zero on success.
# 				- non-zero on failure.
init()
{
	# Initialize global variables.
	export RC=0
	export TST_TOTAL=1
	export TCID="unzip"
	export TST_COUNT=0

	# Inititalize cleanup function.
	trap "cleanup" 0

	# check if commands tst_*, unzip, awk, etc exists.
	chk_ifexists INIT tst_resm  || return $RC
	chk_ifexists INIT unzip     || return $RC
	chk_ifexists INIT mkdir     || return $RC
	chk_ifexists INIT awk       || return $RC

	# create the temporary directory used by this testcase
	if [ -z $TMP ]
	then
		LTPTMP=/tmp/tst_unzip.$$
		TMP=/tmp
	else
		LTPTMP=$TMP/tst_unzip.$$
	fi

	mkdir -p $LTPTMP &>/dev/null || RC=$?
	if [ $RC -ne 0 ]
	then
		 tst_brkm TBROK "INIT: Unable to create temporary directory"
		 return $RC
	fi

	# create expected output files. tst_unzip.exp
	cat > $LTPTMP/tst_unzip.out.exp <<-EOF
	Archive:  $TMP/tst_unzip_file.zip
    creating: tmp/tst_unzip.dir/
    creating: tmp/tst_unzip.dir/d.0/
    extracting: tmp/tst_unzip.dir/d.0/f.0  
    extracting: tmp/tst_unzip.dir/d.0/f.1  
    extracting: tmp/tst_unzip.dir/d.0/f.2  
    creating: tmp/tst_unzip.dir/d.0/d.1/
    extracting: tmp/tst_unzip.dir/d.0/d.1/f.0  
    extracting: tmp/tst_unzip.dir/d.0/d.1/f.1  
    extracting: tmp/tst_unzip.dir/d.0/d.1/f.2  
    creating: tmp/tst_unzip.dir/d.0/d.1/d.2/
    extracting: tmp/tst_unzip.dir/d.0/d.1/d.2/f.0  
    extracting: tmp/tst_unzip.dir/d.0/d.1/d.2/f.1  
    extracting: tmp/tst_unzip.dir/d.0/d.1/d.2/f.2  
	EOF

	return $RC
}


# Function: 	test01
#
# Description:  - Test that unzip can uncompress .zip file correctly.
#               - Execute unzip command on a .zip file, save output to file.
#               - If unzip exits with a non-zero value or, the expected output
#                 is different from actual output, test fails.
# 
# Return:		- zero on success.
# 				- non-zero on failure.
test01()
{
	count=0
	files=" "
	filesize=0

	TCID=unzip01
	TST_COUNT=1

	tst_resm TINFO "Test #1: unzip command un-compresses a .zip file."

	unzip $TMP/tst_unzip_file.zip &>$LTPTMP/tst_unzip.out || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_res TFAIL $LTPTMP/tst_unzip.out \
			"Test #1: unzip command failed. Return value = $RC. Details:"
		return $RC
	else
		diff -iwB $LTPTMP/tst_unzip.out $LTPTMP/tst_unzip.out.exp \
			&>$LTPTMP/tst_unzip.out.err || RC=$?
		if [ $RC -ne 0 ]
		then
			tst_res TFAIL $LTPTMP/tst_unzip.err \
				"Test #1: unzip output differs from expected output. Details"
		else
			tst_resm TINFO "Test #1: check if $PWD/tmp/tst_unzip.dir exits."
			if ! [ -d $PWD/tmp/tst_unzip.dir ]
			then
				tst_resm TFAIL \
					"Test #1: unzip did not uncompress the .zip file"
				$((RC+1))
			else
				tst_resm TINFO \
					"Test #1: $PWD/tmp/tst_unzip.dir was created by unzip"
				tst_resm TPASS \
				   "Test #1: unzip can uncompress .zip file correctly."
			fi
		fi
	fi
	
	return $RC
}

# Function:	main
#
# Description:	- Execute all tests and report results.
#
# Exit:			- zero on success 
#               - non-zero on failure.

RC=0
init || exit $?

test01 || RC=$?

exit $RC
