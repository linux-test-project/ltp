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

SETS_DEFAULTS="${TCID=test09} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

tst_resm TINFO "***************TEST09***************"
tst_resm TINFO "bind: slave child to shared parent."
tst_resm TINFO "************************************"

. "${FS_BIND_ROOT}/bin/setup" || (tst_resm TWARN "Setup of bind/test09 failed" && tst_exit)
export result=0



trap 'ERR=$? ; ERR_MSG="caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"; break' ERR

while /bin/true ; do
	# This loop is for error recovery purposes only


	"${FS_BIND_ROOT}/bin/makedir" share parent1
	"${FS_BIND_ROOT}/bin/makedir" share parent2
	"${FS_BIND_ROOT}/bin/makedir" share share1
	"${FS_BIND_ROOT}/bin/makedir" share share2
	"${FS_BIND_ROOT}/bin/makedir" share parent1/child1

	mount --bind parent2 share2
	mount --bind share1 parent1/child1
	mount --bind "$disk1" share1
	"${FS_BIND_ROOT}/bin/makedir" slave parent1/child1

	check parent1/child1 share1

	mkdir parent2/child2

	mount --bind parent1/child1 parent2/child2

	check parent1/child1 parent2/child2

	mount --bind "$disk2" parent1/child1/a

	check -n  parent1/child1/a parent2/child2/a
	check -n parent1/child1/a share1/a

	mount --bind "$disk3" parent2/child2/b

	check -n parent1/child1/b parent2/child2/b
	check parent2/child2 share2/child2

	mount --bind "$disk4" share1/c
	check share1/c parent1/child1/c
	check parent1/child1/c parent2/child2/c

	break
done
trap 'ERR=$? ; tst_resm TWARN "bind/test09: caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"' ERR
if [ -n "${ERR_MSG}" ]; then
	tst_resm TWARN "bind/test09: ${ERR_MSG}"
	ERR_MSG=""
	result=$ERR
fi
trap '' ERR
{
	umount parent1/child1/a
	umount parent2/child2/b
	umount share1/c
	umount parent2/child2
	umount parent1/child1
	umount parent1/child1
	umount parent1/child1
	umount share1
	umount share2
	umount share2
	umount parent2
	umount parent1


	rm -rf parent* share*
	cleanup
} >& /dev/null
if [ $result -ne 0 ]
then
        tst_resm TFAIL "bind/test09: FAILED: bind: slave child to shared parent."
        exit 1
else
        tst_resm TPASS "bind/test09: PASSED"
        exit 0
fi


tst_exit
