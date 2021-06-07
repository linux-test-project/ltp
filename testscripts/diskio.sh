#!/bin/sh
# These tests require the corresponding media to be present:
#
#  stress_cd: Requires ANY data CD with a minimum of 100MB of data.

cd `dirname $0`

export LTPROOT=${PWD}
echo $LTPROOT | grep testscripts > /dev/null 2>&1
if [ $? -eq 0 ]; then
 cd ..
 export LTPROOT=${PWD}
fi

export TMPDIR=/tmp

mkdir /tmp/diskiopan-$$
cd /tmp/diskiopan-$$

export PATH="${PATH}:${LTPROOT}/testcases/bin"

${LTPROOT}/ver_linux

${LTPROOT}/bin/ltp-pan -e -l /tmp/diskiopan.log -S -a ltpdiskio -n ltpdiskio -f ${LTPROOT}/runtest/io_cd

if [ $? -eq "0" ]; then
  echo ltp-pan reported PASS
else
  echo ltp-pan reported FAIL
fi

