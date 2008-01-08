#!/bin/bash

if [ $# -le 0 ]; then
        echo "Usage run.sh <Test Dir>"
        echo "Give Absolute Path"
        exit;
fi

TEST_DIR=$1

export LTPROOT=${TEST_DIR}
export TMPBASE=${TEST_DIR}
export TMP="${TMPBASE}/ltp-$$"
export TMPDIR=${TMP}

mkdir $TMPDIR

./fallocate01
./fallocate02
./fallocate03

rm -rf  $TMPDIR

