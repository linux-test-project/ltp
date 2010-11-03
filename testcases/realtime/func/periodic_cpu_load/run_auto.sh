#!/bin/sh

profile=${1:-default}

cd $(dirname $0) # Move to test directory
if [ ! $SCRIPTS_DIR ]; then
        # assume we're running standalone
        export SCRIPTS_DIR=../../scripts/
fi

. $SCRIPTS_DIR/setenv.sh

# Warning: tests args are now set in profiles
$SCRIPTS_DIR/run_c_files.sh $profile periodic_cpu_load periodic_cpu_load_single
