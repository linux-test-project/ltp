#!/bin/sh
# Copyright (c) 2014-2016 Oracle and/or its affiliates. All Rights Reserved.
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
	tst_rmdir
}

tst_require_root

if tst_kvcmp -lt "3.7"; then
	tst_brkm TCONF "test must be run with kernel 3.7 or newer"
fi

if tst_kvcmp -lt "3.16" && [ "$TST_IPV6" ]; then
	tst_brkm TCONF "test must be run with kernel 3.16 or newer"
fi

trap "tst_brkm TBROK 'test interrupted'" INT
TST_CLEANUP="cleanup"
tst_tmpdir

tst_resm TINFO "using old TCP API and set tcp_fastopen to '0'"
tst_netload -H $(tst_ipaddr rhost) -a $clients_num -r $client_requests \
	-R $max_requests -t 0
time_tfo_off=$(cat tst_netload.res)

tst_resm TINFO "using new TCP API and set tcp_fastopen to '3'"
tst_netload -H $(tst_ipaddr rhost)  -a $clients_num -r $client_requests \
	-R $max_requests -f -t 3
time_tfo_on=$(cat tst_netload.res)

tfo_cmp=$(( 100 - ($time_tfo_on * 100) / $time_tfo_off ))

if [ "$tfo_cmp" -lt 3 ]; then
	tst_resm TFAIL "TFO performance result is '$tfo_cmp' percent"
else
	tst_resm TPASS "TFO performance result is '$tfo_cmp' percent"
fi

tst_exit
