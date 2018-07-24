#!/bin/sh
# Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001
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
# along with this program. If not, see <http://www.gnu.org/licenses/>.

TCID=xinetd
TST_TOTAL=2
TST_CLEANUP="cleanup"

. daemonlib.sh
TST_USE_LEGACY_API=1
. tst_net.sh

setup()
{
	[ -f "/usr/lib/systemd/system/telnet.socket" ] && \
		tst_brkm TCONF "xinetd doesn't manage telnet"

	tst_tmpdir

	tst_test_cmds xinetd diff telnet

	check_addr="127.0.0.1"
	ip a | grep -q inet6 && check_addr="$check_addr ::1"

	# Create custom xinetd.conf file.
	# tst_xinetd.conf.1 config file has telnet service disabled.
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

	# create a backup of the original xinetd.conf file.
	ROD mv /etc/xinetd.conf xinetd.conf.orig
}

cleanup()
{
	# restore the original xinetd.conf if a back up exits.
	[ -f xinetd.conf.orig ] && \
		mv xinetd.conf.orig /etc/xinetd.conf

	# restoring original services
	restart_daemon xinetd

	tst_rmdir
}

restart_xinetd()
{
	tst_resm TINFO "restart xinetd"
	# restart xinetd to re-start the services
	restart_daemon xinetd > tst_xinetd.out 2>&1
	if [ $? -ne 0 ]; then
		cat tst_xinetd.out
		tst_brkm TBROK "unable to restart service with telnet disabled"
	fi

	# even if xinetd restart has zero exit value,
	# make certain there was no failure.
	grep -qi "fail" tst_xinetd.out && \
		tst_brkm TBROK "xinetd failed to restart"
}

# Description:  - Test that xinetd reads the configuration file and starts or
#                 stops services.
#               - restart xinetd with configuration file with telnet disabled.
#               - telnet to locahost should fail.
#               - restart xinetd with configuration file with telnet enabled.
#               - telnet to locahost should work.
test01()
{
	tst_resm TINFO "install the new config file with telnet disabled"
	ROD mv tst_xinetd.conf.1 /etc/xinetd.conf
	restart_xinetd

	for a in $check_addr; do
		echo '' | telnet $a 2>&1 | grep -qi \
			"telnet: connect to address $a: Connection refused"
		[ $? -ne 0 ] && \
			tst_brkm TFAIL "not expected output for 'telnet $a'"
	done

	tst_resm TPASS "expected output with telnet disabled"

	tst_resm TINFO "install the xinetd config file with telnet enabled"
	ROD mv tst_xinetd.conf.2 /etc/xinetd.conf
	restart_xinetd

	for a in $check_addr; do
		echo '' | telnet $a 2>&1 | grep -qi \
			"Connection closed by foreign host"
		[ $? -ne 0 ] && \
			tst_brkm TFAIL "not expected output for 'telnet $a'"
	done

	tst_resm TPASS "expected output with telnet enabled"
}

setup

test01

tst_exit
