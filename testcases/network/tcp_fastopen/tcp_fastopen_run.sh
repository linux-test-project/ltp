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

tfo_result="netload.res"

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
	tst_rhost_run -c "pkill -9 netstress\$"
	tst_rmdir
}

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

tst_require_root

tst_kvercmp 3 7 0
[ $? -eq 0 ] && tst_brkm TCONF "test must be run with kernel 3.7 or newer"

tst_kvercmp 3 16 0
[ $? -eq 0 -a "$TST_IPV6" ] && \
	tst_brkm TCONF "test must be run with kernel 3.16 or newer"

trap "tst_brkm TBROK 'test interrupted'" INT
TST_CLEANUP="cleanup"
tst_tmpdir

tst_resm TINFO "using old TCP API"
tst_netload $(tst_ipaddr rhost) $tfo_result TFO -o -O
time_tfo_off=$(read_result_file)

tst_resm TINFO "using new TCP API"
tst_netload $(tst_ipaddr rhost) $tfo_result TFO
time_tfo_on=$(read_result_file)

tfo_cmp=$(( 100 - ($time_tfo_on * 100) / $time_tfo_off ))

if [ "$tfo_cmp" -lt 3 ]; then
	tst_resm TFAIL "TFO performance result is '$tfo_cmp' percent"
else
	tst_resm TPASS "TFO performance result is '$tfo_cmp' percent"
fi

tst_exit
