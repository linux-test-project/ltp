#! /bin/sh

if [ -z $LTPROOT ]; then
	export LTPROOT="`cd ../../.. && pwd`"
        export PATH="$PATH:$LTPROOT/testcases/bin"
fi

tst_kvercmp 2 6 15
if [ $? -eq 0 ]; then
	echo "Connectors 0 CONF : system doesn't support execution of the test"
	exit 0
fi

$LTPROOT/testcases/bin/run_pec_test

