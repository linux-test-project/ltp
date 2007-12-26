#! /bin/bash

if [ ! $SCRIPTS_DIR ]; then
        # assume we're running standalone
        export SCRIPTS_DIR=../../scripts/
fi

source $SCRIPTS_DIR/setenv.sh

$SCRIPTS_DIR/run_c_files.sh "async_handler"
$SCRIPTS_DIR/run_c_files.sh "async_handler_jk"

# The async_handler_tsc test is off by default for now.
#$SCRIPTS_DIR/run_c_files.sh "async_handler_tsc "
