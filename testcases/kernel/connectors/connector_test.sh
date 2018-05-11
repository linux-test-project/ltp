#!/bin/sh

if [ ! -f /proc/net/connector ];then
	echo "Connectors 0 CONF : system doesn't support execution of the test"
	exit 32
fi

$LTPROOT/testcases/bin/run_pec_test

