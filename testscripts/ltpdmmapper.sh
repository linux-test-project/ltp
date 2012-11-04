#!/bin/sh
# This script should be run prior to running executing the filesystem tests.
# valid devices need to be passed for Device Mapper to work correctly
# 03/14/03 mridge@us.ibm.com added instance and time command line options

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
	usage: ${0##*/} [ -a part1 ] [ -b part2 ]

	Note: In order to run this test, you must turn on "device mapper"
        component in kernel (it is under device drivers item when you
        run make menuconfig); and you must install userspace supporting
        files (libdevmapper and dmsetup). They are in the device-mapper
        package. You can download it from http://www.sistina.com. Follow
        the README/INSTALL file within the package to install it.


	defaults:
	part1=$part1
	part2=$part2
	ltproot=$LTPROOT
	tmpdir=$TMPBASE

	example: ${0##*/} -a hdc1 -b hdc2


	END
exit
}

while getopts :a:b: arg
do      case $arg in
		a)	part1=$OPTARG;;
                b)      part2=$OPTARG;;

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

echo "Starting Device Mapper Tests..."

echo "0 10240 linear " $part1 "0" > ltp-dev-mapper-table1
echo "0 100000 linear " $part1 "0" > ltp-dev-mapper-table2
echo "0 100000 linear " $part2 "0" > ltp-dev-mapper-table3
echo "0 200000 striped 2 16 " $part1 "0" $part2 "0" > ltp-dev-mapper-table4

echo "Creating Devices..."

dmsetup create dm-test-1 ltp-dev-mapper-table1
dmsetup create dm-test-2 ltp-dev-mapper-table2
dmsetup create dm-test-3 ltp-dev-mapper-table3
dmsetup create dm-test-4 ltp-dev-mapper-table4

echo "Device Info..."

dmsetup info dm-test-1
dmsetup info dm-test-2
dmsetup info dm-test-3
dmsetup info dm-test-4

echo "Device Dependancies..."

dmsetup deps dm-test-1
dmsetup deps dm-test-2
dmsetup deps dm-test-3
dmsetup deps dm-test-4

echo "Device Status..."

dmsetup status dm-test-1
dmsetup status dm-test-2
dmsetup status dm-test-3
dmsetup status dm-test-4

echo "Device Tables..."

dmsetup table dm-test-1
dmsetup table dm-test-2
dmsetup table dm-test-3
dmsetup table dm-test-4

echo "Device Mapper Version..."

dmsetup version

echo "Device Waiting..."

#dmsetup wait dm-test-1
#dmsetup wait dm-test-2
#dmsetup wait dm-test-3
#dmsetup wait dm-test-4

echo "Device Mapper Removing Devices..."

dmsetup remove dm-test-1
dmsetup remove dm-test-2
dmsetup remove dm-test-3
dmsetup remove dm-test-4

echo "Device Mapper Re-Creating Devices..."

dmsetup create dm-test-1 ltp-dev-mapper-table1
dmsetup create dm-test-2 ltp-dev-mapper-table2
dmsetup create dm-test-3 ltp-dev-mapper-table3
dmsetup create dm-test-4 ltp-dev-mapper-table4

echo "Re-Naming Devices..."

dmsetup rename dm-test-1 dm-test-1-new
dmsetup rename dm-test-2 dm-test-2-new
dmsetup rename dm-test-3 dm-test-3-new
dmsetup rename dm-test-4 dm-test-4-new

echo "Suspend Devices..."

dmsetup suspend dm-test-1-new
dmsetup suspend dm-test-2-new
dmsetup suspend dm-test-3-new
dmsetup suspend dm-test-4-new

echo "0 102400 linear " $part1 "0" > ltp-dev-mapper-table1
echo "0 200000 linear " $part1 "0" > ltp-dev-mapper-table2
echo "0 200000 linear " $part2 "0" > ltp-dev-mapper-table3
echo "0 400000 striped 2 16 " $part1 "0" $part2 "0" > ltp-dev-mapper-table4

echo "Re-loading Devices..."

dmsetup reload dm-test-1-new ltp-dev-mapper-table1
dmsetup reload dm-test-2-new ltp-dev-mapper-table2
dmsetup reload dm-test-3-new ltp-dev-mapper-table3
dmsetup reload dm-test-4-new ltp-dev-mapper-table4

echo "Resuming Devices..."

dmsetup resume dm-test-1-new
dmsetup resume dm-test-2-new
dmsetup resume dm-test-3-new
dmsetup resume dm-test-4-new

echo "Device Info..."

dmsetup info dm-test-1-new
dmsetup info dm-test-2-new
dmsetup info dm-test-3-new
dmsetup info dm-test-4-new

echo "Device Dependancies..."

dmsetup deps dm-test-1-new
dmsetup deps dm-test-2-new
dmsetup deps dm-test-3-new
dmsetup deps dm-test-4-new

echo "Device Status..."

dmsetup status dm-test-1-new
dmsetup status dm-test-2-new
dmsetup status dm-test-3-new
dmsetup status dm-test-4-new

echo "Device Tables..."

dmsetup table dm-test-1-new
dmsetup table dm-test-2-new
dmsetup table dm-test-3-new
dmsetup table dm-test-4-new

echo "Device Mapper Remove-all..."

dmsetup remove_all

echo "Device Mapper Checking Status - Shouldn't be anything to check"

dmsetup status dm-test-1-new
dmsetup status dm-test-2-new
dmsetup status dm-test-3-new
dmsetup status dm-test-4-new


