#!/bin/sh
# Script that allows to run the full test suite automatically.
#
SAVEPWD=$PWD
cd ../../../..
export LTPROOT=${PWD}
cd $SAVEPWD
make deploy
make all
./rpc_ts_wizard.sh -all 

