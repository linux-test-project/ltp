#!/bin/sh
#
# Test Case 6
#

CPU_TO_TEST=${1#cpu}
TCID=hotplug06

if [ -z "$CPU_TO_TEST" ]; then
	echo "usage: ${0##*/} <CPU to offline>"
	exit 1
fi

# Includes:
LHCS_PATH=${LHCS_PATH:-${LTPROOT:+$LTPROOT/testcases/bin/cpu_hotplug}}
. $LHCS_PATH/include/testsuite.fns
. $LHCS_PATH/include/hotplug.fns

cat <<EOF
Name:   $TCID 
Date:   `date`
Desc:   Verify user tools can handle adding and removing CPUs.

EOF

$LHCS_PATH/functional/hotplug06.top.sh ${CPU_TO_TEST}

echo
echo

$LHCS_PATH/functional/hotplug06.sar.sh ${CPU_TO_TEST}

echo
