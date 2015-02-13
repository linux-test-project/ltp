#!/bin/sh

TST_TOTAL=1
TCID="network_settings"
. test_net.sh

# Network Test Parameters
#
# ---***** THESE MUST BE SET FOR CORRECT OPERATION *****---

# Management Link
export RHOST=${RHOST:-""}
export PASSWD=${PASSWD:-""}

# Warning:
# Make sure to set valid interface names and IP addresses below.
# 'networktests.sh' expects that IP addresses are already added to interface.
#
# Please note, that for 'networktests.sh' tests, management link and
# test link can be the same.

# Test Links
# Set names for test interfaces, e.g. "eth0 eth1"
export LHOST_IFACES=${LHOST_IFACES:-"eth0"}
export RHOST_IFACES=${RHOST_IFACES:-"eth0"}

# Set corresponding HW addresses, e.g. "00:00:00:00:00:01 00:00:00:00:00:02"
export LHOST_HWADDRS=${LHOST_HWADDRS:-"$(tst_get_hwaddrs lhost)"}
export RHOST_HWADDRS=${RHOST_HWADDRS:-"$(tst_get_hwaddrs rhost)"}

# Set first three octets of the network address, default is '10.0.0'
export IPV4_NETWORK=${IPV4_NETWORK:-"10.0.0"}
# Set local host last octet, default is '2'
export LHOST_IPV4_HOST=${LHOST_IPV4_HOST:-"2"}
# Set remote host last octet, default is '1'
export RHOST_IPV4_HOST=${RHOST_IPV4_HOST:-"1"}
# Set the reverse of IPV4_NETWORK
export IPV4_NET_REV=${IPV4_NET_REV:-"0.0.10"}
# Set first three octets of the network address, default is 'fd00:1:1:1'
export IPV6_NETWORK=${IPV6_NETWORK:-"fd00:1:1:1"}
# Set local host last octet, default is '2'
export LHOST_IPV6_HOST=${LHOST_IPV6_HOST:-":2"}
# Set remote host last octet, default is '1'
export RHOST_IPV6_HOST=${RHOST_IPV6_HOST:-":1"}
# Reverse network portion of the IPv6 address
export IPV6_NET_REV=${IPV6_NET_REV:-"1.0.0.0.1.0.0.0.1.0.0.0.0.0.d.f"}
# Reverse host portion of the IPv6 address of the local host
export LHOST_IPV6_REV=${LHOST_IPV6_REV:-"2.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0"}
# Reverse host portion of the IPv6 address of the remote host
export RHOST_IPV6_REV=${RHOST_IPV6_REV:-"1.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0"}

# Networks that aren't reachable through the test links
export IPV4_NET16_UNUSED=${IPV4_NET16_UNUSED:-"10.23"}
export IPV6_NET32_UNUSED=${IPV6_NET32_UNUSED:-"fd00:23"}

export HTTP_DOWNLOAD_DIR=${HTTP_DOWNLOAD_DIR:-"/var/www/html"}
export FTP_DOWNLOAD_DIR=${FTP_DOWNLOAD_DIR:-"/var/ftp"}
export FTP_UPLOAD_DIR=${FTP_UPLOAD_DIR:-"/var/ftp/pub"}
export FTP_UPLOAD_URLDIR=${FTP_UPLOAD_URLDIR:-"pub"}

# More information about network parameters can be found
# in the following document: testcases/network/stress/README

# ---***************************************************---

# Don't use it in new tests, use tst_rhost_run() from test_net.sh instead.
export LTP_RSH=${LTP_RSH:-"rsh -n"}

export TMPDIR=/tmp/netpan-$$
mkdir -p $TMPDIR
CMDFILE=${TMPDIR}/network.tests
VERBOSE="no"

export LTPROOT=${LTPROOT:-"$PWD"}
echo $LTPROOT | grep testscripts > /dev/null 2>&1
if [ $? -eq 0 ]; then
	cd ..
	export LTPROOT=${PWD}
fi

export PATH="${PATH}:${LTPROOT}/testcases/bin"

# Reset variables.
# Don't break the tests which are using 'testcases/lib/cmdlib.sh'
export TCID=
export TST_LIB_LOADED=
