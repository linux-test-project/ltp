#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001

TST_SETUP="setup"
TST_CLEANUP="cleanup"
TST_NEEDS_CMDS="diff telnet in.telnetd xinetd"
TST_NEEDS_TMPDIR=1
TST_TESTFUNC="do_test"
TST_CNT=2

. daemonlib.sh

setup()
{
	[ -f "/usr/lib/systemd/system/telnet.socket" ] && \
		tst_brk TCONF "xinetd doesn't manage telnet"

	check_addr="127.0.0.1"
	ip a | grep -q inet6 && check_addr="$check_addr ::1"

	cat > tst_xinetd.conf.1 <<-EOF
defaults
{
    instances      = 25
    log_type       = FILE /var/log/servicelog
    log_on_success = HOST PID
    log_on_failure = HOST
    disabled       = telnet
}
EOF

	cat > tst_xinetd.conf.2 <<-EOF
defaults
{
    instances      = 25
    log_type       = FILE /var/log/servicelog
    log_on_success = HOST PID
    log_on_failure = HOST
    # disabled       = telnet
}

service telnet
{
    socket_type     = stream
    protocol        = tcp
    wait            = no
    user            = root
    server          = /usr/sbin/in.telnetd
    server_args     = -n
    no_access       =
    flags           = IPv6
}
EOF
	ROD mv /etc/xinetd.conf xinetd.conf.orig
}

cleanup()
{
	[ -f xinetd.conf.orig ] && \
		mv xinetd.conf.orig /etc/xinetd.conf

	restart_daemon xinetd
}

restart_xinetd()
{
	tst_res TINFO "restart xinetd"
	restart_daemon xinetd > tst_xinetd.out 2>&1
	if [ $? -ne 0 ]; then
		cat tst_xinetd.out
		tst_brk TBROK "unable to restart service with telnet disabled"
	fi

	grep -qi "fail" tst_xinetd.out && \
		tst_brk TBROK "xinetd failed to restart"
}

xinetd_test()
{
	local cnt=$1
	local desc="$2"
	local pattern="$3"
	local a p

	tst_res TINFO "install the new config file with telnet $desc"
	ROD mv tst_xinetd.conf.$cnt /etc/xinetd.conf
	restart_xinetd

	for a in $check_addr; do
		p=$(echo $pattern | sed "s/ADDR/$a/")
		if ! echo '' | telnet $a 2>&1 | grep -qiE "$p"; then
			tst_res TFAIL "not expected output for 'telnet $a'"
			return
		fi
	done
	tst_res TPASS "expected output with telnet $desc"
}

do_test()
{
	case $1 in
	1) xinetd_test $1 "disabled" \
			"telnet: (connect to address ADDR|Unable to connect to remote host): Connection refused";;
	2) xinetd_test $1 "enabled" \
			"Connection closed by foreign host";;
	esac
}

. tst_net.sh
tst_run
