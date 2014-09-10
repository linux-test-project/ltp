#!/bin/sh
################################################################################
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Tiemple Place, Suite 330, Boston, MA 02111-1307 USA   ##
##                                                                            ##
################################################################################
#
# File:          smt_smp_enabled.sh
#
# Description: This program tests whether HT/SMP is supported or not.
#
# Author:      Rohit Verma, rohit.170309@gmail.com
#
# History:     May 21 2009 - Created. - Rohit Verma

export TST_TOTAL=1
export TCID=smt_smp_enabled
export TST_COUNT=1
TFAILCNT=0
RC=0


# check for SMT/SMP System
tst_resm TINFO "Begin: SMT/SMP Enabled"
if [ -f ./ht_enabled ];then
	./ht_enabled
	ret_value=$?

	if [ $ret_value -ne 0 ];then
		tst_resm TCONF "SMT/SMP is not supported"
		tst_resm TINFO "End: SMT/SMP Enabled"
		exit 0
	fi
else
	tst_resm TBROK "ht_enabled:File not found"
	tst_resm TINFO "End: SMT/SMP Enabled"
	TFAILCNT=$(( $TFAILCNT+1 ))
	exit $TFAILCNT
fi

no_of_processor=`tst_ncpus`
no_of_cpu=`tst_ncpus_conf`

if [ $no_of_processor -lt $no_of_cpu ];then
	test_op="Enable:/sys/devices/system/cpu/cpuX/online"
	tst_resm TCONF "$test_op"
	tst_resm TINFO "End: SMT/SMP Enabled"
	TFAILCNT=$(( $TFAILCNT+1 ))
	exit $TFAILCNT
else
cpu_cores=`cat /proc/cpuinfo | grep "cpu cores" | cut -f 2 -d ':'\
			 | sed 's/ //' | uniq`

	if [ $cpu_cores -eq 1 ];then
		tst_resm TPASS "SMT is enable"
	elif [ $cpu_cores -ge 2 ];then
		tst_resm TPASS "SMP System"
	else
		tst_resm TCONF "TBD"
	fi
fi

tst_resm TINFO "End: SMT/SMP Enabled"

exit
