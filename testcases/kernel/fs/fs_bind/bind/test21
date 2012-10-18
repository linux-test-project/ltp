#!/bin/bash

#
# Copyright (c) International Business Machines  Corp., 2005
# Author: Avantika Mathur (mathurav@us.ibm.com)
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

SETS_DEFAULTS="${TCID=test21} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

tst_resm TINFO "***************TEST21***************"
tst_resm TINFO "bind: multi-level slave p-nodes."
tst_resm TINFO "************************************"

. "${FS_BIND_ROOT}/bin/setup" || (tst_resm TWARN "Setup of bind/test21 failed" && tst_exit)
export result=0




trap 'ERR=$? ; ERR_MSG="caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"; break' ERR

while /bin/true ; do
	# This loop is for error recovery purposes only



	"${FS_BIND_ROOT}/bin/makedir" share dir1

	mkdir dir1/x dir2 dir3 dir4

	mount --bind dir1 dir2
	"${FS_BIND_ROOT}/bin/makedir" slave dir2
	"${FS_BIND_ROOT}/bin/makedir" -n share dir2

	mount --bind dir2 dir3
	"${FS_BIND_ROOT}/bin/makedir" slave dir3
	"${FS_BIND_ROOT}/bin/makedir" -n share dir3

	mount --bind dir3 dir4
	"${FS_BIND_ROOT}/bin/makedir" slave dir4

	mount --bind "$disk1" dir1/x

	check dir1/x dir2/x dir3/x dir4/x

	mount --bind "$disk2" dir2/x/a
	check -n dir1/x/a dir2/x/a
	check dir2/x/a dir3/x/a dir4/x/a

	mount --bind "$disk3" dir3/x/b
	check -n dir1/x/b dir3/x/b
	check -n dir2/x/b dir3/x/b
	check dir3/x/b dir4/x/b

	mount --bind "$disk4" dir4/x/c
	check -n dir1/x/c dir4/x/c
	check -n dir2/x/c dir4/x/c
	check -n dir3/x/c dir4/x/c

	break
done
trap 'ERR=$? ; tst_resm TWARN "bind/test21: caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"' ERR
if [ -n "${ERR_MSG}" ]; then
	tst_resm TWARN "bind/test21: ${ERR_MSG}"
	ERR_MSG=""
	result=$ERR
fi
trap '' ERR
{
	umount dir2/x/a
	umount dir3/x/b
	umount dir4/x/c
	umount dir1/x
	umount dir1
	umount dir1
	umount dir2
	umount dir3
	umount dir4

	rm -rf dir*

	cleanup
} >& /dev/null

if [ $result -ne 0 ]
then
        tst_resm TFAIL "bind/test21: FAILED: bind: multi-level slave p-nodes."
        exit 1
else
        tst_resm TPASS "bind/test21: PASSED"
        exit 0
fi



tst_exit
