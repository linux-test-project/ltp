#!/bin/bash
#
# Test script for KVM RAS
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; version
# 2.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should find a copy of v2 of the GNU General Public License somewhere
# on your Linux system; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
# Copyright (C) 2010, Intel Corp.
# Author: Jiajia Zheng <jiajia.zheng@intel.com>
#

image=""
mce_inject_file=""

HOST_DIR="/test"
GUEST_DIR="/test"
host_key_pub="$HOST_DIR/id_rsa.pub"
host_key_priv="$HOST_DIR/id_rsa"
early_kill="1"

kernel=""
initrd=""
root=""

usage()
{
	echo "Usage: ./host_run.sh [-options] [arguments]"
	echo "================Below are the must have options==============="
	echo -e "\t-i image\t: guest image"
	echo -e "\t-f mcefile\t: which mce data file to inject"
	echo "================Below are the optional options================"
	echo -e "\t-d hostdir\t: where you put the test scripts on host system"
	echo -e "\t\t\tBy default, hostdir is set to $HOST_DIR"
	echo -e "\t-g guestdir\t: where you put the test scripts on guest system"
	echo -e "\t\t\tBy default, hostdir is set to $GUEST_DIR"
	echo -e "\t-b pubkey\t: host public key"
	echo -e "\t\t\tBy default, host public key is $host_key_pub"
	echo -e "\t-p privkey\t: host privite key"
	echo -e "\t\t\tBy default, host privite key is $host_key_priv"
	echo -e "\t-o offset\t: guest image offset (optional) "
	echo -e "\t\t\tBy default, offset is calculated by kpartx "
        echo -e "\t-l latekill\t: disable early kill in guest system"
        echo -e "\t\t\tBy default, earlykill is enabled "
	echo "============If you want to specify the guest kernel==========="
	echo "============please set below options all together============="
	echo -e "\t-k kernel\t: guest kernel"
	echo -e "\t-n initrd\t: guest initrd"
	echo -e "\t-r root\t\t: guest root partition"
	exit 0
}

while getopts ":i:f:d:g:o:b:p:k:n:r:h:l" option
do
        case $option in
		i) image=$OPTARG; offset=`kpartx -l $image | awk '{print $NF*512}'`;;
		f) mce_inject_file=$OPTARG;;
		d) HOST_DIR=$OPTARG; host_key_pub=$HOST_DIR/id_rsa.pub; host_key_priv=$HOST_DIR/id_rsa;;
		g) GUEST_DIR=$OPTARG;;
		b) host_key_pub=$OPTARG;;
		p) host_key_priv=$OPTARG;;
		o) offset=$OPTARG;;
		l) early_kill="0";;
		k) kernel=$OPTARG;;
		n) initrd=$OPTARG;;
		r) root=$OPTARG;;
		h) usage;;
		*) echo "invalid option!"; usage;;
        esac
done


guest_script=$GUEST_DIR/guest_run.sh
guest_tmp=$GUEST_DIR/guest_tmp
guest_page=$GUEST_DIR/guest_page
GUEST_PHY=""

guest_init=$HOST_DIR/guest_init
host_start=$HOST_DIR/host_start
pid_file=$HOST_DIR/pid_file
monitor_console=""
serial_console=""
monitor_console_output=$HOST_DIR/monitor_console_output
serial_console_output=$HOST_DIR/serial_console_output
host_tmp=$HOST_DIR/host_tmp
mce_inject_data=$HOST_DIR/mce_inject_data


invalid()
{
	echo $1
	echo "Try \`./host_run.sh -h\` for more information."
	exit 0
}

check_env()
{
	[ -z $image ] && invalid "please input the guest image!"
	[ ! -e $image ] && invalid "guest image $image does not exist!"
	[ -z $mce_inject_file ] && invalid "please input the mce data file!"
	[ ! -e $mce_inject_file ] && invalid "mce data file $mce_inject_file does not exist!"

	[ ! -e $host_key_pub ] && invalid "host public key does not exist!"
	[ ! -e $host_key_priv ] && invalid "host privite key does not exist!"
	chmod 600 $host_key_pub
	chmod 600 $host_key_priv
}

#Guest Image Preparation
image_prepare()
{
	mnt=`mktemp -d`
	mount -oloop,offset=$offset $image $mnt && echo "mount image to $mnt "
	if [ $? -ne 0 ]; then
	    echo "mount image failed!"
	    return 1
	fi

	[ ! -e $mnt$guest_script ] && umount $mnt && invalid "Invalid guest directory!"
	rm -f $mnt/etc/rc3.d/S99kvm_ras
	rm -f $mnt$guest_tmp $mnt$guest_page

	if [ ! -d $mnt/root/.ssh ]; then
	    mkdir $mnt/root/.ssh
	    chmod 700 $mnt/root/.ssh
	fi
	cat $host_key_pub >> $mnt/root/.ssh/authorized_keys
        kvm_ras=/etc/init.d/kvm_ras
	sed "s#EARLYKILL#$early_kill#g" $guest_init | sed "s#GUESTRUN#$guest_script#g" > $mnt$kvm_ras
	chmod a+x $mnt$kvm_ras
	ln -s $kvm_ras $mnt/etc/rc3.d/S99kvm_ras
	sleep 2
	umount $mnt
	sleep 2
	rm -rf $mnt
	return 0
}

