#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines Corp., 2005
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

TST_CLEANUP="cleanup"
TST_SETUP="setup"
TST_TESTFUNC="test"
TST_CNT=3
TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="sshd ssh ssh-keygen od pkill pgrep"


# SSH config file on the remote host
RHOST_SSH_CONF=
# SSH command to connect from the remote host to the test host
RHOST_SSH=
# Processes started on the remote host, killed at cleanup
RHOST_PIDS=
# Netstress process started on the test host, killed at cleanup
NETSTRESS_PID=

cleanup()
{
	local pids

	# Stop the ssh daemon
	[ -s sshd.pid ] && kill $(cat sshd.pid)
	[ -n "$NETSTRESS_PID" ] && kill -INT $NETSTRESS_PID >/dev/null 2>&1

	[ -n "$RHOST_PIDS" ] && tst_rhost_run -c "kill $RHOST_PIDS" >/dev/null 2>&1

	# Kill all remaining ssh processes
	[ -n "$RHOST_SSH_CONF" ] && tst_rhost_run -c "pkill -f '^ssh $RHOST_SSH_CONF'"
}

setup()
{
	local port rc


	port=$(tst_rhost_run -c "tst_get_unused_port ipv${TST_IPVER} stream")

	cat << EOF > sshd_config
Port $port
ListenAddress $(tst_ipaddr)
PermitRootLogin yes
AuthorizedKeysFile $TST_TMPDIR/authorized_keys
PasswordAuthentication no
AllowTcpForwarding yes
TCPKeepAlive yes
UseDNS no
StrictModes no
PidFile $TST_TMPDIR/sshd.pid
HostKey $TST_TMPDIR/ssh_host_rsa_key
HostKey $TST_TMPDIR/ssh_host_ecdsa_key
HostKey $TST_TMPDIR/ssh_host_ed25519_key
EOF

	ssh-keygen -q -N "" -t rsa -b 4096 -f $TST_TMPDIR/ssh_host_rsa_key
	ssh-keygen -q -N "" -t ecdsa -f $TST_TMPDIR/ssh_host_ecdsa_key
	ssh-keygen -q -N "" -t ed25519 -f $TST_TMPDIR/ssh_host_ed25519_key

	tst_res TINFO "Generate configuration file and key at the remote host"
	tst_rhost_run -s -c "ssh-keygen -t rsa -N \"\" -f $TST_TMPDIR/id_rsa \
		>/dev/null"

	RHOST_SSH_CONF=$TST_TMPDIR/ssh_config

	tst_rhost_run -s -c "printf \"\
Port $port\n\
StrictHostKeyChecking no\n\
PasswordAuthentication no\n\
ExitOnForwardFailure yes\n\
UserKnownHostsFile $TST_TMPDIR/known_hosts\n\
IdentityFile $TST_TMPDIR/id_rsa\n\" > $RHOST_SSH_CONF"

	tst_res TINFO "Generate authorized_keys"
	tst_rhost_run -c "cat ${TST_TMPDIR}/id_rsa.pub" > authorized_keys

	tst_res TINFO "restore context of authorized_keys"
	rc=$(command -v restorecon)
	[ -n "$rc" ] && $rc authorized_keys

	$(command -v sshd) -f $TST_TMPDIR/sshd_config || \
		tst_brk TBROK "Failed to run sshd daemon"

	RHOST_SSH="ssh -$TST_IPVER -F $RHOST_SSH_CONF $(tst_ipaddr)"
}

test_ssh_connectivity()
{
	tst_rhost_run -c "$RHOST_SSH 'true >/dev/null 2>&1' >/dev/null"
	[ $? -ne 0 ] && tst_brk TFAIL "SSH not reachable"
}

test1()
{
	local num all_conn pid

	tst_res TINFO "Verify SSH connectivity over IPv$TST_IPVER is not broken after creating many SSH sessions"

	test_ssh_connectivity

	RHOST_PIDS=
	num=0
	while [ $num -lt $CONNECTION_TOTAL ]; do
		pid=$(tst_rhost_run -c "$RHOST_SSH -N </dev/null 1>/dev/null 2>&1 \
			& echo \$!")
		RHOST_PIDS="$RHOST_PIDS $pid"
		num=$(($num + 1))
	done

	tst_res TINFO "Killing all ssh sessions"
	num=0
	for pid in $RHOST_PIDS; do
		tst_rhost_run -c "kill $pid" >/dev/null
		[ $? -ne 0 ] && num=$((num + 1))
	done

	[ $num -ne 0 ] && tst_brk TFAIL "$num ssh processes died unexpectedly during execution"

	test_ssh_connectivity

	tst_res TPASS "Test finished successfully"
}

