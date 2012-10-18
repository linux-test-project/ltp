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
tst_resm TINFO "move: shared tree within a tree it is bound to."
tst_resm TINFO "************************************"

. "${FS_BIND_ROOT}/bin/setup" || (tst_resm TWARN "Setup of move/test21 failed" && tst_exit)
export result=0



trap 'ERR=$? ; ERR_MSG="caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"; break' ERR

while /bin/true ; do
	# This loop is for error recovery purposes only


	mkdir parent1 parent2
	mount --bind "$disk1" parent1
	mount --make-rshared parent1 > /dev/null 2>&1 || "${FS_BIND_ROOT}/bin/smount" parent1 rshared
	mount --bind parent1 parent2

	check parent1 parent2

	mount --move parent1 parent2/a

	check parent2 parent2/a parent2/a/a

	mount --bind "$disk2" parent2/b

	check parent2/b parent2/a/b parent2/a/a/b

	mount --bind "$disk3" parent2/a/c

	check parent2/c parent2/a/c parent2/a/a/c

	break
done
trap 'ERR=$? ; tst_resm TWARN "move/test21: caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"' ERR
if [ -n "${ERR_MSG}" ]; then
	tst_resm TWARN "move/test21: ${ERR_MSG}"
	ERR_MSG=""
	result=$ERR
fi
trap '' ERR
{
	umount parent2/a/a/c

	check parent2/c parent2/a/c parent2/a/a/c

	umount parent2/b
	umount parent2/a/a
	umount parent2/a
	umount parent2
	umount parent1

	rm -rf dir parent* share*
	cleanup
} >& /dev/null
if [ $result -ne 0 ]
then
	tst_resm TFAIL "move/test21: FAILED: move: shared tree within a tree it is bound to."
	exit 1
else
	tst_resm TPASS "move/test21: PASSED"
	exit 0
fi
tst_exit
