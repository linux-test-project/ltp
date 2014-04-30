#!/bin/sh
#
# Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
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
# suite (http://nfsv4.bullopensource.org/doc/rpc_testsuite.php) in LTP
#
# This wrapper uses the RHOST environment variable:
#
# If the RHOST variable is set, then the rpc server instance (if needed)
# is started on RHOST, using rsh, and the client program is passed
# the RHOST value.
#
# If the RHOST variable is not set, then the rpc server instance (if needed)
# is started on the local host, and the client program is passed `hostname`.

SERVER_HOST=${RHOST:-`hostname`}
SERVER=""
CLIENT=""
CLIENT_EXTRA_OPTS=""
CLEANER=""
# Program number to register the services to rpcbind
PROGNUMNOSVC=536875000
SERVER_STARTUP_SLEEP=1

run_cmd()
{
	if [ ! -z "$RHOST" ]; then
		rsh -n "$RHOST" "$1"
	else
		$1
	fi
}

cleanup()
{
	if [ ! -z "$SERVER" ]; then
		run_cmd "killall -9 $SERVER"
		run_cmd "$CLEANER $PROGNUMNOSVC"
	fi
}

usage()
{
	echo "USAGE: $0 [-s sprog] -c clprog [ -e extra ]"
	echo ""
	echo "sprog   - server program binary"
	echo "clprog  - client program binary"
	echo "extra   - extra client options"
	echo ""
	echo "This scripts connects to the RHOST host by rsh and starts"
	echo "sprog there. After that it executes clprog passing it the"
	echo "RHOST value."
	echo "After the test completes, this script kills sprog on RHOST"
	echo "and performs a cleaning operation."
	echo ""
	echo "If RHOST is not set, the local host is used."

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
. test.sh
TST_CLEANUP=cleanup

if [ ! -z "$SERVER" ]; then
	run_cmd "$SERVER $PROGNUMNOSVC" &
	sleep "$SERVER_STARTUP_SLEEP"
fi

"$CLIENT" "$SERVER_HOST" "$PROGNUMNOSVC" $CLIENT_EXTRA_OPTS
ret=$?

if [ "$ret" -eq 0 ]; then
	tst_resm TPASS "Test passed"
else
	tst_resm TFAIL "Test failed"
fi

tst_exit
