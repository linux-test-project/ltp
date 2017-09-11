#!/bin/sh

# Copyright (C) 2017 Red Hat, Inc.
#
# This program is free software;  you can redistribute it and#or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#

# Test description: See if gdb can successfully attach to a process and
#                   this process exits normally.

# Usage
# ./gdb01.sh

TST_TESTFUNC=simple_test
TST_NEEDS_CMDS="gdb /bin/cat"

. tst_test.sh

simple_test()
{
	gdb /bin/cat -ex "run /etc/passwd" -ex quit < /dev/null
	RC=$?
	if [ $RC -eq 0 ] ; then
		tst_res TPASS "gdb attached to process and completed run"
	else
		tst_res TFAIL "gdb test failed with" $RC
	fi
}

tst_run
