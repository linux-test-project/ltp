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

TCID=http-stress
TST_TOTAL=2
TST_CLEANUP="cleanup"

TST_USE_LEGACY_API=1

cleanup()
{
	rm -f $HTTP_DOWNLOAD_DIR/http_file
}

setup()
{
	tst_require_root

	tst_resm TINFO "run over IPv$TST_IPVER"

	trap "tst_brkm TBROK 'test interrupted'" INT

	[ -d "$HTTP_DOWNLOAD_DIR" ] || \
		tst_brkm TCONF "Start server manually, set HTTP_DOWNLOAD_DIR"

	# As the apache with normal setting cannot handle the file whose size
	# is bigger than or equal to 2G byte. Therefore, the file size is
	# resetted into 2G - 1 byte."
	if [ $DOWNLOAD_BIGFILESIZE -gt 2147483647 ]; then
		tst_resm TINFO "Setting file size to 2G - 1 byte"
		DOWNLOAD_BIGFILESIZE=2147483647
	fi

	create_file $HTTP_DOWNLOAD_DIR/http_file $DOWNLOAD_BIGFILESIZE || \
		tst_resm TBROK "Failed to create test file"
}

test01()
{
	tst_resm TINFO "http client download test file"

	tst_rhost_run -s -c "http-stress01-rmt.sh $(tst_ipaddr) \
		http_file $DOWNLOAD_BIGFILESIZE"

	tst_resm TPASS "Test is finished successfully"
}

test02()
{
	tst_resm TINFO "clients request data asynchronously $NS_DURATION sec"

	tst_rhost_run -s -c "http-stress02-rmt.sh $(tst_ipaddr) http_file \
		$DOWNLOAD_BIGFILESIZE $NS_DURATION $CONNECTION_TOTAL"

	tst_resm TPASS "Test is finished successfully"
}

. tst_net.sh
setup
test01
test02
tst_exit
