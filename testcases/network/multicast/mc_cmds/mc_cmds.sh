#! /bin/sh

# Copyright (c) 2015-2017 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2000
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
# TEST DESCRIPTION :
#     To determine the stability of the IP Multicast product
#     and to verify the accuracy and usability of IP Multicast
#     related publications associated with changes and/or
#     additions to command level interfaces for this implementations
#     of IP Multicast.
#
# Robbie Williamson (robbiew@us.ibm.com)

TCID=mc_cmds
TST_TOTAL=1

TST_USE_LEGACY_API=1
. tst_net.sh

knob="net.ipv4.icmp_echo_ignore_broadcasts"
knob_changed=

setup()
{
	val=$(sysctl -n $knob)
	if [ "$val" -ne 0 ]; then
		ROD sysctl -q ${knob}=0
		knob_changed=1
	fi
	tst_tmpdir
}

do_test()
{
	local ip_fixed_version=170220

	ip addr show $(tst_iface) | grep -q 'MULTICAST' || \
		tst_brkm TFAIL "Multicast not listed for $(tst_iface)"

	ip maddr show $(tst_iface) | grep -q '224.0.0.1'
	if [ $? -ne 0 ]; then
		[ `ip -V | cut -d's' -f3` -lt $ip_fixed_version ] && \
			tst_resm TINFO "'ip maddr show $(tst_iface)' failed"\
					"(caused by old ip version, fixed in"\
					"$ip_fixed_version)" || \
			tst_resm TWARN "'ip maddr show $(tst_iface)' failed"
		tst_resm TINFO "parsing 'ip maddr show' command"
		ip maddr show | sed -ne "/\s$(tst_iface)/,/^[0-9]/p" | \
			grep -q 224.0.0.1 || \
			tst_brkm TFAIL "$(tst_iface) not joined 224.0.0.1"
	fi

	tst_resm TINFO "Ping all-host-groups over specified interface"
	ping -c2 -I $(tst_ipaddr) 224.0.0.1 > ping_out.log
	if [ $? -ne 0 ]; then
		tst_resm TINFO "Trying to ping with $(tst_iface)"\
			       "with the -I option instead of IP address"
		ping -c2 -I $(tst_iface) 224.0.0.1 > ping_out.log || \
			tst_brkm TFAIL "No response from MC hosts to ping -c2 "\
				       "-I $(tst_ipaddr) 224.0.0.1"
	fi

	grep -q $(tst_ipaddr) ping_out.log
	if [ $? -ne 0 ]; then
		cat ping_out.log
		tst_brkm TFAIL "Local host did not respond to ping -c2 "\
			       "-I $(tst_iface) 224.0.0.1"
	fi

	tst_resm TPASS "Test Successful"
	tst_exit
}

do_cleanup()
{
	[ "$knob_changed" ] && sysctl -q ${knob}=1
	tst_rmdir
}

setup
TST_CLEANUP=do_cleanup

do_test
