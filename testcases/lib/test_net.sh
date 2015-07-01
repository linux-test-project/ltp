#!/bin/sh
# Copyright (c) 2014-2015 Oracle and/or its affiliates. All Rights Reserved.
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

[ -z "$TST_LIB_LOADED" ] && . test.sh

# Run command on remote host.
# Options:
# -b run in background
# -s safe option, if something goes wrong, will exit with TBROK
# -c specify command to run

tst_rhost_run()
{
	local pre_cmd=
	local post_cmd=' || echo RTERR'
	local out=
	local user="root"
	local cmd=
	local safe=0

	OPTIND=0

	while getopts :bsc:u: opt; do
		case "$opt" in
		b)
			pre_cmd="nohup"
			post_cmd=" > /dev/null 2>&1 &"
			out="1> /dev/null"
		;;
		s) safe=1 ;;
		c) cmd=$OPTARG ;;
		u) user=$OPTARG ;;
		*)
			tst_brkm TBROK "tst_rhost_run: unknown option: $OPTARG"
		;;
		esac
	done

	OPTIND=0

	if [ -z "$cmd" ]; then
		[ "$safe" -eq 1 ] && \
			tst_brkm TBROK "tst_rhost_run: command not defined"
		tst_resm TWARN "tst_rhost_run: command not defined"
		return 1
	fi

	local output=
	local ret=0
	if [ -n "$TST_USE_SSH" ]; then
		output=`ssh -n -q $user@$RHOST "sh -c \
			'$pre_cmd $cmd $post_cmd'" $out 2>&1 || echo 'RTERR'`
	else
		output=`rsh -n -l $user $RHOST "sh -c \
			'$pre_cmd $cmd $post_cmd'" $out 2>&1 || echo 'RTERR'`
	fi
	echo "$output" | grep -q 'RTERR$' && ret=1
	if [ $ret -eq 1 ]; then
		output=$(echo "$output" | sed 's/RTERR//')
		[ "$safe" -eq 1 ] && \
			tst_brkm TBROK "'$cmd' failed on '$RHOST': '$output'"
	fi

	[ -z "$out" -a -n "$output" ] && echo "$output"

	return $ret
}

# Get test interface names for local/remote host.
# tst_get_ifaces [TYPE]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
tst_get_ifaces()
{
	local type=${1:-"lhost"}
	if [ "$type" = "lhost" ]; then
		echo "$LHOST_IFACES"
	else
		echo "$RHOST_IFACES"
	fi
}

# Get HW addresses from defined test interface names.
# tst_get_hwaddrs [TYPE]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
tst_get_hwaddrs()
{
	local type=${1:-"lhost"}
	local addr=
	local list=

	for eth in $(tst_get_ifaces $type); do

		local addr_path="/sys/class/net/${eth}/address"

		case $type in
		lhost) addr=$(cat $addr_path) ;;
		rhost) addr=$(tst_rhost_run -s -c "cat $addr_path")
		esac

		[ -z "$list" ] && list="$addr" || list="$list $addr"
	done
	echo "$list"
}

# Get test HW address.
# tst_hwaddr [TYPE] [LINK]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# LINK: link number starting from 0. Default value is '0'.
tst_hwaddr()
{
	local type=${1:-"lhost"}
	local link_num=${2:-"0"}
	local hwaddrs=
	link_num=$(( $link_num + 1 ))
	[ "$type" = "lhost" ] && hwaddrs=$LHOST_HWADDRS || hwaddrs=$RHOST_HWADDRS
	echo "$hwaddrs" | awk '{ print $'"$link_num"' }'
}

# Get test interface name.
# tst_iface [TYPE] [LINK]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# LINK: link number starting from 0. Default value is '0'.
tst_iface()
{
	local type=${1:-"lhost"}
	local link_num=${2:-"0"}
	link_num=$(( $link_num + 1 ))
	echo "$(tst_get_ifaces $type)" | awk '{ print $'"$link_num"' }'
}

# Blank for an IPV4 test; 6 for an IPV6 test.
TST_IPV6=

tst_read_opts()
{
	OPTIND=0
	while getopts ":6" opt; do
		case "$opt" in
		6)
			TST_IPV6=6;;
		esac
	done
	OPTIND=0
}

tst_read_opts $*

