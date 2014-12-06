#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (C) 2009 IBM Corporation                                         ##
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
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
################################################################################
#
# File :        ima_violations.sh
#
# Description:  This file tests ToMToU and open_writer violations invalidate
#		the PCR and are logged.
#
# Author:       Mimi Zohar, zohar@ibm.vnet.ibm.com
#
# Return        - zero on success
#               - non zero on failure. return value from commands ($RC)
################################################################################

export TST_TOTAL=3
export TCID="ima_violations"

open_file_read()
{
	exec 3< $1
	if [ $? -ne 0 ]; then
		exit 1
	fi
}

close_file_read()
{
	exec 3>&-
}

open_file_write()
{
	exec 4> $1
	if [ $? -ne 0 ]; then
		exit 1
	echo 'testing, testing, ' >&4
	fi
}

close_file_write()
{
	exec 4>&-
}

init()
{
	service auditd status > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		log=/var/log/messages
	else
		log=/var/log/audit/audit.log
		tst_resm TINFO "requires integrity auditd patch"
	fi

	ima_violations=$SECURITYFS/ima/violations
}

# Function:     test01
# Description	- Verify open writers violation
test01()
{
	read num_violations < $ima_violations

	TMPFN=test.txt
	open_file_write $TMPFN
	open_file_read $TMPFN
	close_file_read
	close_file_write
	read num_violations_new < $ima_violations
	num=$(($(expr $num_violations_new - $num_violations)))
	if [ $num -gt 0 ]; then
		tail $log | grep test.txt | grep -q 'open_writers'
		if [ $? -eq 0 ]; then
			tst_resm TPASS "open_writers violation added(test.txt)"
		else
			tst_resm TFAIL "(message ratelimiting?)"
		fi
	else
		tst_resm TFAIL "open_writers violation not added(test.txt)"
	fi
}

# Function:     test02
# Description   - Verify ToMToU violation
test02()
{
	read num_violations < $ima_violations

	TMPFN=test.txt
	open_file_read $TMPFN
	open_file_write $TMPFN
	close_file_write
	close_file_read
	read num_violations_new < $ima_violations
	num=$(($(expr $num_violations_new - $num_violations)))
	if [ $num -gt 0 ]; then
		tail $log | grep test.txt | grep -q 'ToMToU'
		if [ $? -eq 0 ]; then
			tst_resm TPASS "ToMToU violation added(test.txt)"
		else
			tst_resm TFAIL "(message ratelimiting?)"
		fi
	else
		tst_resm TFAIL "ToMToU violation not added(test.txt)"
	fi
}

# Function:     test03
# Description 	- verify open_writers using mmapped files
test03()
{
	read num_violations < $ima_violations

	TMPFN=test.txtb
	echo 'testing testing ' > $TMPFN
	ima_mmap $TMPFN & p1=$!
	sleep 1		# got to wait for ima_mmap to mmap the file
	open_file_read $TMPFN
	read num_violations_new < $ima_violations
	num=$(($(expr $num_violations_new - $num_violations)))
	if [ $num -gt 0 ]; then
		tail $log | grep test.txtb | grep -q 'open_writers'
		if [ $? -eq 0 ]; then
			tst_resm TPASS "mmapped open_writers violation added(test.txtb)"
		else
			tst_resm TFAIL "(message ratelimiting?)"
		fi
	else
		tst_resm TFAIL "mmapped open_writers violation not added(test.txtb)"
	fi
	close_file_read
}

. ima_setup.sh

setup
TST_CLEANUP=cleanup

init
test01
test02
test03

tst_exit
