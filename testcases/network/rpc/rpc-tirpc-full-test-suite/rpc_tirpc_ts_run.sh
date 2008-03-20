#!/bin/sh
# Script that allows to run the full test suite automatically.
#
make deploy
make all
./rpc_ts_wizard.sh -all 

