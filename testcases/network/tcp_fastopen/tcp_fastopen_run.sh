#!/bin/sh

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
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#

# default command-line options
user_name="root"
use_ssh=0
clients_num=2
client_requests=2000000
max_requests=3

TST_TOTAL=1
TCID="tcp_fastopen"

. test_net.sh

bind_timeout=5
tfo_result="${TMPDIR}/tfo_result"

while getopts :hu:sr:p:n:R:6 opt; do
	case "$opt" in
	h)
		echo "Usage:"
		echo "h        help"
		echo "u x      server user name"
		echo "s        use ssh to run remote cmds"
		echo "n x      num of clients running in parallel"
		echo "r x      the number of client requests"
		echo "R x      num of requests, after which conn. closed"
		echo "6        run over IPv6"
		exit 0
	;;
	u) user_name=$OPTARG ;;
	s) export TST_USE_SSH=1 ;;
	n) clients_num=$OPTARG ;;
	r) client_requests=$OPTARG ;;
	R) max_requests=$OPTARG ;;
	6) # skip, test_net library already processed it
	;;
	*) tst_brkm TBROK "unknown option: $opt" ;;
	esac
done

cleanup()
{
	tst_resm TINFO "cleanup..."
	tst_rhost_run -c "pkill -9 tcp_fastopen\$"
	rm -f $tfo_result
}

TST_CLEANUP="cleanup"
trap "tst_brkm TBROK 'test interrupted'" INT

read_result_file()
{
	if [ -f $tfo_result ]; then
		if [ -r $tfo_result ]; then
			cat $tfo_result
		else
			tst_brkm TBROK "Failed to read result file"
		fi
	else
		tst_brkm TBROK "Failed to find result file"
	fi
}

run_client_server()
{
	# kill tcp server on remote machine
	tst_rhost_run -c "pkill -9 tcp_fastopen\$"

	port=$(tst_rhost_run -c "tst_get_unused_port ipv6 stream")
	[ $? -ne 0 ] && tst_brkm TBROK "failed to get unused port"

	# run tcp server on remote machine
	tst_rhost_run -s -b -c "tcp_fastopen -R $max_requests $1 -g $port"
	sleep $bind_timeout

	# run local tcp client
	tcp_fastopen -a $clients_num -r $client_requests -l \
		-H $(tst_ipaddr rhost) $1 -g $port -d $tfo_result
	[ "$?" -ne 0 ] && tst_brkm TBROK "Last test has failed"

	run_time=$(read_result_file)

	[ -z "$run_time" -o "$run_time" -eq 0 ] && \
		tst_brkm TBROK "Last test result isn't valid: $run_time"
}

tst_require_root

tst_kvercmp 3 7 0
[ $? -eq 0 ] && tst_brkm TCONF "test must be run with kernel 3.7 or newer"

tst_kvercmp 3 16 0
[ $? -eq 0 -a "$TST_IPV6" ] && \
	tst_brkm TCONF "test must be run with kernel 3.16 or newer"

run_client_server "-o -O"
time_tfo_off=$run_time

run_client_server
time_tfo_on=$run_time

tfo_cmp=$(( 100 - ($time_tfo_on * 100) / $time_tfo_off ))

if [ "$tfo_cmp" -lt 3 ]; then
	tst_resm TFAIL "TFO performance result is '$tfo_cmp' percent"
else
	tst_resm TPASS "TFO performance result is '$tfo_cmp' percent"
fi

tst_exit
