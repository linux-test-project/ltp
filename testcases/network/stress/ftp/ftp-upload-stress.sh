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

TCID=ftp-upload-stress
TST_TOTAL=2
TST_CLEANUP="cleanup"

TST_USE_LEGACY_API=1

# Big file size to upload (byte)
UPLOAD_BIGFILESIZE=${UPLOAD_BIGFILESIZE:-2147483647}  # 2GB - 1
# Regular file size to upload(byte)
UPLOAD_REGFILESIZE=${UPLOAD_REGFILESIZE:-1024}     # 1K byte

cleanup()
{
	rm -f $FTP_UPLOAD_DIR/ftp_file*
	rm -f $FTP_UPLOAD_DIR/ftp_reg_file*
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

	[ -d "$FTP_UPLOAD_DIR" ] || \
		tst_brkm TCONF "Start ftp server and set FTP_UPLOAD_DIR var"

	chmod o+w $FTP_UPLOAD_DIR

	getenforce 2> /dev/null | grep -q Enforcing
	if [ $? -eq 0 ]; then
		tst_resm TINFO "configuring SELinux FTP parameters"
		tst_require_cmds chcon setsebool
		setsebool allow_ftpd_anon_write 1 || \
			tst_brkm TBROK "Failed to allow ftpd anonymous write"
		chcon -R -t public_content_rw_t $FTP_UPLOAD_DIR || \
			tst_brkm TBROK "Failed to apply public_content_rw_t"
	fi

	tst_resm TINFO "restart vsftpd with custom options"
	pkill vsftpd
	touch vsftpd.conf

	local ip_opt="-olisten=YES -olisten_ipv6=NO"
	[ $TST_IPV6 ] && ip_opt="-olisten=NO -olisten_ipv6=YES"

	upload_opt="-owrite_enable=YES -oanon_upload_enable=YES"

	vsftpd $ip_opt $upload_opt -oanonymous_enable=YES vsftpd.conf
}

test01()
{
	tst_resm TINFO "upload file with size '$UPLOAD_BIGFILESIZE'"

	# Run the script at the remote host
	tst_rhost_run -s -c "ftp-upload-stress01-rmt.sh $(tst_ipaddr) \
		$FTP_UPLOAD_URLDIR ftp_file $UPLOAD_BIGFILESIZE"

	tst_resm TPASS "Test is finished successfully"
}

test02()
{
	tst_resm TINFO "Upload data asynchronously in $NS_DURATION sec"

	tst_rhost_run -s -c "ftp-upload-stress02-rmt.sh \
		$(tst_ipaddr) $FTP_UPLOAD_URLDIR ftp_reg_file \
		$UPLOAD_REGFILESIZE $NS_DURATION $CONNECTION_TOTAL"

	tst_resm TPASS "Test is finished successfully"
}

. tst_net.sh
setup
test01
test02
tst_exit
