#!/bin/bash

SETS_DEFAULTS="${TCID=test07-2} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

tst_resm TINFO "***************TEST07***************"
tst_resm TINFO "rbind: create slave then mount master - slave still propagates."
tst_resm TINFO "************************************"

. "${FS_BIND_ROOT}/bin/setup" || (tst_resm TWARN "Setup of rbind/test07-2 failed" && tst_exit)
export result=0



trap 'ERR=$? ; ERR_MSG="caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"; break' ERR

while /bin/true ; do
	# This loop is for error recovery purposes only


	"${FS_BIND_ROOT}/bin/makedir" share parent2
	"${FS_BIND_ROOT}/bin/makedir" share share2

	mount --rbind share2 parent2
	"${FS_BIND_ROOT}/bin/makedir" slave parent2
	mount --rbind "$disk1" share2

	check parent2 share2

	mount --rbind "$disk2" parent2/a

	check -n parent2/a share2/a

	break
done
trap 'ERR=$? ; tst_resm TWARN "rbind/test07-2: caught error near: ${BASH_SOURCE[0]}:${FUNCNAME[0]}:${LINENO}:$_ (returned ${ERR})"' ERR
if [ -n "${ERR_MSG}" ]; then
	tst_resm TWARN "rbind/test07-2: ${ERR_MSG}"
	ERR_MSG=""
	result=$ERR
fi
trap '' ERR
{
	umount parent2/a
	umount parent2
	umount parent2
	umount parent2

	umount share2
	umount share2


	rm -rf parent* share*
	cleanup
} >& /dev/null
if [ $result -ne 0 ]
then
        tst_resm TFAIL "rbind/test07-2: FAILED: rbind: create slave then mount master - slave still propagates."
        exit 1
else
        tst_resm TPASS "rbind/test07-2: PASSED"
        exit 0
fi


tst_exit
