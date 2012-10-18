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

SETS_DEFAULTS="${TCID=test22} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

tst_resm TINFO "***************TEST22***************"
tst_resm TINFO "move: shared tree within a tree it is bound to - and then move to another share subtree"
tst_resm TINFO "************************************"

. "${FS_BIND_ROOT}/bin/setup" || (tst_resm TWARN "Setup of move/test22 failed" && tst_exit)
export result=0



trap 'ERR=$? ; ERR_MSG="caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"; break' ERR

while /bin/true ; do
	# This loop is for error recovery purposes only


	"${FS_BIND_ROOT}/bin/makedir" share parent1
	mkdir parent1/a parent2
	mount --bind parent1 parent2

	check parent1 parent2

	mount --move parent1 parent2/a

	check parent2 parent2/a parent2/a/a

	"${FS_BIND_ROOT}/bin/makedir" share tmp1
	mkdir tmp2 tmp1/1

	mount --bind tmp1 tmp2
	mount --move parent2  tmp1/1

	break
done
trap 'ERR=$? ; tst_resm TWARN "move/test22: caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"' ERR
if [ -n "${ERR_MSG}" ]; then
	tst_resm TWARN "move/test22: ${ERR_MSG}"
	ERR_MSG=""
	result=$ERR
fi
trap '' ERR
{
	umount tmp1/1/a/a
	umount tmp1/1/a
	umount tmp1/1
	umount tmp1
	umount tmp1
	umount tmp2

	rm -rf dir parent* tmp1 tmp2
	cleanup
} >& /dev/null
if [ $result -ne 0 ]
then
	tst_resm TFAIL "move/test22: FAILED: move: shared tree within a tree it is bound to - and then move to another share subtree"
	exit 1
else
	tst_resm TPASS "move/test22: PASSED"
	exit 0
fi
tst_exit