# Get IP address
# tst_ipaddr [TYPE]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
tst_ipaddr()
{
	local type=${1:-"lhost"}
	local ipv=${TST_IPV6:-"4"}
	local tst_host=

	if [ "$type" = "lhost" ]; then
		eval "tst_host=\$LHOST_IPV${ipv}_HOST"
	else
		eval "tst_host=\$RHOST_IPV${ipv}_HOST"
	fi

	if [ "$TST_IPV6" ]; then
		echo "${IPV6_NETWORK}:${tst_host}"
	else
		echo "${IPV4_NETWORK}.${tst_host}"
	fi
}

# tst_init_iface [TYPE] [LINK]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# LINK: link number starting from 0. Default value is '0'.
tst_init_iface()
{
	local type=${1:-"lhost"}
	local link_num=${2:-"0"}
	local iface=$(tst_iface $type $link_num)
	tst_resm TINFO "initialize '$type' '$iface' interface"

	if [ "$type" = "lhost" ]; then
		ip link set $iface down || return $?
		ip route flush dev $iface || return $?
		ip addr flush dev $iface || return $?
		ip link set $iface up
		return $?
	fi

	tst_rhost_run -c "ip link set $iface down" || return $?
	tst_rhost_run -c "ip route flush dev $iface" || return $?
	tst_rhost_run -c "ip addr flush dev $iface" || return $?
	tst_rhost_run -c "ip link set $iface up"
}

# tst_add_ipaddr [TYPE] [LINK]
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# LINK: link number starting from 0. Default value is '0'.
tst_add_ipaddr()
{
	local type=${1:-"lhost"}
	local link_num=${2:-"0"}

	local mask=24
	[ "$TST_IPV6" ] && mask=64

	local iface=$(tst_iface $type $link_num)

	if [ $type = "lhost" ]; then
		tst_resm TINFO "set local addr $(tst_ipaddr)/$mask"
		ip addr add $(tst_ipaddr)/$mask dev $iface
		return $?
	fi

	tst_resm TINFO "set remote addr $(tst_ipaddr rhost)/$mask"
	tst_rhost_run -c "ip addr add $(tst_ipaddr rhost)/$mask dev $iface"
}

# tst_restore_ipaddr [TYPE] [LINK]
# Restore default ip addresses defined in network.sh
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# LINK: link number starting from 0. Default value is '0'.
tst_restore_ipaddr()
{
	local type=${1:-"lhost"}
	local link_num=${2:-"0"}

	tst_init_iface $type $link_num || return $?

	local ret=0
	local backup_tst_ipv6=$TST_IPV6
	TST_IPV6= tst_add_ipaddr $type $link_num || ret=$?
	TST_IPV6=6 tst_add_ipaddr $type $link_num || ret=$?
	TST_IPV6=$backup_tst_ipv6

	return $ret
}

# tst_netload ADDR [FILE] [TYPE]
# Run network load test
# ADDR: IP address
# FILE: file with result time
# TYPE: PING or TFO (TCP traffic)
tst_netload()
{
	local ip_addr="$1"
	local rfile=${2:-"netload.res"}
	local type=${3:-"TFO"}
	local ret=0

	case "$type" in
	PING)
		local ipv6=
		echo "$ip_addr" | grep ":" > /dev/null
		[ $? -eq 0 ] && ipv6=6
		tst_resm TINFO "run ping${ipv6} test with rhost '$ip_addr'..."
		local res=
		res=$(ping${ipv6} -f -c $client_requests $ip_addr -w 600 2>&1)
		[ $? -ne 0 ] && return 1
		echo $res | sed -nE 's/.*time ([0-9]+)ms.*/\1/p' > $rfile
	;;
	TFO)
		local port=
		port=$(tst_rhost_run -c 'tst_get_unused_port ipv6 stream')
		[ $? -ne 0 ] && tst_brkm TBROK "failed to get unused port"

		tst_resm TINFO "run tcp_fastopen with '$ip_addr', port '$port'"
		tst_rhost_run -s -b -c "tcp_fastopen -R $max_requests -g $port"

		# run local tcp client
		tcp_fastopen -a $clients_num -r $client_requests -l \
			-H $ip_addr -g $port -d $rfile > /dev/null || ret=1

		if [ $ret -eq 0 -a ! -f $rfile ]; then
			tst_brkm TBROK "can't read $rfile"
		fi

		tst_rhost_run -c "pkill -9 tcp_fastopen\$"
	;;
	*) tst_brkm TBROK "invalid net_load type '$type'" ;;
	esac

	return $ret
}
