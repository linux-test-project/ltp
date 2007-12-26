#! /bin/bash

if [ ! $SCRIPTS_DIR ]; then
        # assume we're running standalone
        export SCRIPTS_DIR=../../scripts/
fi

source $SCRIPTS_DIR/setenv.sh

$SCRIPTS_DIR/run_c_files.sh "gtod_latency"

# This is only for hostility testing
#$SCRIPTS_DIR/run_c_files.sh "gtod_infinite"

