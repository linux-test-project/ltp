#!/bin/sh
# This will only run the quickhit tests.  
cd `dirname $0`
LTPROOT=${PWD}
echo $LTPROOT | grep testscripts > /dev/null 2>&1
if [ $? -eq 0 ]; then
 cd ..
 export LTPROOT=${PWD}
fi


mkdir /tmp/runpan-$$
cd /tmp/runpan-$$

export PATH="${PATH}:${LTPROOT}/doio:${LTPROOT}/testcases/bin"
 
${LTPROOT}/bin/ltp-pan -e $@ -a ltp -n ltp -f ${LTPROOT}/runtest/quickhit

if [ $? -eq "0" ]; then
  echo ltp-pan reported PASS
else
  echo ltp-pan reported FAIL
fi
