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

SETS_DEFAULTS="${TCID=test16} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

tst_resm TINFO "***************TEST16***************"
tst_resm TINFO "rbind: uncloneable child to uncloneable parent."
tst_resm TINFO "************************************"

. "${FS_BIND_ROOT}/bin/setup" || (tst_resm TWARN "Setup of rbind/test16 failed" && tst_exit)
export result=0




trap 'ERR=$? ; ERR_MSG="caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"; break' ERR

while /bin/true ; do
	# This loop is for error recovery purposes only


	"${FS_BIND_ROOT}/bin/makedir" share parent1
	"${FS_BIND_ROOT}/bin/makedir" unclone parent2
	"${FS_BIND_ROOT}/bin/makedir" share share1
	"${FS_BIND_ROOT}/bin/makedir" unclone parent1/child1
	mkdir parent1/child1/x

	mount --rbind parent1 share1
	mount --rbind "$disk1" parent1/child1/x

	mkdir parent2/child2
	mount --rbind parent1/child1 parent2/child2 2> /dev/null || result=$? # mount should fail

	break
done
trap 'ERR=$? ; tst_resm TWARN "rbind/test16: caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"' ERR
if [ -n "${ERR_MSG}" ]; then
	tst_resm TWARN "rbind/test16: ${ERR_MSG}"
	ERR_MSG=""
	result=$ERR
fi
trap '' ERR
{
	umount parent1/child1/x
	umount parent1/child1
	umount share1
	umount share1
	umount parent2
	umount parent1


	rm -rf parent* share*
	cleanup
} >& /dev/null
if [ $result -eq 0 ]
then
        tst_resm TFAIL "rbind/test16: FAILED: rbind: uncloneable child to uncloneable parent."
        exit 1
else
        tst_resm TPASS "rbind/test16: PASSED"
        exit 0
fi


tst_exit
