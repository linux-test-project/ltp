#!/bin/sh


CPU_TO_TEST=${CPU_TO_TEST:=1}
LOOPS=${LOOPS:=1}
export LHCS_PATH=${LHCS_PATH:-$LTPROOT/testcases/bin/cpu_hotplug}


echo "CPU:        $CPU_TO_TEST"
echo "LOOPS:      $LOOPS"
echo "LHCS_PATH:  $LHCS_PATH"


for case in functional/hotplug??.sh; do
	$case $CPU_TO_TEST
done
