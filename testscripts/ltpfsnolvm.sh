#!/bin/sh
# This script should be run prior to running executing the filesystem tests.
# valid devices need to be passed for Device Mapper to work correctly
# 03/14/03 mridge@us.ibm.com added instance and time command line options
# 05/16/03 changed script paths
# 05/20/03 Added instructions on setup and warnings

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
	usage: ${0##*/} [ -a part1 ] [ -b part2 ] [ -c part3 ]
                [ -d part4 ] [ -n nfsmount ]
	defaults:
	part1=$part1
	part2=$part2
	part3=$part3
	part4=$part4
        nfsmount=$nfsmount
	ltproot=$TPROOT
	tmpdir=$TMPBASE

	example: ${0##*/} -a hdc1 -b hdc2 -c hdc3 -d hdc4 -n mytesthost:/testmountdir

        fdisk needs to be run and the 4 HD partitions marked as 0x8e -- Linux LVM

        - If this is run on a 2.4 kernel system then LVM must be configured and the kernel rebuilt. In a 2.5 environment
        you must configure Device Mapper and install LVM2 from www.systina.com for the testcase to run correctly.

        - These operations are destructive so do NOT point the tests to partitions where the data shouldn't be overwritten.
        Once these tests are started all data in the partitions you point to will be destroyed.

	END
exit
}

while getopts :a:b:c:d:e:n:v: arg
do      case $arg in
		a)	part1=$OPTARG;;
                b)      part2=$OPTARG;;
                c)      part3=$OPTARG;;
                d)      part4=$OPTARG;;
                n)      nfsmount=$OPTARG;;
                v)      verb=$OPTARG;;

                \?)     echo "************** Help Info: ********************"
                        usage;;
        esac
done

if [ ! -n "$part1"  ]; then
  echo "Missing 1st partition. You must pass 4 partitions for testing"
  usage;
  exit
fi

if [ ! -n "$part2" ]; then
  echo "Missing 2nd partition. You must pass 4 partitions for testing"
  usage;
  exit
fi

if [ ! -n "$part3" ]; then
  echo "Missing 3rd partition. You must pass 4 partitions for testing"
  usage;
  exit
fi

if [ ! -n "$part4" ]; then
  echo "Missing 4th partition. You must pass 4 partitions for testing"
  usage;
  exit
fi

if [ ! -n "$nfsmount" ]; then
  echo "Missing NFS partition. You must pass an NFS mount point for testing"
  usage;
  exit
fi

export PATH="${PATH}:${LTPROOT}/testcases/bin"


mkdir /test                   >/dev/null 2>&1
mkdir /test/growfiles         >/dev/null 2>&1
mkdir /test/growfiles/ext2    >/dev/null 2>&1
mkdir /test/growfiles/ext3    >/dev/null 2>&1
mkdir /test/growfiles/msdos   >/dev/null 2>&1
mkdir /test/growfiles/reiser  >/dev/null 2>&1
mkdir /test/growfiles/minix   >/dev/null 2>&1
mkdir /test/growfiles/nfs     >/dev/null 2>&1
mkdir /test/growfiles/jfs     >/dev/null 2>&1
mkdir /test/growfiles/ramdisk >/dev/null 2>&1

mkfs -V -t ext2     /dev/$part1
mkfs -V -t msdos    /dev/$part2
mkreiserfs          /dev/$part3
mkfs -V -t minix    /dev/$part4


mount -v -t nfs $nfsmount               /test/growfiles/nfs
mount -v /dev/$part1                    /test/growfiles/ext2
mount -v /dev/$part2                    /test/growfiles/msdos
mount -v /dev/$part3                    /test/growfiles/reiser
mount -v /dev/$part4                    /test/growfiles/minix
mount -v /dev/ram                       /test/growfiles/ramdisk

echo "************ Running tests "
sort -R ${LTPROOT}/../runtest/lvm.part1 -o ${TMPBASE}/lvm.part1

${LTPROOT}/../bin/ltp-pan -e -S -a lvmpart1 -n lvmpart1 -l lvmlogfile -f ${TMPBASE}/lvm.part1 &

wait $!



umount -v -t nfs $nfsmount
umount -v /dev/$part1
umount -v /dev/$part2
umount -v /dev/$part3
umount -v /dev/$part4
umount -v /dev/ram

mkfs -V -t ext3     /dev/$part4
mkfs -V -t jfs /dev/$part1  <yesenter.txt

mount -v -t ext3   /dev/$part4         /test/growfiles/ext3
mount -v -t jfs    /dev/$part1         /test/growfiles/jfs

echo "************ Running EXT3 & JFS tests...  "
sort -R ${LTPROOT}/../runtest/lvm.part2 -o ${TMPBASE}/lvm.part2

${LTPROOT}/../bin/ltp-pan -e -S -a lvmpart2 -n lvmpart2 -l lvmlogfile -f ${TMPBASE}/lvm.part2 &

wait $!

umount -v /dev/$part1
umount -v /dev/$part4


