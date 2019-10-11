#!/bin/sh
#   Copyright (c) International Business Machines  Corp., 2000
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
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
#    03/01 Robbie Williamson (robbiew@us.ibm.com)

TCID="rlogin01"
TST_TOTAL=1

TST_USE_LEGACY_API=1
. tst_net.sh

setup()
{
	tst_require_cmds rlogin expect

	if [ -z $RUSER ]; then
		RUSER=root
	fi

	if [ -z $PASSWD ]; then
		tst_brkm TCONF "Please set PASSWD for $RUSER."
	fi

	if [ -z $RHOST ]; then
		tst_brkm TCONF "Please set RHOST."
	fi

	if [ -z $LOOPCOUNT ]; then
		LOOPCOUNT=25
	fi
}

do_test()
{
	tst_resm TINFO "Starting"

	for i in $(seq 1 ${LOOPCOUNT})
	do
		rlogin_test || return 1
	done
}

rlogin_test()
{
	tst_resm TINFO "login with rlogin($i/$LOOPCOUNT)"

	expect -c "
		spawn rlogin $RHOST -l $RUSER

		expect {
			\"Password:\" {
				send \"$PASSWD\r\"; exp_continue
			} \"incorrect\" {
				exit 1
			} \"$RUSER@\" {
				send \"LC_ALL=C; ls -l /etc/hosts | \\
				       wc -w > $RUSER.$RHOST\rexit\r\";
				exp_continue
			}
		}
	" > /dev/null || return 1

	tst_resm TINFO "checking rlogin status($i/$LOOPCOUNT)"
	tst_rhost_run -u $RUSER -c "grep -q 9 $RUSER.$RHOST" || return 1
	tst_rhost_run -u $RUSER -c "rm -f $RUSER.$RHOST"
}

setup

do_test
if [ $? -ne 0 ]; then
	tst_resm TFAIL "Test $TCID failed."
else
	tst_resm TPASS "Test $TCID succeeded."
fi

tst_exit
