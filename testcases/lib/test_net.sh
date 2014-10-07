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

[ -z "$TST_LIB_LOADED" ] && . test.sh

# Run command on remote host.
# Options:
# -b run in background
# -s safe option, if something goes wrong, will exit with TBROK
# -c specify command to run

tst_rhost_run()
{
	# this is needed to run tools/apicmds on remote host
	local pre_cmd=
	local post_cmd=
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
			tst_brkm TBROK "tst_rhost_run: unknown option: $opt"
		;;
		esac
	done

	OPTIND=0

	[ -z "$cmd" ] && tst_brkm TBROK "command not defined"

	local output=
	local ret=
	if [ -n "$TST_USE_SSH" ]; then
		output=`ssh -n -q $user@$RHOST "sh -c \
			'$pre_cmd $cmd $post_cmd'" $out 2> /dev/null`
	else
		output=`rsh -n -l $user $RHOST "sh -c \
			'$pre_cmd $cmd $post_cmd'" $out 2> /dev/null`
	fi
	ret=$?
	[ "$ret" -ne 0 -a "$safe" -eq 1 ] && \
		tst_brkm TBROK "failed to run '$cmd' on '$RHOST'"

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
		ip link set $iface down || tst_brkm TBROK "iface down failed"
		ip route flush dev $iface || tst_brkm TBROK "route flush failed"
		ip addr flush dev $iface || tst_brkm TBROK "addr flush failed"
		ip link set $iface up || tst_brkm TBROK "iface up failed"
		return
	fi

	tst_rhost_run -s -c "ip link set $iface down"
	tst_rhost_run -s -c "ip route flush dev $iface"
	tst_rhost_run -s -c "ip addr flush dev $iface"
	tst_rhost_run -s -c "ip link set $iface up"
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

	if [ $type = "lhost" ]; then
		tst_resm TINFO "set local addr $(tst_ipaddr)/$mask"
		ip addr add $(tst_ipaddr)/$mask dev $iface || \
			tst_brkm TBROK "failed to add IP address"
		return
	fi

	tst_resm TINFO "set remote addr $(tst_ipaddr rhost)/$mask"
	tst_rhost_run -s -c "ip addr add $(tst_ipaddr rhost)/$mask dev $iface"
}

# tst_restore_ipaddr [TYPE] [LINK]
# Restore default ip addresses defined in network.sh
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# LINK: link number starting from 0. Default value is '0'.
tst_restore_ipaddr()
{
	local type=${1:-"lhost"}
	local link_num=${2:-"0"}

	tst_init_iface $type $link_num

	local iface=$(tst_iface $type $link_num)

	TST_IPV6= tst_add_ipaddr $type $link_num
	TST_IPV6=6 tst_add_ipaddr $type $link_num
}
