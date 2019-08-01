#!/bin/sh
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2000
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
# along with this program; if not, write the Free Software Foundation,
# Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
#  PURPOSE: To verify that three of the new options for level IPPROTO_IP
#           Service Interface are initially set to the default values as
#           defined in the documentation and that each of the new options
#           can be set and read properly by the setsockopt and getsockopt
#           routines, respectively.  To test boundary conditions and to
#           generate errors while exercising the IP Multicast Service
#           Interface options.
#
#
# Robbie Williamson (robbiew@us.ibm.com)
#  -Ported

EXECUTABLES=${EXECUTABLES:-"mc_verify_opts mc_verify_opts_error"}
TCID=mc_opts
TST_TOTAL=1

TST_USE_LEGACY_API=1
. tst_net.sh

COUNT=1
while [ $COUNT -le 10 ]; do
	# Run setsockopt/getsockopt test
	tst_resm TINFO "Running $EXECUTABLES on $(tst_ipaddr)"
	for i in $EXECUTABLES; do
		$i $(tst_ipaddr) > /dev/null || \
			tst_brkm TFAIL "$i $(tst_ipaddr) failed"
	done

	tst_resm TINFO "Running ping with bad values"
	ping -T 777 224.0.0.1 > /dev/null 2>&1 && \
		tst_brkm TFAIL "Multicast range should be out of range"

	tst_resm TINFO "Running ping on a invalid interface!"
	ping -I 3.3.3.3 224.0.0.1 > /dev/null 2>&1 && \
		tst_brkm TFAIL "ping on bogus interface should fail"

	COUNT=$(( $COUNT + 1 ))
done

tst_resm TPASS "Test Successful"
tst_exit
