#!/bin/sh
export LTPROOT=${PWD}
echo $LTPROOT

TMP="/tmp/runalltests-$$"
mkdir ${TMP}
cd ${TMP}

RT=${LTPROOT}/ltctests/runtest

export PATH="${PATH}:${LTPROOT}/doio:${LTPROOT}/tests:${LTPROOT}/ltctests/bin"
export TCtmp=${TMP}
cat ${RT}/pth_str ${RT}/mem ${RT}/files ${RT}/fs ${RT}/ade > ${TMP}/ltctests

${LTPROOT}/pan/pan -e -S -a $$ -n $$ -f ${TMP}/ltctests -l /tmp/ltc.log
if [ $? -eq "0" ]; then
  echo pan reported PASS
else
  echo pan reported FAIL
fi


rm -rf ${TMP}
