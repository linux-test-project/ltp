#!/bin/sh
# Copyright (c) 2017 Oracle and/or its affiliates. All Rights Reserved.
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

. test.sh

TST_CLEANUP=tst_rmdir

dns_check_answer()
{
	local fname="$1"

	if ! grep -q 'status: NOERROR' $fname; then
		cat $fname
		tst_brkm TFAIL "unexpected query status"
	fi
	if ! grep -q 'QUERY: 1, ANSWER: 1' $fname; then
		cat $fname
		tst_brkm TFAIL "unexpected number of query/answer"
	fi
}

dns_check_send_requests()
{
	[ $num -eq $connect_quantity ] && return
	tst_brkm TFAIL "some requests failed: $num/$connect_quantity"
}

tst_tmpdir
