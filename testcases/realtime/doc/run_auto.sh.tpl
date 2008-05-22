#! /bin/bash

profile=${1:-default}

cd $(dirname $0) # Move to test directory
if [ ! $SCRIPTS_DIR ]; then
        # assume we're running standalone
        export SCRIPTS_DIR=../../scripts/
fi

source $SCRIPTS_DIR/setenv.sh

# Warning: tests args are now set in profiles

# Customize below. One line per test.
$SCRIPTS_DIR/run_c_files.sh $profile testexecutable1
# $SCRIPTS_DIR/run_c_files.sh $profile testexecutable2
