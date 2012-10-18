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

SETS_DEFAULTS="${TCID=test08} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

tst_resm TINFO "***************TEST08***************"
tst_resm TINFO "move: private subtree to uncloneable parent."
tst_resm TINFO "************************************"

. "${FS_BIND_ROOT}/bin/setup" || (tst_resm TWARN "Setup of move/test08 failed" && tst_exit)
export result=0



trap 'ERR=$? ; ERR_MSG="caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"; break' ERR

while /bin/true ; do
	# This loop is for error recovery purposes only


	"${FS_BIND_ROOT}/bin/makedir" priv dir
	"${FS_BIND_ROOT}/bin/makedir" unclone parent2
	"${FS_BIND_ROOT}/bin/makedir" share share1

	mkdir dir/grandchild
	mount --bind dir share1
	mkdir parent2/child2
	mount --move dir parent2/child2

	mount --bind "$disk1" parent2/child2/grandchild
	check -n dir/grandchild parent2/child2/grandchild
	check -n share1/grandchild parent2/child2/grandchild

	break
done
trap 'ERR=$? ; tst_resm TWARN "move/test08: caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"' ERR
if [ -n "${ERR_MSG}" ]; then
	tst_resm TWARN "move/test08: ${ERR_MSG}"
	ERR_MSG=""
	result=$ERR
fi
trap '' ERR
{
	umount parent2/child2/grandchild
	umount parent2/child2 # if move succeeded
	umount dir # if move failed
	umount share1
	umount share1
	umount parent2
	umount dir

	rm -rf dir parent* share*
	cleanup
} >& /dev/null
if [ $result -ne 0 ]
then
	tst_resm TFAIL "move/test08: FAILED: move: private subtree to uncloneable parent."
	exit 1
else
	tst_resm TPASS "move/test08: PASSED"
	exit 0
fi
tst_exit
