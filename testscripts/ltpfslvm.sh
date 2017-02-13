#!/bin/sh
# This script should be run prior to running executing the filesystem tests.
# valid devices need to be passed for Device Mapper to work correctly
# 03/14/03 mridge@us.ibm.com added instance and time command line options
# 05/16/03 changed script paths
# 05/20/03 Added instructions on setup and warnings
# 05/03/2004 hien1@us.ibm.com  Added resize2fs and resize_reiserfs after extend and reduce LVs
# 05/03/2004 Moved the mount after resizing
# 05/03/2004 Modified /dev/ram to /dev/ram0

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
mkdir /test/growfiles/msdos   >/dev/null 2>&1
mkdir /test/growfiles/reiser  >/dev/null 2>&1
mkdir /test/growfiles/minix   >/dev/null 2>&1
mkdir /test/growfiles/nfs     >/dev/null 2>&1
mkdir /test/growfiles/jfs     >/dev/null 2>&1
mkdir /test/growfiles/ramdisk >/dev/null 2>&1

vgscan
vgchange -a y

pvcreate -v /dev/$part1 /dev/$part2 /dev/$part3 /dev/$part4
vgcreate -v ltp_test_vg1 /dev/$part1 /dev/$part2
vgcreate -v ltp_test_vg2 /dev/$part3 /dev/$part4
vgcfgbackup -v
lvcreate -v -L 100 ltp_test_vg1 -n ltp_test_lv1
lvcreate -v -L 100 ltp_test_vg1 -n ltp_test_lv2 -i 2
lvcreate -v -L 100 ltp_test_vg2 -n ltp_test_lv3
lvcreate -v -L 100 ltp_test_vg2 -n ltp_test_lv4

mkfs -V -t ext2     /dev/ltp_test_vg1/ltp_test_lv1
mkfs -V -t msdos    /dev/ltp_test_vg1/ltp_test_lv2
mkreiserfs          /dev/ltp_test_vg2/ltp_test_lv3 <yesenter.txt
mkfs -V -t minix    /dev/ltp_test_vg2/ltp_test_lv4

### there is no /dev/ram - has /dev/ram0 ... /dev/ram15
mkfs -V -t ext3        /dev/ram0


lvmdiskscan -v
lvscan      -v
vgdisplay   -v
lvextend -v -l +5000 /dev/ltp_test_vg1/ltp_test_lv1
lvreduce -v -f -l -20 /dev/ltp_test_vg1/ltp_test_lv1

### Need to be resize to get LV to the correct size
resize2fs -f /dev/ltp_test_vg1/ltp_test_lv1
lvextend -v -l +5000 /dev/ltp_test_vg1/ltp_test_lv2
lvreduce -v -f -l -20 /dev/ltp_test_vg1/ltp_test_lv2

### Need to be resize to get LV to the correct size
resize_reiserfs -f /dev/ltp_test_vg1/ltp_test_lv2
lvextend -v -l +20 /dev/ltp_test_vg2/ltp_test_lv3
lvreduce -v -f -l -20 /dev/ltp_test_vg2/ltp_test_lv3
lvextend -v -l +20 /dev/ltp_test_vg2/ltp_test_lv4
lvreduce -v -f -l -20 /dev/ltp_test_vg2/ltp_test_lv4

vgreduce -v /dev/ltp_test_vg1 /dev/$part2
vgextend -v /dev/ltp_test_vg1 /dev/$part2
vgck -v

### Move mount filesystems to the last since resize can't work on a mounted filesystem.
mount -v -t nfs $nfsmount               /test/growfiles/nfs
mount -v /dev/ltp_test_vg1/ltp_test_lv1 /test/growfiles/ext2
mount -v /dev/ltp_test_vg1/ltp_test_lv2 /test/growfiles/msdos
mount -v /dev/ltp_test_vg2/ltp_test_lv3 /test/growfiles/reiser
mount -v /dev/ltp_test_vg2/ltp_test_lv4 /test/growfiles/minix
mount -v /dev/ram                       /test/growfiles/ramdisk

echo "************ Running tests "
sort -R ${LTPROOT}/runtest/lvm.part1 -o ${TMPBASE}/lvm.part1

${LTPROOT}/bin/ltp-pan -e -S -a lvmpart1 -n lvmpart1 -l lvmlogfile -f ${TMPBASE}/lvm.part1 &

wait $!



umount -v -t nfs $nfsmount
umount -v /dev/ltp_test_vg1/ltp_test_lv1
umount -v /dev/ltp_test_vg1/ltp_test_lv2
umount -v /dev/ltp_test_vg2/ltp_test_lv3
umount -v /dev/ltp_test_vg2/ltp_test_lv4
umount -v /dev/ram

lvremove -f -v /dev/ltp_test_vg1/ltp_test_lv1
lvremove -f -v /dev/ltp_test_vg1/ltp_test_lv2
lvremove -f -v /dev/ltp_test_vg2/ltp_test_lv3
lvremove -f -v /dev/ltp_test_vg2/ltp_test_lv4

lvscan -v
vgchange -a n
vgremove -v /dev/ltp_test_vg1
vgremove -v /dev/ltp_test_vg2

mkfs -V -t ext3     /dev/$part4
mkfs -V -t jfs /dev/$part1  <yesenter.txt

mount -v -t ext3   /dev/$part4         /test/growfiles/ext3
mount -v -t jfs    /dev/hdc1           /test/growfiles/jfs

echo "************ Running EXT3 & JFS tests...  "
sort -R ${LTPROOT}/runtest/lvm.part2 -o ${TMPBASE}/lvm.part2

${LTPROOT}/bin/ltp-pan -e -S -a lvmpart2 -n lvmpart2 -l lvmlogfile -f ${TMPBASE}/lvm.part2 &

wait $!

umount -v /dev/$part1
umount -v /dev/$part4


