#!/bin/sh
# Copyright (c) 2015-2017 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2005
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
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

TCID=dns-stress
TST_TOTAL=2
TST_CLEANUP="cleanup"

TST_USE_LEGACY_API=1

# Minimum host ID in the zone file.
# The ID is used as the host portion of the address
MIN_ID=2
# Maximum host ID in the zone file.
MAX_ID=254
# Domain name for testing
DOMAIN="ltp-ns.org"

cleanup()
{
	# Stop the dns daemon
	test -s named.pid && kill -9 $(cat named.pid) > /dev/null
	tst_rmdir
}

common_setup()
{
	tst_require_root
	tst_require_cmds named dig

	if [ "$TST_IPV6" ]; then
		record="AAAA"
		net="fd00:cafe"
		net_rev="0.0.0.0.0.0.0.0.e.f.a.c.0.0.d.f"
	else
		record="A"
		net="10.23.0"
		net_rev="0.23.10"
	fi

	trap "tst_brkm TBROK 'test interrupted'" INT

	tst_tmpdir

	ip6_opt=
	[ "$TST_IPV6" ] && ip6_opt="listen-on-v6 { any; };"

	ip_zone_opt="in-addr"
	[ "$TST_IPV6" ] && ip_zone_opt="ip6"

	cat << EOD > named.conf
	options {
		directory "$(pwd)";
		pid-file "named.pid";
		recursion no;
		$ip6_opt
	};

	zone "$DOMAIN" {
		type master;
		file "ltp-ns.zone";
	};

	zone "$net_rev.$ip_zone_opt.arpa" {
		type master;
		file "ltp-ns.rev";
	};
EOD

	# zone file
	cat << EOD > ltp-ns.zone
\$TTL 10
@	IN	SOA dns.$DOMAIN. root.$DOMAIN. (
	2005092701 ; serial
	3600       ; dummy value
	900        ; dummy value
	604800     ; dummy value
	86400      ; dummy value
)
	IN	NS	dns.$DOMAIN.
EOD
}

setup_4()
{
	printf "dns\tIN\tA\t$net.1\n" >> ltp-ns.zone
	local id=$MIN_ID
	while [ $id -le $MAX_ID ]; do
		printf "node$id\tIN\tA\t$net.$id\n" >> ltp-ns.zone
		id=$(($id + 1))
	done

	# reverse zone file
	cat << EOD > ltp-ns.rev
\$TTL 10
@	IN	SOA $DOMAIN. root.$DOMAIN. (
	2005092701 ; serial
	3600       ; dummy value
	900        ; dummy value
	604800     ; dummy value
	86400      ; dummy value
)
        IN      NS      dns.$DOMAIN.
EOD

	id=$MIN_ID
	while [ $id -le $MAX_ID ]; do
		printf "$id\tIN\tPTR\tnode$id.$DOMAIN.\n" >> ltp-ns.rev
		id=$(($id + 1))
	done
}

setup_6()
{
	printf "dns\tIN\tAAAA\t$net::1\n" >> ltp-ns.zone
	local id=$MIN_ID
	while [ $id -le $MAX_ID ]; do
		printf "node$id\tIN\tAAAA\t$net::%x\n" $id >> ltp-ns.zone
		id=$(($id + 1))
	done

	# reverse zone file
	cat << EOD > ltp-ns.rev
\$TTL 10
@	IN	SOA $DOMAIN. root.$DOMAIN. (
	2005092701 ; serial
	3600       ; dummy value
	900        ; dummy value
	604800     ; dummy value
	86400      ; dummy value
)
        IN      NS      dns.$DOMAIN.
EOD

	id=$MIN_ID
	local rev_ip="0.0.0.0.0.0.0.0.0.0.0.0.0.0"
	while [ $id -le $MAX_ID ]; do
		printf "%x.%x.$rev_ip\tIN\tPTR\tnode$id.$DOMAIN.\n" \
			$(($id % 16)) $(($id / 16)) >> ltp-ns.rev
		id=$(($id + 1))
	done
}

start_named()
{
	chmod 770 .
	chmod 660 ./*

	port=$(tst_get_unused_port ipv${TST_IPVER} dgram)

	tst_resm TINFO "Start named daemon, port $port"
	named -$TST_IPVER -c named.conf -p $port || \
		tst_brkm TBROK "Failed to run named daemon"

	# Make sure named.pid is created.
	while true ; do
		test -s named.pid && break
		tst_sleep 100ms
	done
}

test01()
{
	tst_resm TINFO "Handling name lookup queries '$NS_TIMES' times"

	tst_rhost_run -s -c "dns-stress01-rmt.sh $TST_IPVER $(tst_ipaddr) $port \
		$DOMAIN $MIN_ID $MAX_ID $NS_TIMES"

	tst_resm TPASS "Test is finished successfully"
}

test02()
{
	tst_resm TINFO "Handling reverse lookup queries '$NS_TIMES' times"

	tst_rhost_run -s -c "dns-stress02-rmt.sh $TST_IPVER $(tst_ipaddr) $port $net \
		$MIN_ID $MAX_ID $NS_TIMES"

	tst_resm TPASS "Test is finished successfully"
}

. tst_net.sh
common_setup
setup_$TST_IPVER
start_named
test01
test02
tst_exit
