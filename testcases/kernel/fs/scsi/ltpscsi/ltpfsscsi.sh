#!/bin/sh
# This script should be run to execute the filesystem tests on SCSI vitual devices.
# 10/21/03 mridge@us.ibm.com Initial creation of testcases
#
#

cd `dirname $0`
export LTPROOT=${PWD}
echo $LTPROOT | grep testscripts > /dev/null 2>&1
if [ $? -eq 0 ]; then
 cd ..
 export LTPROOT=${PWD}
fi

export TMPBASE="/tmp"


usage()
{
	cat <<-END >&2
	usage: ${0##*/} [ -a part1 ] [ -b part2 ] [ -k Kernel Path - fully qualified kernel path ]
	defaults:

        There are no defaults, all items MUST be passed

	example: ${0##*/} -a sda -b sdb -k /usr/src/linux

        - These tests must be run after ssi_debug has been configured and built as a module so it can be loaded with
          the correct parameters.

        - These operations are destructive so do NOT point the tests to partitions where the data shouldn't be overwritten.
        Once these tests are started all data in the partitions you point to will be destroyed.

	END
exit
}

while getopts :a:b:c:k: arg
do      case $arg in
		a)	part1=$OPTARG;;
                b)      part2=$OPTARG;;
                c)      part3=$OPTARG;;
                k)      kernpath=$OPTARG;;

                \?)     echo "************** Help Info: ********************"
                        usage;;
        esac
done

if [ ! -n "$part1"  ]; then
  echo "Missing 1st partition. You must pass 2 partitions for testing"
  usage;
  exit
fi

if [ ! -n "$part2" ]; then
  echo "Missing 2nd partition. You must pass 2 partitions for testing"
  usage;
  exit
fi

if [ ! -n "$part3" ]; then
  echo "Missing 3rd partition. You must pass 3 partitions for testing"
  usage;
  exit
fi

if [ ! -n "$kernpath" ]; then
  echo "Missing kernel path. You must pass kernel path for testing"
  usage;
  exit
fi

export PATH="${PATH}:${LTPROOT}/testcases/bin"


mkdir /test                   >/dev/null 2>&1
mkdir /test/growfiles         >/dev/null 2>&1
mkdir /test/growfiles/scsi    >/dev/null 2>&1
mkdir /test/growfiles/scsi/ext2    >/dev/null 2>&1
mkdir /test/growfiles/scsi/ext3    >/dev/null 2>&1
mkdir /test/growfiles/scsi/reiser  >/dev/null 2>&1


mkfs -V -t ext2     /dev/$part1 <yesenter.txt
mkfs -V -t ext3     /dev/$part2 <yesenter.txt
mkreiserfs -f       /dev/$part3 <yesenter.txt


mount -v -t ext2 /dev/$part1       /test/growfiles/scsi/ext2
mount -v -t ext3 /dev/$part2       /test/growfiles/scsi/ext3
mount -v /dev/$part3               /test/growfiles/scsi/reiser

cd $kernpath/drivers/scsi
modprobe scsi_debug max_luns=2 num_tgts=7 add_host=10
cd ${LTPROOT}

echo "************ Running tests "
sort -R ${LTPROOT}/runtest/scsi.part1 -o ${TMPBASE}/scsi.part1

${LTPROOT}/pan/pan -e -S -a scsipart1 -n scsipart1 -l scsilogfile -f ${TMPBASE}/scsi.part1 &

wait $!

umount -v /dev/$part1
umount -v /dev/$part2
umount -v /dev/$part3
rmmod scsi_debug



