#!/bin/sh
#
# Test Case 6
#

CASE="hotplug06"
CPU_TO_TEST=${1#cpu}

if [ -z $CPU_TO_TEST ]; then
        echo "Usage:  $0 <CPU to offline>"
        exit -1
fi

# Includes:
LHCS_PATH=${LHCS_PATH:-".."}
. $LHCS_PATH/include/testsuite.fns
. $LHCS_PATH/include/hotplug.fns

echo "Name:   Hotplug Test - Test Case 6"
echo "Date:   `date`"
echo "Desc:   Verify user tools can handle adding and removing CPUs."
echo

$LHCS_PATH/functional/hotplug06.top.sh ${CPU_TO_TEST}

echo
echo

$LHCS_PATH/functional/hotplug06.sar.sh ${CPU_TO_TEST}

echo