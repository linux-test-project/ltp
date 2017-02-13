#!/bin/bash

# Check if scsi_debug module was built or not
export kernel=$(uname -r)

ls /lib/modules/$kernel/kernel/drivers/scsi | grep scsi_debug > /dev/null 2>&1
if [ $? -ne 0 ];
then
	echo "scsi_debug was not built in the kernel as a module"
	echo "Build scsi_debug as a module first before running the test"
fi

# Unload scsi_debug moudle if it was already loaded:
lsmod | grep scsi_debug > /dev/null 2>&1
if [ $? -eq 0 ];
then
        echo "The scsi_debug module was already loaded, unload it before test..."
	rmmod -f scsi_debug
fi
lsmod | grep scsi_debug > /dev/null 2>&1
if [ $? -eq 0 ];
then
        echo "The scsi_debug module was not unloaded - unload error"
else
        echo "The scsi_debug module was unloaded successfully"
	echo "start testing..."
fi

orig_count=$(cat /proc/partitions | wc -l)

echo " Load the scsi_debug module..."
modprobe scsi_debug
if [ $? -ne 0 ];
then
        echo "Can't load scsi_debug modules"
	exit
fi

echo "Check if scsi_debug was loaded..."
lsmod | grep scsi_debug > /dev/null 2>&1
if [ $? -eq 0 ];
then
        echo "scsi_debug module was loaded successfully"
else
	echo "scsi_debug module was not loaded"
	exit
fi


echo "Remove the scsi_debug device..."
dev_name=$(ls /proc/scsi/scsi_debug)
# echo $dev_name
rm_dev=$dev_name:0:0:0
# echo $rm_dev
echo 1 > /sys/class/scsi_device/$rm_dev/device/delete

echo "Check if the scsi_debug device was removed..."
ls /sys/class/scsi_device | grep $rm_dev > /dev/null 2>&1
if [ $? -eq 0 ];
then
        echo "The device was not removed - remove error"
else
	echo "The device $dev_name was removed successfully"
fi

echo "Add the device back..."
echo "0 0 0" > /sys/class/scsi_host/host$dev_name/scan
ls /sys/class/scsi_device | grep $rm_dev > /dev/null 2>&1
if [ $? -ne 0 ];
then
        echo "The device was not added - add device error"
else
        echo "The device $dev_name was added back successfully"
fi

echo "Add a new host..."
echo 1 > /sys/bus/pseudo/drivers/scsi_debug/add_host
num_host=$(cat /sys/bus/pseudo/drivers/scsi_debug/add_host)
if [ $num_host -ne 2 ];
then
	echo "The new host was not added - add host error"
else
	echo "The new host was added successfully"
fi

echo "Romove hosts..."
echo -2 > /sys/bus/pseudo/drivers/scsi_debug/add_host
num_host=$(cat /sys/bus/pseudo/drivers/scsi_debug/add_host)
if [ $num_host -ne 0 ];
then
        echo "The hosts were not removed - remove hosts error"
else
	echo "The hosts were removed successfully"
fi

echo "Unload scsi_debug moudle..."
rmmod -f scsi_debug
lsmod | grep scsi_debug > /dev/null 2>&1
if [ $? -eq 0 ];
then
        echo "The scsi_debug module was not unloaded - unload error"
else
        echo "The scsi_debug module was unloaded successfully"
fi

echo "Load scsi_debug with multiple hosts..."
modprobe scsi_debug max_luns=2 num_tgts=2 add_host=2 dev_size_mb=20
lsmod | grep scsi_debug > /dev/null 2>&1
if [ $? -eq 0 ];
then
        echo "The multiple scsi_debug modules were loaded successfully"
else
        echo "The multiple scsi_debug modules were not loaded - load error"
fi

