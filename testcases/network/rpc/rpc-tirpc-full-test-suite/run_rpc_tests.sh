#!/bin/sh
# Script that allows to run the full test suite automatically.
#
cd ${LTPROOT}/testcases/network/rpc/rpc-tirpc-full-test-suite
chmod +x ./configure.auto
./configure.auto
make deploy
make all
./rpc_ts_wizard.sh -allrpc 

