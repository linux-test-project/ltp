-#!/bin/sh
# This script should be run prior to running executing the filesystem tests.
# valid devices need to be passed for the test to work correctly
# 10/06/03 mridge@us.ibm.com added instance and time command line options
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
	usage: ${0##*/} [ -a part1 ] [ -n nfsmount ]
	defaults:
	part1=$part1
        nfsmount=$nfsmount
	ltproot=$TPROOT
	tmpdir=$TMPBASE

	example: ${0##*/} -a hdc1 -b hdc2 -c hdc3 -d hdc4 -n mytesthost:/testmountdir

        - This test will ONLY run on a 2.5.66 or higher kernel system.

        - These operations are destructive so do NOT point the tests to partitions where the data shouldn't be overwritten.
        Once these tests are started all data in the partitions you point to will be destroyed.

	END
exit
}

while getopts :a:n:v: arg
do      case $arg in
		a)	part1=$OPTARG;;
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

if [ ! -n "$nfsmount" ]; then
  echo "Missing NFS partition. You must pass an NFS mount point for testing"
  usage;
  exit
fi

export PATH="${PATH}:${LTPROOT}/testcases/bin"


mkdir /test                   >/dev/null 2>&1
mkdir /test/growfiles         >/dev/null 2>&1
mkdir /test/growfiles/ext2    >/dev/null 2>&1
mkdir /test/growfiles/msdos   >/dev/null 2>&1
mkdir /test/growfiles/reiser  >/dev/null 2>&1
mkdir /test/growfiles/minix   >/dev/null 2>&1
mkdir /test/growfiles/xfs     >/dev/null 2>&1
mkdir /test/growfiles/nfs     >/dev/null 2>&1
mkdir /test/growfiles/jfs     >/dev/null 2>&1
mkdir /test/growfiles/ext3 >/dev/null 2>&1


mkfs -V -t ext2     /dev/$part1


mount -v -t nfs $nfsmount               /test/growfiles/nfs
mount -v /dev/$part1 /test/growfiles/ext2


echo "************ Running tests "
sort -R ${LTPROOT}/runtest/ltpfs.part1 -o ${TMPBASE}/ltpfs.part1

${LTPROOT}/pan/pan -e -S -a ltpfspart1 -n ltpfspart1 -l lvmlogfile -f ${TMPBASE}/ltpfs.part1 &

wait $!

umount -v -t nfs $nfsmount
umount -v /dev/$part1
mkfs.xfs -f   /dev/$part1
mount -v /dev/$part1 /test/growfiles/xfs


sort -R ${LTPROOT}/runtest/ltpfs.part2 -o ${TMPBASE}/ltpfs.part2

${LTPROOT}/pan/pan -e -S -a ltpfspart2 -n ltpfspart2 -l lvmlogfile -f ${TMPBASE}/ltpfs.part2 &

wait $!

mkfs -V -t msdos    /dev/$part1
umount -v /dev/$part1
mount -v /dev/$part1 /test/growfiles/msdos

sort -R ${LTPROOT}/runtest/ltpfs.part3 -o ${TMPBASE}/ltpfs.part3

${LTPROOT}/pan/pan -e -S -a ltpfspart3 -n ltpfspart3 -l lvmlogfile -f ${TMPBASE}/ltpfs.part3 &

wait $!

umount -v /dev/$part1
mkreiserfs          /dev/$part1 <yesenter.txt
mount -v /dev/$part1 /test/growfiles/reiser

sort -R ${LTPROOT}/runtest/ltpfs.part4 -o ${TMPBASE}/ltpfs.part4

${LTPROOT}/pan/pan -e -S -a ltpfspart4 -n ltpfspart4 -l lvmlogfile -f ${TMPBASE}/ltpfs.part4 &

wait $!

umount -v /dev/$part1
mkfs -V -t minix    /dev/$part1
mount -v /dev/$part1 /test/growfiles/minix

sort -R ${LTPROOT}/runtest/ltpfs.part5 -o ${TMPBASE}/ltpfs.part5

${LTPROOT}/pan/pan -e -S -a ltpfspart5 -n ltpfspart5 -l lvmlogfile -f ${TMPBASE}/ltpfs.part5 &

wait $!

umount -v /dev/$part1
mkfs -V -t ext3     /dev/$part1
mount -v /dev/$part1 /test/growfiles/ext3

sort -R ${LTPROOT}/runtest/ltpfs.part6 -o ${TMPBASE}/ltpfs.part6

${LTPROOT}/pan/pan -e -S -a ltpfspart6 -n ltpfspart6 -l lvmlogfile -f ${TMPBASE}/ltpfs.part6 &

wait $!

umount -v /dev/$part1
mkfs -V -t jfs /dev/$part1  <yesenter.txt
mount -v -t jfs    /dev/$part1           /test/growfiles/jfs

sort -R ${LTPROOT}/runtest/ltpfs.part7 -o ${TMPBASE}/ltpfs.part7

${LTPROOT}/pan/pan -e -S -a ltpfspart7 -n ltpfspart7 -l lvmlogfile -f ${TMPBASE}/ltpfs.part7 &

wait $!


