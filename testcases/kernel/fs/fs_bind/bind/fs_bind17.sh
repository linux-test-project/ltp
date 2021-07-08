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

SETS_DEFAULTS="${TCID=test17} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

result=0

tst_resm TINFO "***************TEST17***************"
tst_resm TINFO "bind: shared subtree with shared child to shared subtree."
tst_resm TINFO "************************************"

. "${FS_BIND_ROOT}/bin/setup" || (tst_resm TWARN "Setup of bind/test17 failed" && tst_exit)
export result=0



trap 'ERR=$? ; ERR_MSG="caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"; break' ERR

while /bin/true ; do
	# This loop is for error recovery purposes only



	"${FS_BIND_ROOT}/bin/makedir" share parent1
	"${FS_BIND_ROOT}/bin/makedir" share parent2
	"${FS_BIND_ROOT}/bin/makedir" share share1

	mount --bind "$disk1" share1
	mount --bind share1 parent1

	mount --bind parent1 parent2
	"${FS_BIND_ROOT}/bin/makedir" share parent1/child1

	check parent1 share1 parent2
	mount --bind "$disk2" parent1/child1
	check parent1/child1 parent2/child1
	check parent1/child1 share1/child1
	mount --bind "$disk3" parent2/a

	check parent1/a parent2/a share1/a

	mount --bind "$disk4" share1/b

	check parent1/b parent2/b share1/b

	break
done
trap 'ERR=$? ; tst_resm TWARN "bind/test17: caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"' ERR
if [ -n "${ERR_MSG}" ]; then
	tst_resm TWARN "bind/test17: ${ERR_MSG}"
	ERR_MSG=""
	result=$ERR
fi
trap '' ERR
{
	umount share1/b
	umount parent2/a
	umount parent1/child1
	umount parent1/child1
	umount parent2
	umount parent1
	umount share1
	umount parent2
	umount parent1
	umount share1

	rm -rf parent* share*
	cleanup
} >& /dev/null
if [ $result -ne 0 ]
then
	tst_resm TFAIL "bind/test17: FAILED: bind: shared subtree with shared child to shared subtree."
	exit 1
else
	tst_resm TPASS "bind/test17: PASSED"
	exit 0
fi
tst_exit
