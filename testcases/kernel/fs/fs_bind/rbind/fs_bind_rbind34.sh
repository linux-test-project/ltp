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

SETS_DEFAULTS="${TCID=test34} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

tst_resm TINFO "***************TEST34***************"
tst_resm TINFO "rbind: rbind within same tree - root to child, child is shared "
tst_resm TINFO "************************************"

. "${FS_BIND_ROOT}/bin/setup" || (tst_resm TWARN "Setup of rbind/test34 failed" && tst_exit)
export result=0



trap 'ERR=$? ; ERR_MSG="caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"; break' ERR

while /bin/true ; do
	# This loop is for error recovery purposes only



	"${FS_BIND_ROOT}/bin/makedir" share parent
	"${FS_BIND_ROOT}/bin/makedir" share parent/child1
	"${FS_BIND_ROOT}/bin/makedir" share parent/child2

	mount --rbind "$disk3" parent/child1

	mount --rbind parent parent/child2/
	check parent parent/child2/
	check parent/child1 parent/child2/child1

	break
done
trap 'ERR=$? ; tst_resm TWARN "rbind/test34: caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"' ERR
if [ -n "${ERR_MSG}" ]; then
	tst_resm TWARN "rbind/test34: ${ERR_MSG}"
	ERR_MSG=""
	result=$ERR
fi
trap '' ERR
{
	umount parent/child2/child1
	check parent/child1 parent/child2/child1

	umount parent/child1
	check parent/child1 parent/child2/child1

	mount --rbind "$disk4" parent/child2/child1
	check parent/child1 parent/child2/child1

	umount parent/child1
	check parent/child1 parent/child2/child1

	umount parent/child2/child2
	umount parent/child2
	umount parent/child2
	umount parent

	rm -rf parent

	cleanup
} >& /dev/null
if [ $result -ne 0 ]
then
	tst_resm TFAIL "rbind/test34: FAILED: rbind: rbind within same tree - root to child, child is shared "
	exit 1
else
	tst_resm TPASS "rbind/test34: PASSED"
	exit 0
fi
tst_exit
