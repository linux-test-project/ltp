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

SETS_DEFAULTS="${TCID=test03} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

tst_resm TINFO "***************TEST03***************"
tst_resm TINFO "cloneNS: namespace with unclonable mount "
tst_resm TINFO "************************************"

. "${FS_BIND_ROOT}/bin/setup" || (tst_resm TWARN "Setup of cloneNS/test03 failed" && tst_exit)
export result=0


trap 'ERR=$? ; ERR_MSG="caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"; break' ERR

while /bin/true ; do
	# This loop is for error recovery purposes only

	cp "${FS_BIND_ROOT}/cloneNS/parent03" ./
	cp "${FS_BIND_ROOT}/cloneNS/child03" .

	chmod 755 parent03 child03

	"${FS_BIND_ROOT}/bin/makedir" unclone dir1
	mount --bind "$disk1" dir1


	"${FS_BIND_ROOT}/bin/nsclone" ./parent03 ./child03 || result=$?

	break
done
trap 'ERR=$? ; tst_resm TWARN "cloneNS/test03: caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"' ERR
if [ -n "${ERR_MSG}" ]; then
	tst_resm TWARN "cloneNS/test03: ${ERR_MSG}"
	ERR_MSG=""
	result=$ERR
fi
trap '' ERR
{
	umount dir1
	umount dir1

	rm -rf dir* parent* child*

	cleanup
} >& /dev/null
if [ $result -ne 0 ]
then
	tst_resm TFAIL "cloneNS/test03: FAILED: cloneNS: namespace with unclonable mount "
	exit 1
else
	tst_resm TPASS "cloneNS/test03: PASSED"
	exit 0
fi
tst_exit
