#! /bin/bash

if [ ! $SCRIPTS_DIR ]; then
        # assume we're running standalone
        export SCRIPTS_DIR=../../scripts/
fi

source $SCRIPTS_DIR/setenv.sh

$SCRIPTS_DIR/run_c_files.sh "hrtimer-prio"


