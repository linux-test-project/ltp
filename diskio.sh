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

export TMPDIR=/tmp

mkdir /tmp/diskiopan-$$
cd /tmp/diskiopan-$$

export PATH="${PATH}:${LTPROOT}/doio:${LTPROOT}/ltctests/bin"
 
${LTPROOT}/pan/pan -e -l /tmp/diskiopan.log -S -a ltpdiskio -n ltpdiskio -f ${LTPROOT}/ltctests/runtest/io

if [ $? -eq "0" ]; then
  echo pan reported PASS
else
  echo pan reported FAIL
fi

