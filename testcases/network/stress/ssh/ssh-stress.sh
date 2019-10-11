#!/bin/sh

# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
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
#

TCID=ssh-stress
TST_TOTAL=3
TST_CLEANUP="cleanup"

TST_USE_LEGACY_API=1
. tst_net.sh

# Temporary directory to store sshd setting or ssh key
# Note: ssh doesn't work when those directory is under /tmp.
TMPDIR="/root"

cleanup()
{
	# Stop the ssh daemon
	test -s sshd.pid && kill $(cat sshd.pid)
	pkill 'netstress$'
	tst_rmdir
	[ "$rtmpdir" ] && tst_rhost_run -c "rm -rf $rtmpdir"
	TMPDIR=
}

setup()
{
	trap "tst_brkm TBROK 'test interrupted'" INT

	tst_require_root
	tst_require_cmds pkill sshd ssh od

	# Get the sshd command with absolute path
	SSHD=$(which sshd)
	test "$SSHD" || tst_brkm TBROK "sshd daemon is not found"

	check_icmpv${TST_IPVER}_connectivity $(tst_iface) $(tst_ipaddr rhost) || \
		tst_brkm TBROK "Failed to ping to $(tst_ipaddr rhost)"

	port=$(tst_rhost_run -c "tst_get_unused_port ipv${TST_IPVER} stream")

	tst_tmpdir

	tmpdir=$TST_TMPDIR

	cat << EOD > $tmpdir/sshd_config
Port $port
ListenAddress $(tst_ipaddr)
PermitRootLogin yes
AuthorizedKeysFile $tmpdir/authorized_keys
PasswordAuthentication no
AllowTcpForwarding yes
TCPKeepAlive yes
UseDNS no
PidFile $tmpdir/sshd.pid
EOD

	$SSHD -f $tmpdir/sshd_config || \
		tst_brkm TBROK "Failed to run sshd daemon"

	tst_resm TINFO "Generate configuration file and key at the remote host"
	rtmpdir=$(tst_rhost_run -c "mktemp -d -p $TMPDIR")
	tst_rhost_run -s -c "ssh-keygen -t rsa -N \"\" -f $rtmpdir/id_rsa > /dev/null"

	rconfig=$rtmpdir/ssh_config

	tst_rhost_run -s -c "printf \"\
Port $port\n\
StrictHostKeyChecking no\n\
PasswordAuthentication no\n\
UserKnownHostsFile $rtmpdir/known_hosts\n\
IdentityFile $rtmpdir/id_rsa\n\" > $rconfig"

	tst_rhost_run -s -c "chmod 700 $rtmpdir; chmod 600 $rtmpdir/*"

	tst_resm TINFO "Generate authorized_keys"
	tst_rhost_run -c "cat ${rtmpdir}/id_rsa.pub" > $tmpdir/authorized_keys

	tst_resm TINFO "restore context of authorized_keys"
	local rc=$(which restorecon)
	test "$rc" && $rc $tmpdir/authorized_keys

	chmod 700 $tmpdir
	chmod 600 $tmpdir/*
}

test01()
{
	tst_resm TINFO "Creating '$CONNECTION_TOTAL' ssh sessions"

	tst_rhost_run -s -c "ssh-stress01-rmt.sh $TST_IPVER $(tst_ipaddr) \
		$rconfig $CONNECTION_TOTAL"

	tst_resm TPASS "Test is finished successfully"
}

test02()
{
	tst_resm TINFO "Log in/out by many clients asynchronously"
	tst_resm TINFO "'$CONNECTION_TOTAL' clients, time $NS_DURATION sec"

	tst_rhost_run -s -c "ssh-stress02-rmt.sh $TST_IPVER $(tst_ipaddr) \
		$rconfig $CONNECTION_TOTAL $NS_DURATION"

	tst_resm TPASS "Test is finished successfully"
}

test03()
{
	tst_resm TINFO "Forwarding TCP traffic with $NS_TIMES requests"

	# Run a TCP traffic server
	port=$(tst_get_unused_port ipv${TST_IPVER} stream)

	netstress -R 3 -g $port > tcp_server.log 2>&1 &

	tst_rhost_run -s -c "ssh-stress03-rmt.sh $TST_IPVER $(tst_ipaddr) \
		$rconfig $port $NS_TIMES"

	tst_resm TPASS "Test is finished successfully"
}

setup

test01
test02
test03

tst_exit
