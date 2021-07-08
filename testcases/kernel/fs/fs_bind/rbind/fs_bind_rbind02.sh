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

SETS_DEFAULTS="${TCID=test02} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

tst_resm TINFO "***************TEST02***************"
tst_resm TINFO "rbind: shared child to private parent."
tst_resm TINFO "************************************"

. "${FS_BIND_ROOT}/bin/setup" || (tst_resm TWARN "Setup of rbind/test02 failed" && tst_exit)
export result=0


trap 'ERR=$? ; ERR_MSG="caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"; break' ERR

while /bin/true ; do
	# This loop is for error recovery purposes only

	"${FS_BIND_ROOT}/bin/makedir" share parent1
	"${FS_BIND_ROOT}/bin/makedir" priv parent2
	"${FS_BIND_ROOT}/bin/makedir" share share1
	"${FS_BIND_ROOT}/bin/makedir" share parent1/child1

	mount --rbind "$disk1" parent1/child1

	mkdir parent2/child2

	mount --rbind parent1/child1 share1
	mount --rbind parent1/child1 parent2/child2

	check parent1/child1 share1
	check parent1/child1 parent2/child2

	mount --rbind "$disk2" parent1/child1/a

	check parent1/child1/a parent2/child2/a share1/a

	mount --rbind "$disk3" parent2/child2/b

	check parent1/child1/b parent2/child2/b share1/b

	break
done
trap 'ERR=$? ; tst_resm TWARN "rbind/test02: caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"' ERR
if [ -n "${ERR_MSG}" ]; then
	tst_resm TWARN "rbind/test02: ${ERR_MSG}"
	ERR_MSG=""
	result=$ERR
fi
trap '' ERR
{
	umount parent2/child2/b
	umount parent1/child1/a
	umount parent2/child2
	umount share1
	umount parent1/child1
	umount parent1/child1
	umount share1
	umount parent2
	umount parent1

	rm -rf parent* share*
	cleanup
} >& /dev/null

if [ $result -ne 0 ]
then
	tst_resm TFAIL "rbind/test02: FAILED: rbind: shared child to private parent."
	exit 1
else
	tst_resm TPASS "rbind/test02: PASSED"
	exit 0
fi


tst_exit
