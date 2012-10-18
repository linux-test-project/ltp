#!/bin/sh
# These tests require the corresponding media to be present:
#
#  stress_floppy: Requires a writeable HD floppy disk
#
#  stress_cd: Requires ANY data CD with a minimum of 100MB of data.
#
# NOTE: The stress_floppy test is easily modifiable for use on any
#       write device, i.e. tape drive, zip drive....etc
#

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

${LTPROOT}/bin/ltp-pan -e -l /tmp/diskiopan.log -S -a ltpdiskio -n ltpdiskio -f ${LTPROOT}/runtest/io_floppy -f ${LTPROOT}/runtest/io_cd

if [ $? -eq "0" ]; then
  echo ltp-pan reported PASS
else
  echo ltp-pan reported FAIL
fi

