#!/bin/sh
# Copyright (c) 2017-2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) International Business Machines  Corp., 2006
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Setup: testcases/network/stress/README
#
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

. mcast-lib.sh

do_setup()
{
	# Increase the maximum number of open file descriptors
	if [ $(ulimit -n) -lt $MCASTNUM_HEAVY ]; then
		ulimit -n $MCASTNUM_HEAVY || tst_brk TCONF \
			"Failed to set the maximum number of open file descriptors to $MCASTNUM_HEAVY"
	fi

	mcast_setup $MCASTNUM_HEAVY
}

do_test()
{
	tst_res TINFO "joining $MCASTNUM_HEAVY IPv$TST_IPVER multicast groups on multiple sockets"
	do_multicast_test_multiple_join $MCASTNUM_HEAVY true
}

tst_run
