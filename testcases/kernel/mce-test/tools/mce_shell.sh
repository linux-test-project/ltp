#!/bin/bash
#
# Simulate the environment of mce-test driver or test case shell
# script, used for debugging. You can invoking mce-test library
# functions directly in shell created.
#
# Copyright (C) 2009, Intel Corp.
#   Author: Huang Ying <ying.huang@intel.com>
#
# This file is released under the GPLv2.
#

sd=$(dirname "$0")
export ROOT=`(cd $sd/..; pwd)`

if [ $# -eq 1 ]; then
    export driver=$1
else
    export driver=simple
fi

tmpfile=$(mktemp)

trap "rm $tmpfile" EXIT

cat <<"EOF" > $tmpfile
if [ -f /etc/bash.bashrc ]; then
    source /etc/bash.bashrc
fi

if [ -f $HOME/.bashrc ]; then
    source $HOME/.bashrc
fi

. $ROOT/lib/functions.sh
setup_path
. $ROOT/lib/dirs.sh
. $ROOT/lib/mce.sh
. $ROOT/lib/soft-inject.sh

export PS1="MDE $driver: "

echo "-----------------------------------------------------"
echo "| MCE-test shell, You can use mce internal function |"
echo "-----------------------------------------------------"
EOF

export PS1="MCE $driver: "
/bin/bash --rcfile $tmpfile
rm $tmpfile
