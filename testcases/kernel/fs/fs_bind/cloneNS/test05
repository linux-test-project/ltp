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

SETS_DEFAULTS="${TCID=test05} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

tst_resm TINFO "***************TEST05***************"
tst_resm TINFO "cloneNS: namespace with multi-level "
tst_resm TINFO "chain of slaves"
tst_resm TINFO "************************************"

. "${FS_BIND_ROOT}/bin/setup" || (tst_resm TWARN "Setup of cloneNS/test05 failed" && tst_exit)
export result=0


trap 'ERR=$? ; ERR_MSG="caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"; break' ERR

while /bin/true ; do
	# This loop is for error recovery purposes only

	cp "${FS_BIND_ROOT}/cloneNS/parent05" ./
	cp "${FS_BIND_ROOT}/cloneNS/child05" .

	chmod 755 parent05 child05


	"${FS_BIND_ROOT}/bin/makedir" share parent
	"${FS_BIND_ROOT}/bin/makedir" share parent/child1
	"${FS_BIND_ROOT}/bin/makedir" share parent/child2

	mount --rbind "$disk1" parent/child1

	mount --rbind parent parent/child2/

	"${FS_BIND_ROOT}/bin/nsclone" ./parent05 ./child05 || result=$?

	break
done
trap 'ERR=$? ; tst_resm TWARN "cloneNS/test05: caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"' ERR
if [ -n "${ERR_MSG}" ]; then
	tst_resm TWARN "cloneNS/test05: ${ERR_MSG}"
	ERR_MSG=""
	result=$ERR
fi
trap '' ERR
{
	umount parent/child2/child2
	umount parent/child2/child1/a
	umount parent/child2/child1/c
	umount parent/child2/child1
	umount parent/child2/child1
	umount parent/child2
	umount parent/child2
	umount parent

	rm -rf parent* child*

	cleanup
} >& /dev/null
if [ $result -ne 0 ]
then
	tst_resm TFAIL "cloneNS/test05: FAILED: cloneNS: namespace with multi-level
chain of slaves"
	exit 1
else
	tst_resm TPASS "cloneNS/test05: PASSED"
	exit 0
fi
tst_exit
