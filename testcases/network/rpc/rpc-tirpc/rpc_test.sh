#!/bin/sh
#
# Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
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
# This is a wrapper script to execute tests from the RPC/TI-RPC tests
# suite (http://nfsv4.bullopensource.org/doc/rpc_testsuite.php) in LTP.

SERVER=""
CLIENT=""
CLIENT_EXTRA_OPTS=""
CLEANER=""
# Program number to register the services to rpcbind
PROGNUMNOSVC=536875000
SERVER_STARTUP_SLEEP=1

cleanup()
{
	if [ ! -z "$SERVER" ]; then
		killall -9 $SERVER
		$CLEANER $PROGNUMNOSVC
	fi
}

usage()
{
	cat << EOF
USAGE: $0 [-s sprog] -c clprog [ -e extra ]

sprog   - server program binary
clprog  - client program binary
extra   - extra client options

This scripts connects to the remote host and starts sprog there. After that it
executes clprog passing it the remote host value.

After the test completes, this script kills sprog on remote and performs a
cleaning operation.
EOF

	exit 1
}

while getopts s:c:e:h arg; do
	case $arg in
		s) SERVER="$LTPROOT/testcases/bin/$OPTARG" ;;
		c) CLIENT="$OPTARG" ;;
		e) CLIENT_EXTRA_OPTS="$OPTARG" ;;
		h) usage ;;
	esac
done

if [ ! -z "$SERVER" ]; then
	if `echo "$SERVER" | grep -e '^tirpc'`; then
		CLEANER="$LTPROOT/testcases/bin/tirpc_cleaner"
	else
		CLEANER="$LTPROOT/testcases/bin/rpc_cleaner"
	fi
fi

if [ -z "$CLIENT" ]; then
	echo "client program not set"
	echo ""
	usage
fi

TCID="$CLIENT"
TST_TOTAL=1
TST_COUNT=1
TST_CLEANUP=cleanup

. test_net.sh

if [ ! -z "$SERVER" ]; then
	$SERVER $PROGNUMNOSVC &
	sleep "$SERVER_STARTUP_SLEEP"
fi

tst_rhost_run -sc "$CLIENT $(tst_ipaddr) $PROGNUMNOSVC $CLIENT_EXTRA_OPTS"

tst_resm TPASS "Test passed"

tst_exit
