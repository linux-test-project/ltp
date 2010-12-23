#! /bin/sh

################################################################################
##                                                                            ##
## Copyright (c) 2009 FUJITSU LIMITED                                         ##
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
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
## Author: Shi Weihua <shiwh@cn.fujitsu.com>                                  ##
##                                                                            ##
################################################################################
cd $LTPROOT/testcases/bin

export TCID="cgroup_test_fj"
export TST_TOTAL=194
export TST_COUNT=1

export TESTROOT=`pwd`
export LOGFILE=$LTPROOT/output/cgroup_fj_log_`date +%F`.txt
export TMPFILE=$TESTROOT/tmp_tasks
export CASENO1=0
export CASENO2=0

if [ ! -f /proc/cgroups ]; then
	tst_brkm TCONF ignored "Kernel does not support for control groups; skipping testcases";
	exit 0
elif [ "x$(id -ru)" != x0 ]; then
	tst_brkm TCONF ignored "Test must be run as root"
	exit 0
fi

if [ ! -x "$TESTROOT/cgroup_fj_proc" ]; then
	tst_resm TBROK "Test application - cgroup_fj_proc - does not exist or is not executable";
	exit 1
fi

CPUSET=`grep -w cpuset /proc/cgroups | cut -f1`;
DEBUG=`grep -w debug /proc/cgroups | cut -f1`;
NS=`grep -w ns /proc/cgroups | cut -f1`;
CPU=`grep -w cpu /proc/cgroups | cut -f1`;
CPUACCT=`grep -w cpuacct /proc/cgroups | cut -f1`;
MEMORY=`grep -w memory /proc/cgroups | cut -f1`;
FREEZER=`grep -w freezer /proc/cgroups | cut -f1`
DEVICES=`grep -w devices /proc/cgroups | cut -f1`
SUBSYSCOMPILED="";
if [ "$CPUSET" = "cpuset" ]; then
	SYBSYSCOMPILED="cpuset,"
fi
if [ "$DEBUG" = "debug" ]; then
	SYBSYSCOMPILED="$SYBSYSCOMPILED debug,"
fi
if [ "$NS" = "ns" ]; then
	SYBSYSCOMPILED="$SYBSYSCOMPILED ns,"
fi
if [ "$CPU" = "cpu" ]; then
	SYBSYSCOMPILED="$SYBSYSCOMPILED cpu,"
fi
if [ "$CPUACCT" = "cpuacct" ]; then
	SYBSYSCOMPILED="$SYBSYSCOMPILED cpuacct,"
fi
if [ "$MEMORY" = "memory" ]; then
	SYBSYSCOMPILED="$SYBSYSCOMPILED memory,"
fi
if [ "$FREEZER" = "freezer" ]; then
	SYBSYSCOMPILED="$SYBSYSCOMPILED freezer,"
fi
if [ "$DEVICES" = "devices" ]; then
        SYBSYSCOMPILED="$SYBSYSCOMPILED devices,"
fi
if [ "$SYBSYSCOMPILED" = "" ];then	# Warning and exit if all cgroup subsystem are not compiled
	echo "CONTROLLERS TESTCASES: WARNING";
	echo "Kernel does not support for any cgroup subsystem";
	echo "Skipping all controllers testcases....";
	exit -2;
fi

echo "Now, we start the test for cgroup...";

rm -f $LOGFILE 2>/dev/null
echo `date` > $LOGFILE
echo `uname -a` >> $LOGFILE

echo "" >> $LOGFILE
echo "Now, we start the test for basic function of cgroup..." >> $LOGFILE

nlines=`cat cgroup_fj_testcases.sh | wc -l`
for i in `seq 1 $nlines`
do
	CASETYPE=`sed -n "$i""p" cgroup_fj_testcases.sh | cut -f1`
	CASECMD=`sed -n "$i""p" cgroup_fj_testcases.sh | cut -f2`
	echo $CASETYPE | grep "#"
	if [ $? -ne 0 ]; then
		case $CASETYPE in
		"function" )
			: $(( CASENO1 += 1 ))
			export CASENO1=$CASENO1
			$TESTROOT/cgroup_fj_function.sh $CASECMD
			;;
		"function2" )
			: $(( CASENO1 += 1 ))
			export CASENO1=$CASENO1
			$TESTROOT/cgroup_fj_function2.sh $CASECMD
			;;
		"stress" )
			: $(( CASENO2 += 1 ))
			export CASENO2=$CASENO2
			$TESTROOT/cgroup_fj_stress.sh $CASECMD
			;;
		esac

		ret=$?
		if [ $ret -eq 0 ]; then
			tst_resm TPASS "case$i(`sed -n "$i""p" cgroup_fj_testcases.sh`)    PASS"
		elif [ $ret -ne 9 ]; then
			tst_resm TFAIL "case$i(`sed -n "$i""p" cgroup_fj_testcases.sh`)    FAIL"
		fi
	fi
done

exit $ret;
