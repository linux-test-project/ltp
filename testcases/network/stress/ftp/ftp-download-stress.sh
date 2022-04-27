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

TCID=ftp-download-stress
TST_TOTAL=2
TST_CLEANUP="cleanup"

TST_USE_LEGACY_API=1

# Big file size to upload/download in ftp tests (byte)
DOWNLOAD_BIGFILESIZE=${DOWNLOAD_BIGFILESIZE:-2147483647}
# Regular file size to upload/download in ftp tests (byte)
DOWNLOAD_REGFILESIZE=${DOWNLOAD_REGFILESIZE:-1048576}

cleanup()
{
	rm -f $FTP_DOWNLOAD_DIR/ftp_file*
	rm -f $FTP_DOWNLOAD_DIR/ftp_reg_file*
	tst_rmdir
	pkill vsftpd
}

setup()
{
	tst_require_root
	tst_require_cmds pkill vsftpd
	tst_tmpdir

	tst_resm TINFO "run FTP over IPv$TST_IPVER"

	trap "tst_brkm TBROK 'test interrupted'" INT

	[ "$FTP_DOWNLOAD_DIR" ] || \
		tst_brkm TCONF "Start ftp server and set FTP_DOWNLOAD_DIR var"

	tst_resm TINFO "restart vsftpd with custom options"
	pkill vsftpd
	touch vsftpd.conf

	local ip_opt="-olisten=YES -olisten_ipv6=NO"
	[ $TST_IPV6 ] && ip_opt="-olisten=NO -olisten_ipv6=YES"

	vsftpd $ip_opt -oanonymous_enable=YES vsftpd.conf
}

test01()
{
	tst_resm TINFO "download file with size '$DOWNLOAD_BIGFILESIZE'"

	create_file $FTP_DOWNLOAD_DIR/ftp_file $DOWNLOAD_BIGFILESIZE || \
		tst_resm TBROK "Failed to create test file"

	# Run the script at the remote host
	tst_rhost_run -s -c "ftp-download-stress01-rmt.sh \
		$(tst_ipaddr) ftp_file $DOWNLOAD_BIGFILESIZE"

	tst_resm TPASS "Test is finished successfully"
}

test02()
{
	tst_resm TINFO "Download data asynchronously in $NS_DURATION sec"

	create_file $FTP_DOWNLOAD_DIR/ftp_reg_file $DOWNLOAD_REGFILESIZE || \
		tst_resm TBROK "Failed to create test file"

	tst_rhost_run -s -c "ftp-download-stress02-rmt.sh $(tst_ipaddr) \
		ftp_reg_file $DOWNLOAD_REGFILESIZE \
		$NS_DURATION $CONNECTION_TOTAL"

	tst_resm TPASS "Test is finished successfully"
}

. tst_net.sh
setup
test01
test02
tst_exit