#Start guest system
start_guest()
{
	local i
	if [ ! -z $kernel ]; then
	    if [ ! -z $initrd ]; then
		if [ ! -z $root ]; then
		    append="root=$root ro loglevel=8 mce=3 console=ttyS0,115200n8 console=tty0"
	            qemu-system-x86_64 -hda $image -kernel $kernel -initrd $initrd --append "$append" \
		    -net nic,model=rtl8139 -net user,hostfwd=tcp::5555-:22 -monitor pty -serial pty \
		    -pidfile $pid_file > $host_start 2>&1 &
		    sleep 5
		else
		    invalid "please specify the guest root partition!"
		fi
	    else
		invalid "please specify the guest initrd!"
	    fi
	else
	    echo "Start the default kernel on guest system"
	    qemu-system-x86_64 -hda $image -net nic,model=rtl8139 -net user,hostfwd=tcp::5555-:22 \
	    -monitor pty -serial pty -pidfile $pid_file > $host_start 2>&1 &
	    sleep 5
	fi
	monitor_console=`awk '{print $NF}' $host_start | sed -n -e '1p'`
	serial_console=`awk '{print $NF}' $host_start | sed -n -e '2p'`
	QUME_PID=`cat $pid_file`
	echo "monitor console is $monitor_console"
	echo "serial console is $serial_console"
	echo "Waiting for guest system start up..."
}

check_guest_alive()
{
	for i in 1 2 3 4 5 6 7 8 9
	do
	    sleep 10
            ssh -i $host_key_priv localhost -p 5555 echo "" > /dev/null 2>&1
	    if [ $? -eq 0 ]; then
		return 0
	    else
		echo "Waiting..."
	    fi
	done
	return 1
}

addr_translate()
{
	#Get Guest physical address
        scp -i $host_key_priv -P 5555 localhost:$guest_tmp $guest_tmp > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		echo "Failed to get Guest physical address, quit testing!"
		exit 0
	fi
	sleep 2
	GUEST_PHY=`awk '{print $NF}' $guest_tmp`
	echo "Guest physical address is $GUEST_PHY"
	sleep 2

	#Get Host virtual address
	echo p2v $GUEST_PHY > $monitor_console
	cat $monitor_console > $monitor_console_output &
	sleep 5
	HOST_VIRT=`awk '/address/{print $NF}' $monitor_console_output |cut -b 3-11`
	echo "Host virtual address is $HOST_VIRT"

	#Get Host physical address
	page-types/page-types -p $QUME_PID -LN -b anon | grep $HOST_VIRT > $host_tmp
	sleep 5
	ADDR=`cat $host_tmp | awk '{print "0x"$2"000"}' `
	echo "Host physical address is $ADDR"
}

error_inj()
{
	#Inject SRAO error
	cat $mce_inject_file > $mce_inject_data
	echo "ADDR $ADDR" >> $mce_inject_data
	mce-inject $mce_inject_data
}


get_guest_klog()
{
        cat $serial_console > $serial_console_output &
}

check_guest_klog()
{
	GUEST_PHY_KLOG=`echo $GUEST_PHY | sed 's/000$//'`
	echo "Guest physical klog address is $GUEST_PHY_KLOG"
	cat $serial_console_output | grep "MCE $GUEST_PHY_KLOG"
	if [ $? -ne 0 ]; then
		return 1
	fi
	return 0
}



check_env
image_prepare
if [ $? -ne 0 ]; then
    echo "Mount Guest image failed, quit testing!"
else
    start_guest
    get_guest_klog
    check_guest_alive
    if [ $? -ne 0 ]; then
        echo "Start Guest system failed, quit testing!"
    else
	sleep 5
        addr_translate
        error_inj
	sleep 5
	check_guest_klog
	if [ $? -ne 0 ]; then
            echo "FAIL: Did not get expected log!"
	    exit 0
	else
	    echo "PASS: Inject error into guest!"
	fi
	sleep 10
	check_guest_alive
	if [ $? -ne 0 ]; then
            echo "FAIL: Guest System could have died!"
	else
	    echo "PASS: Guest System alive!"
	fi
    fi
fi

rm -f $host_start $monitor_console_output $serail_console_output $host_tmp $pid_file $mce_inject_data
