#!/bin/sh

# 
#  7/10/02 martinjn@us.ibm.com added instance and time command line options
#

cd `dirname $0`
export LTPROOT=${PWD}
export TMP="/tmp/runalltests-$$"
export PAN_LOG="/root/ltp/ltp-logfile"

mkdir ${TMP}
cd ${TMP}

export PATH="${PATH}:${LTPROOT}/testcases/bin"

cat ${LTPROOT}/runtest/syscalls ${LTPROOT}/runtest/fs ${LTPROOT}/runtest/dio ${LTPROOT}/runtest/mm ${LTPROOT}/runtest/commands ${LTPROOT}/runtest/ipc ${LTPROOT}/runtest/sched ${LTPROOT}/runtest/float > ${TMP}/alltests

# The fsx-linux tests use the SCRATCHDEV environment variable as a location
# that can be reformatted and run on.  Set SCRATCHDEV if you want to run 
# these tests.  As a safeguard, this is disabled.
unset SCRATCHDEV
if [ -n "$SCRATCHDEV" ]; then
  cat ${LTPROOT}/runtest/fsx >> ${TMP}/alltests
fi

# In case you want to specify the number of instances from the command line 
if [ $1 ]; then
    instances="-x $1"
else
    instances=""
fi

# In case you want to specify the time to run from the command line (2m = two minutes, 2h = two hours, etc)
# Obviously if you want to use this parameter, you must include the instances parameter first
if [ $2 ]; then
    time="-t $2"
else
    time=""
fi

${LTPROOT}/ver_linux

${LTPROOT}/pan/pan -e -S $instances $time -a $$ -n $$ -f ${TMP}/alltests -l /tmp/sniff
# Use the following pan runline for use with ltp automation scripts instead of the one above.
# ${LTPROOT}/pan/pan -l $PAN_LOG -e -S $instances $time -a $$ -n $$ -f ${TMP}/alltests

if [ $? -eq "0" ]; then
  echo pan reported PASS
else
  echo pan reported FAIL
fi

rm -rf ${TMP}