test2()
{
	local start_epoc pids total_connections elapse_epoc new_pids
	local ssh_num wait_sec login_sec

	tst_res TINFO "Verify SSH connectivity over IPv$TST_IPVER is not broken after logging in/out by many clients asynchronously"

	test_ssh_connectivity

	start_epoc=$(date +%s)
	RHOST_PIDS=
	total_connections=0
	while true; do
		# Exit after the specified time has elapsed.
		elapse_epoc=$(( $(date +%s) - $start_epoc))
		[ $elapse_epoc -ge $NS_DURATION ] && break

		new_pids=
		for pid in $RHOST_PIDS; do
			if tst_rhost_run -c "kill -0 $pid" >/dev/null; then
				new_pids="$new_pids $pid"
			fi
		done
		RHOST_PIDS="$new_pids"

		# Do not make ssh connection over the specified quantity
		ssh_num=$(echo "$pids" | wc -w)
		if [ $ssh_num -ge $CONNECTION_TOTAL ]; then
			tst_res TINFO "Max connections reached"
			tst_sleep 1
			continue
		fi

		# specified wait time and login time
		wait_sec=$(( $(od -A n -d -N 1 /dev/urandom) * 3 / 255 ))
		login_sec=$(( $(od -A n -d -N 1 /dev/urandom) * 10 / 255 ))

		# Login to the server
		pid=$(tst_rhost_run -c "( \
			  sleep $wait_sec && $RHOST_SSH -l root \"sleep $login_sec\" \
			) </dev/null 1>/dev/null 2>&1 & echo \$!"
		)
		RHOST_PIDS="$RHOST_PIDS $pid"
		total_connections=$(( total_connections + 1 ))
	done

	tst_res TINFO "Waiting for all connections to terminate"
	while [ -n "$RHOST_PIDS" ]; do
		tst_sleep 1
		new_pids=
		for pid in $RHOST_PIDS; do
			if tst_rhost_run -c "kill -0 $pid" >/dev/null 2>&1; then
				new_pids="$new_pids $pid"
			fi
		done
		RHOST_PIDS="$new_pids"
	done

	test_ssh_connectivity

	tst_res TPASS "Test finished successfully ($total_connections connections)"
}

test3()
{
	local port lport localhost rhost ret
	tst_res TINFO "Verify SSH connectivity over IPv$TST_IPVER is not broken after forwarding TCP traffic"

	localhost="127.0.0.1"
	rhost="$(tst_ipaddr)"
	if [ "$TST_IPVER" = "6" ]; then
		localhost="::1"
		rhost="[$(tst_ipaddr)]"
	fi

	test_ssh_connectivity

	# Get an ssh forwarding port
	lport=$(tst_rhost_run -c "tst_get_unused_port ipv${TST_IPVER} stream")

	# Start a tcp server
	netstress -R 3 -B $TST_TMPDIR >/dev/null 2>&1
	[ $? -ne 0 ] && tst_brk TBROK "Unable to start netstress server"
	NETSTRESS_PID=$(pgrep -f "^netstress .*$TST_TMPDIR")
	port=$(cat netstress_port)

	# Setup an ssh tunnel from the remote host to testhost
	tst_rhost_run -c "$RHOST_SSH -f -N -L $lport:$rhost:$port </dev/null >/dev/null 2>&1"
	[ "$?" -ne 0 ] && tst_brk TFAIL "Failed to create an SSH session with port forwarding"
	RHOST_PIDS=$(tst_rhost_run -c "pgrep -f '^ssh .*$lport:$rhost:$port'")

	# Start the TCP traffic clients
	tst_rhost_run -s -c "netstress -r $NS_TIMES -l -H $localhost -g $lport > /dev/null"

	tst_rhost_run -c "kill $RHOST_PIDS >/dev/null 2>&1"

	test_ssh_connectivity

	tst_res TPASS "Test finished successfully"
}

. tst_net.sh
tst_run
