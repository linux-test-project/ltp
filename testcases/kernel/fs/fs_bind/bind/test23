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

SETS_DEFAULTS="${TCID=test23} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

tst_resm TINFO "***************TEST24***************"
tst_resm TINFO "bind: shared child to shared parent."
tst_resm TINFO "************************************"

. "${FS_BIND_ROOT}/bin/setup" || (tst_resm TWARN "Setup of bind/test23 failed" && tst_exit)
export result=0



trap 'ERR=$? ; ERR_MSG="caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"; break' ERR

while /bin/true ; do
	# This loop is for error recovery purposes only


	mkdir mnt mnt/1 mnt/2 mnt/1/abc tmp1 tmp1/3 tmp2

	mount --bind mnt mnt
	"${FS_BIND_ROOT}/bin/makedir" share mnt/1 mnt/1
	mount --bind mnt/1 mnt/2
	mount --bind "$disk1" mnt/1/abc

	check mnt/1/abc mnt/2/abc "$disk1"

	"${FS_BIND_ROOT}/bin/makedir" share tmp1
	mount --bind tmp1 tmp2

	mount --move mnt tmp1/3
	check tmp1/3/1/abc tmp2/3/1/abc tmp2/3/2/abc "$disk1"

	break
done
trap 'ERR=$? ; tst_resm TWARN "bind/test23: caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"' ERR
if [ -n "${ERR_MSG}" ]; then
	tst_resm TWARN "bind/test23: ${ERR_MSG}"
	ERR_MSG=""
	result=$ERR
fi
trap '' ERR
{
	umount tmp1/3/1/abc
	umount tmp1/3/1
	umount tmp1/3/2
	umount tmp1/3
	umount tmp1
	umount tmp1
	umount tmp2

	rm -rf mnt tmp1 tmp2


	cleanup
} >& /dev/null
if [ $result -ne 0 ]
then
	tst_resm TFAIL "bind/test23: FAILED: bind: shared child to shared parent."
	exit 1
else
	tst_resm TPASS "bind/test23: PASSED"
	exit 0
fi
tst_exit
