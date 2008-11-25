#!/bin/sh
# Script that allows to run the full test suite automatically.
#
chmod +x ./configure.interactive
./configure.interactive
make deploy
make all
./rpc_ts_wizard.sh 

