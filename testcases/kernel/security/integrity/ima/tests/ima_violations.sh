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
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
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
	export TST_TOTAL=3
	export TCID="init"
	export TST_COUNT=0
	RC=0

	if [ -f /etc/init.d/auditd ]; then
		service auditd status > /dev/null 2>&1 || RC=$?
	else
		RC=$?
	fi

	if [ $RC -ne 0 ]; then
		log=/var/log/messages
	else
		log=/var/log/audit/audit.log
		tst_res TINFO $LTPTMP/imalog.$$ \
		 "$TCID: requires integrity auditd patch"
	fi
}

# Function:     test01
# Description   - Verify ToMToU violation
test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	ima_violations=$SECURITYFS/ima/violations
	read num_violations < $ima_violations

	TMPFN=$LTPIMA/test.txt-$$
	open_file_write $TMPFN
	open_file_read $TMPFN
	close_file_read
	close_file_write
	read num_violations_new < $ima_violations
	num=$((`expr $num_violations_new - $num_violations`))
	if [ $num -gt 0 ]; then
		tail $log | grep test.txt-$$ | \
			grep 1>/dev/null 'open_writers' || RC=$?
		if [ $RC -eq 0 ]; then
			tst_res TPASS $LTPTMP/imalog.$$ \
			 "$TCID: open_writers violation added(test.txt-$$)"
			return $RC
		else
			tst_res TINFO $LTPTMP/imalog.$$ \
			 "$TCID: (message ratelimiting?)"
		fi
	fi
	tst_res TFAIL $LTPTMP/imalog.$$ \
	 "$TCID: open_writers violation not added(test.txt-$$)"
	return $RC
}

# Function:     test02
# Description	- Verify open writers violation
test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	ima_violations=$SECURITYFS/ima/violations
	read num_violations < $ima_violations

	TMPFN=$LTPIMA/test.txt-$$
	open_file_read $TMPFN
	open_file_write $TMPFN
	close_file_write
	close_file_read
	read num_violations_new < $ima_violations
	num=$((`expr $num_violations_new - $num_violations`))
	if [ $num -gt 0 ]; then
		tail $log | grep test.txt-$$ | \
			grep 'ToMToU' 1>/dev/null || RC=$?
		if [ $RC -eq 0 ]; then
			tst_res TPASS $LTPTMP/imalog.$$ \
			 "$TCID: ToMToU violation added(test.txt-$$)"
			return $RC
		else
			tst_res TINFO $LTPTMP/imalog.$$ \
			 "$TCID: (message ratelimiting?)"
		fi
	fi
	tst_res TFAIL $LTPTMP/imalog.$$ \
	 "$TCID: ToMToU violation not added(test.txt-$$)"
	return $RC
}

# Function:     test03
# Description 	- verify open_writers using mmapped files
test03()
{
	TCID="test03"
	TST_COUNT=3
	RC=0

	ima_violations=$SECURITYFS/ima/violations
	read num_violations < $ima_violations

	TMPFN=$LTPIMA/test.txtb-$$
	mkdir -p $LTPIMA
	echo 'testing testing ' > $TMPFN
	ima_mmap $TMPFN & p1=$!
	sleep 1		# got to wait for ima_mmap to mmap the file
	open_file_read $TMPFN
	read num_violations_new < $ima_violations
	num=$((`expr $num_violations_new - $num_violations`))
	if [ $num -gt 0 ]; then
		tail $log | grep test.txtb-$$ | \
			grep 1>/dev/null 'open_writers' || RC=$?
		if [ $RC -eq 0 ]; then
			tst_res TPASS $LTPTMP/imalog.$$ \
			 "$TCID: mmapped open_writers violation added(test.txtb-$$)"
			return $RC
		else
			tst_res TINFO $LTPTMP/imalog.$$ \
			 "$TCID: (message ratelimiting?)"
		fi
	fi
	tst_res TFAIL $LTPTMP/imalog.$$ \
	 "$TCID: mmapped open_writers violation not added(test.txtb-$$)"
	close_file_read
	return $RC
}

# Function:     main
#
# Description:  - Execute all tests, exit with test status.
#
# Exit:         - zero on success
#               - non-zero on failure.
#
RC=0    # Return value from setup, init, and test functions.
EXIT_VAL=0

. $(dirname "$0")/ima_setup.sh
setup || exit $?
init || exit $?
test01 || EXIT_VAL=$RC
test02 || EXIT_VAL=$RC
test03 || EXIT_VAL=$RC
exit $EXIT_VAL