echo "Check if modules were loaded as required by premeters..."
max_luns=$(cat /sys/bus/pseudo/drivers/scsi_debug/max_luns)
add_host=$(cat /sys/bus/pseudo/drivers/scsi_debug/add_host)
num_tgts=$(cat /sys/bus/pseudo/drivers/scsi_debug/num_tgts)
# echo "max_lunx is $max_luns"
# echo "add_host is $add_host"
# echo "num_tgts is $num_tgts"

premeter_err_ct=0;

if [ $max_luns -ne 2 ];
then
	echo "max_luns was not correct"
	premeter_err_ct=$premeter_err_ct+1;
fi
if [ $add_host -ne 2 ];
then
        echo "add_host was not correct"
	premeter_err_ct=$premeter_err_ct+1;
fi
if [ $num_tgts -ne 2 ];
then
        echo "num_tgts was not correct"
	premeter_err_ct=$premeter_err_ct+1;
fi
if [ $premeter_err_ct -eq 0 ];
then
	echo "multiple scsi_debug was loaded as required premeters"
else
	echo "multip.e scsi_debug was not loaded as required premeters"
fi
echo "scsi_debug first part of test has been done."

echo "Now we are doing fs test for scsi_debug driver..."

cd `dirname $0`
export LTPROOT=${PWD}
echo $LTPROOT | grep testscripts > /dev/null 2>&1
if [ $? -eq 0 ]; then
 cd ..
 export LTPROOT=${PWD}
fi

export TMPBASE="/tmp"

# check if the newly created scsi_debug partitions are in /proc/partitions file
check_count=$(cat /proc/partitions | wc -l)
save_count=$(( $check_count - $orig_count ))
if [ $save_count -lt 4 ]; then
	echo "Not enough scsi_debug partitions to run the test"
	exit
fi

# Get the 4 partitions created by scsi_debug for testing
cat /proc/partitions | awk '{print $4}' | tail -n 4 > $TMPBASE/partition-test
echo "The 4 partitions used to run the test are:"
part1=$(cat $TMPBASE/partition-test | tail -n 1)
echo $part1
part2=$(cat $TMPBASE/partition-test | head -n 3 | tail -n 1)
echo $part2
part3=$(cat $TMPBASE/partition-test | head -n 2 | tail -n 1)
echo $part3
part4=$(cat $TMPBASE/partition-test | head -n 1)
echo $part4

export PATH="${PATH}:${LTPROOT}/testcases/bin"

mkdir /test                   >/dev/null 2>&1
mkdir /test/growfiles         >/dev/null 2>&1
mkdir /test/growfiles/ext2    >/dev/null 2>&1
mkdir /test/growfiles/ext3    >/dev/null 2>&1
mkdir /test/growfiles/reiser  >/dev/null 2>&1
mkdir /test/growfiles/msdos     >/dev/null 2>&1

echo "----- make ext3 fs -----"
mkfs -V -t ext3     /dev/$part1
echo "----- make ext2 fs -----"
mkfs -V -t ext2	    /dev/$part2
echo "----- make reiserfs fs -----"
mkreiserfs -f          /dev/$part3
echo "----- make msdos fs -----"
mkfs -V -t msdos -I     /dev/$part4

echo "----- Mount partitions -----"
mount /dev/$part1 /test/growfiles/ext3
mount /dev/$part2 /test/growfiles/ext2
mount /dev/$part3 /test/growfiles/reiser
mount /dev/$part4 /test/growfiles/msdos

echo "----- Running tests ----- "
echo "The test may take about 2 hours to finish..."
sort -R ${LTPROOT}/runtest/scsi_debug.part1 -o ${TMPBASE}/scsi_debug

${LTPROOT}/bin/ltp-pan -e -S -a scsi_debug -n scsi_debug -l ${TMPBASE}/fs-scsi_debug.log -o ${TMPBASE}/fs-scsi_debug.out -f ${TMPBASE}/scsi_debug

wait $!

umount -v /dev/$part1
umount -v /dev/$part2
umount -v /dev/$part3
umount -v /dev/$part4

echo "Look into /tmp/fs-scsi_debug.log and /tmp/fs-scsi_debug.out for detail results..."
