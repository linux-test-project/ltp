#!/bin/sh
######################################################################
##   Copyright (c) International Business Machines  Corp., 2003
##
##   This program is free software;  you can redistribute it and/or modify
##   it under the terms of the GNU General Public License as published by
##   the Free Software Foundation; either version 2 of the License, or
##   (at your option) any later version.
##
##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY;  without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
##   the GNU General Public License for more details.
##
##   You should have received a copy of the GNU General Public License
##   along with this program;  if not, write to the Free Software
##   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
##
##  FILE   : verify_seclvl.sh
##
##  PURPOSE: To test the compliance of the seclvl module.  This will
##           exit with an error code at the first sign of trouble.
##
##  HISTORY:
##    10/03 Originated by Michael A. Halcrow <mhalcrow@us.ibm.com>
##    02/05 Updated by Mike Halcrow to verify suid and guid permissions
##
######################################################################

if test "${VALID_BLOCK_DEVICE+set}" = set; 
then 
    echo "* VALID_BLOCK_DEVICE=${VALID_BLOCK_DEVICE}";
else
    echo "* Setting VALID_BLOCK_DEVICE to /dev/ubd0.  If this is incorrect, then you need to set that environment variable yourself.";
    VALID_BLOCK_DEVICE=/dev/ubd0
fi

if test "${SYSFS_MOUNT_POINT+set}" = set; 
then 
    echo "* SYSFS_MOUNT_POINT=${SYSFS_MOUNT_POINT}";
else
    echo "* Setting SYSFS_MOUNT_POINT to /sys.  If this is incorrect, then you need to set that environment variable yourself.";
    SYSFS_MOUNT_POINT=/sys
fi

# We must be root
if [ `whoami` = "root" ];
then
    echo "* I am root.  This is good."
else
    echo "* I am not root.  Make me root!!"
    exit 1
fi

# Assume seclvl is built as a module and is not yet inserted

# Make a directory to which to mount the test iso
umount mntseclvl
mkdir -p mntseclvl
mount -o loop seclvl.iso mntseclvl

# Read the first byte from block device, so in case the block device
# test fails, we don't blow away whatever was there first
rm -f firstbyteblock.dat
dd if=${VALID_BLOCK_DEVICE} of=firstbyteblock.dat bs=1 count=1
if [ -e firstbyteblock.dat ]
then
    /bin/true
else
    echo "Error creating firstbyteblock.dat"
    exit 1
fi;

# Same for /dev/mem; /dev/kmem would give an ``invalid address''
# error, so we are living on the edge on that one
rm -f firstbytemem.dat
dd if=/dev/mem of=firstbytemem.dat bs=1 count=1

# We need SHA1 to do our password work
cat /proc/crypto | grep sha1
if [ $? = 0 ]
then
    /bin/true
else
    /sbin/insmod /lib/modules/`uname -r`/kernel/crypto/sha1.ko
    if [ $? = 0 ]
	then
	/bin/true
    else
	echo "SHA1 support not found in kernel; error attempting to perform insmod on sha1.ko"
	exit 1
    fi
fi

# TEST SECLVL -1
echo "* TESTING SECLVL -1"

# Insert seclvl with initlvl -1
insmod /lib/modules/`uname -r`/kernel/security/seclvl.ko initlvl=-1
if [ $? != 0 ]
then
    echo "* Error insmod'ing seclvl.ko with initlvl=-1"
    echo "* Maybe the module doesn't exist?"
    echo "* Maybe it's compiled into the kernel?"
    echo "* Maybe it's already loaded?"
    echo "* This test assumes that seclvl is built as a module and is not yet loaded."
    echo "* If one of those isn't the problem, maybe seclvl is broken."
    exit 1
fi

# Test /$(SYSFS_MOUNT_POINT)/seclvl/seclvl
#  Existence
if [ -e /$(SYSFS_MOUNT_POINT)/seclvl/seclvl ]
then
    /bin/true
else
    echo "* /$(SYSFS_MOUNT_POINT)/seclvl/seclvl does not exist"
    exit 1
fi
#  Value
if test `cat /$(SYSFS_MOUNT_POINT)/seclvl/seclvl` = -1
then
    /bin/true
else
    echo "* /$(SYSFS_MOUNT_POINT)/seclvl/seclvl does not contain -1"
    exit 1
fi

# Remove seclvl
/sbin/rmmod seclvl
if [ $? != 0 ]
then
    echo "* Error removing seclvl module in seclvl=-1"
fi

# Test /$(SYSFS_MOUNT_POINT)/seclvl/seclvl existence
if [ -e /$(SYSFS_MOUNT_POINT)/seclvl/seclvl ]
then
    echo "* /$(SYSFS_MOUNT_POINT)/seclvl/seclvl still exists after module removal"
    exit 1
else
    /bin/true
fi


# TEST SECLVL 0
echo "* TESTING SECLVL 0"

# Insert seclvl with initlvl 0
/sbin/insmod /lib/modules/`uname -r`/kernel/security/seclvl.ko initlvl=0
if [ $? != 0 ]
then
    echo "* Error insmod'ing seclvl.ko with initlvl=0"
    echo "* Maybe the module doesn't exist?"
    echo "* Maybe it's compiled into the kernel?"
    echo "* Maybe it's already loaded?"
    echo "* This test assumes that seclvl is built as a module and is not yet loaded."
    echo "* If one of those isn't the problem, maybe seclvl is broken."
    exit 1
fi

# Test /$(SYSFS_MOUNT_POINT)/seclvl/seclvl
#  Existence
if [ -e /$(SYSFS_MOUNT_POINT)/seclvl/seclvl ]
then
    /bin/true
else
    echo "* /$(SYSFS_MOUNT_POINT)/seclvl/seclvl does not exist"
    exit 1
fi
# Value
if test `cat /$(SYSFS_MOUNT_POINT)/seclvl/seclvl` = 0
then
    /bin/true
else
    echo "* /$(SYSFS_MOUNT_POINT)/seclvl/seclvl does not contain 0"
    exit 1
fi

# Remove seclvl
/sbin/rmmod seclvl
if [ $? != 0 ]
then
    echo "* Error removing seclvl module in seclvl=0"
fi

# Test /$(SYSFS_MOUNT_POINT)/seclvl/seclvl
#  Existence
if [ -e /$(SYSFS_MOUNT_POINT)/seclvl/seclvl ]
then
    echo "* /$(SYSFS_MOUNT_POINT)/seclvl/seclvl still exists after module removal"
    exit 1
else
    /bin/true
fi

# TEST SECLVL 1
echo "* TESTING SECLVL 1"

# Insert seclvl with default initlvl (should be 1)
/sbin/insmod /lib/modules/`uname -r`/kernel/security/seclvl.ko passwd=secret
if [ $? != 0 ]
then
    echo "* Error insmod'ing seclvl.ko"
    echo "* Maybe the module doesn't exist?"
    echo "* Maybe it's compiled into the kernel?"
    echo "* Maybe it's already loaded?"
    echo "* This test assumes that seclvl is built as a module and is not yet loaded."
    echo "* If one of those isn't the problem, maybe seclvl is broken."
    exit 1
fi

# Test /$(SYSFS_MOUNT_POINT)/seclvl/seclvl
#  Existence
if [ -e /$(SYSFS_MOUNT_POINT)/seclvl/seclvl ]
then
    /bin/true
else
    echo "* /$(SYSFS_MOUNT_POINT)/seclvl/seclvl does not exist"
    exit 1
fi
# Value
if test `cat /$(SYSFS_MOUNT_POINT)/seclvl/seclvl` = 1
then
    /bin/true
else
    echo "* /$(SYSFS_MOUNT_POINT)/seclvl/seclvl does not contain 1 (should be the default)"
    exit 1
fi

# Attempt to remove module
echo "* \`\`Operation not permitted'' message is expected:"
/sbin/rmmod seclvl
echo "* seclvl should be listed below:"
/sbin/lsmod | grep seclvl
if [ $? != 0 ]
then
    echo "* seclvl module successfully removed in seclvl=1"
    exit 1
fi

# Attempt to lower secure level
echo "-1" > /$(SYSFS_MOUNT_POINT)/seclvl/seclvl
if [ $? = 0 ]
then
    echo "* Success result when setting seclvl from 1 to -1"
    exit 1
fi
echo "0" > /$(SYSFS_MOUNT_POINT)/seclvl/seclvl
if [ $? = 0 ]
then
    echo "* Success result when setting seclvl from 1 to 0"
    exit 1
fi

if test `cat /$(SYSFS_MOUNT_POINT)/seclvl/seclvl` = 1
then
    /bin/true
else
    echo "* /$(SYSFS_MOUNT_POINT)/seclvl/seclvl does not contain 1"
    exit 1
fi

# The character devices /dev/mem and /dev/kmem cannot be written to
echo "* \`\`Operation not permitted'' message is expected:"
cat firstbytemem.dat > /dev/mem
if [ $? = 0 ]
then
    echo "* Succeeded in writing to /dev/mem in seclvl 1"
    exit 1
fi

echo "* \`\`Operation not permitted'' message is expected:"
echo "*  " > /dev/kmem
if [ $? = 0 ]
then
    echo "* Succeeded in writing to /dev/kmem in seclvl 1"
    exit 1
fi

# IMMUTABLE and APPEND extended attributes may not be set
rm -f answer.txt
echo "42" > answer.txt
if [ -e answer.txt ]
then
    /bin/true
else
    echo "Error creating answer.txt"
    exit 1
fi;
echo "* \`\`Operation not permitted'' message is expected:"
chattr +i answer.txt
lsattr answer.txt | grep "\-\-\-\-i\-\-\-\-\-\-\-\-\-\-\-\- answer.txt"
if [ $? = 0 ]
then
    echo "* Succeeded in setting the immutable flag on answer.txt"
    exit 1
fi
echo "* \`\`Operation not permitted'' message is expected:"
chattr +a answer.txt
lsattr answer.txt | grep "\-\-\-\-\-a\-\-\-\-\-\-\-\-\-\-\- answer.txt"
if [ $? = 0 ]
then
    echo "* Succeeded in setting the append flag on answer.txt"
    exit 1
fi
rm -f answer.txt

# Module operations test previously performed by attempt to remove seclvl module

# Attempt to change I/O permission level
if [ -e ioperm ]
then
    /bin/true
else
    echo "* ioperm doesn't exit.  Try running make."
    exit 1
fi
echo "* Return code [-1] expected:"
./ioperm
if [ $? = 0 ]
then
    echo "* Succeeded in changing I/O permission level in seclvl 1"
    exit 1
fi
if [ -e ioperm ]
then
    /bin/true
else
    echo "* iopl doesn't exit.  Try running make."
    exit 1
fi
echo "* Return code [-1] expected:"
./iopl
if [ $? = 0 ]
then
    echo "* Succeeded in changing I/O permission level in seclvl 1"
    exit 1
fi

# Mess with a network configuration option
echo "* Ignore any FATAL errors:"
/usr/sbin/iptables -F
if [ $? = 0 ]
then
    echo "* Succeeded in flushing firewall rules via iptables in seclvl 1"
    exit 1
fi

# Try setting the setuid bit on /bin/true
echo "* \`\`Operation not permitted'' message is expected:"
chmod u+s /bin/true
if [ $? = 0 ]
then
    echo "* Succeeded in raising the setuid bit on /bin/true in seclvl 1"
    echo "* Lowering setuid bit on /bin/true..."
    chmod u-s /bin/true    
    exit 1
fi

# Try setting the setgid bit on /bin/true
echo "* \`\`Operation not permitted'' message is expected:"
chmod g+s /bin/true
if [ $? = 0 ]
then
    echo "* ERROR: Succeeded in raising the setgid bit on /bin/true in seclvl 1"
    echo "* Lowering setgid bit on /bin/true..."
    chmod g-s /bin/true    
    exit 1
fi

# Try setting the setgid bit on a directory; verify success
echo "* Verifying that we can set the suid bit on a directory:"
rmdir seclvl_suid_dir
mkdir seclvl_suid_dir
chmod 4777 seclvl_suid_dir
if [ $? = 0 ]
then
    /bin/true
else
    echo "* Error setting the suid bit on a directory in seclvl 1; should be allowed.  You are probably using an older version of seclvl."
    rmdir seclvl_suid_dir
    exit 1
fi
chmod 2777 seclvl_suid_dir
if [ $? = 0 ]
then
    /bin/true
else
    echo "* Error setting the guid bit on a directory in seclvl 1; should be allowed.  You are probably using an older version of seclvl."
    rmdir seclvl_suid_dir
    exit 1
fi
rmdir seclvl_suid_dir

echo "* Attempt to create suid file:"
rm -f suid_file
./create_suid_file
if [ $? = 0 ]
then
    echo "* ERROR: Success in creating suid file in seclvl 1"
    rm -f suid_file
    exit 1
else
    /bin/true
fi

echo "* Attempt to create guid file:"
rm -f guid_file
./create_guid_file
if [ $? = 0 ]
then
    echo "* ERROR: Success in creatings guid file in seclvl 1"
    rm -f guid_file
    exit 1
else
    /bin/true
fi

echo "* Attempt to create suid node:"
rm -f suid_node
./create_suid_node
if [ $? = 0 ]
then
    echo "* ERROR: Success in creating suid node in seclvl 1"
    rm -f suid_node
    exit 1
else
    /bin/true
fi

echo "* Attempt to create guid node:"
rm -f guid_node
./create_guid_node
if [ $? = 0 ]
then
    echo "* ERROR: Success in creating guid node in seclvl 1"
    rm -f guid_node
    exit 1
else
    /bin/true
fi

# TEST SECLVL 2
echo "* TESTING SECLVL 2"

# Raise the seclvl
echo "2" > /$(SYSFS_MOUNT_POINT)/seclvl/seclvl
if [ $? = 0 ]
then
    /bin/true
else
    echo "* Error result attempting to raise seclvl to 2"
    exit 1
fi

# Test /$(SYSFS_MOUNT_POINT)/seclvl/seclvl (make sure it didn't go away after raising the seclvl)
#  Existence
if [ -e /$(SYSFS_MOUNT_POINT)/seclvl/seclvl ]
then
    /bin/true
else
    echo "* /$(SYSFS_MOUNT_POINT)/seclvl/seclvl does not exist"
    exit 1
fi
# Value
if test `cat /$(SYSFS_MOUNT_POINT)/seclvl/seclvl` = 2
then
    /bin/true
else
    echo "* /$(SYSFS_MOUNT_POINT)/seclvl/seclvl does not contain 2"
    exit 1
fi

# Attempt to remove module
echo "* \`\`Operation not permitted'' message is expected:"
/sbin/rmmod seclvl
echo "* seclvl should be listed below:"
/sbin/lsmod | grep seclvl
if [ $? != 0 ]
then
    echo "* seclvl module successfully removed in seclvl=2"
    exit 1
fi

# Attempt to lower secure level
echo "-1" > /$(SYSFS_MOUNT_POINT)/seclvl/seclvl
if [ $? = 0 ]
then
    echo "* Success result when setting seclvl from 2 to -1"
    exit 1
fi
echo "0" > /$(SYSFS_MOUNT_POINT)/seclvl/seclvl
if [ $? = 0 ]
then
    echo "* Success result when setting seclvl from 2 to 0"
    exit 1
fi
echo "1" > /$(SYSFS_MOUNT_POINT)/seclvl/seclvl
if [ $? = 0 ]
then
    echo "* Success result when setting seclvl from 2 to 1"
    exit 1
fi

if test `cat /$(SYSFS_MOUNT_POINT)/seclvl/seclvl` = 2
then
    /bin/true
else
    echo "* /$(SYSFS_MOUNT_POINT)/seclvl/seclvl does not contain 2"
    exit 1
fi

# The character devices /dev/mem and /dev/kmem cannot be written to
echo "* \`\`Operation not permitted'' message is expected:"
cat firstbytemem.dat > /dev/mem
if [ $? = 0 ]
then
    echo "* Succeeded in writing to /dev/mem in seclvl 2"
    exit 1
fi

echo "* \`\`Operation not permitted'' message is expected:"
echo " " > /dev/kmem
if [ $? = 0 ]
then
    echo "* Succeeded in writing to /dev/kmem in seclvl 2"
    exit 1
fi

# IMMUTABLE and APPEND extended attributes may not be set
echo "42" > answer.txt
if [ -e answer.txt ]
then
    /bin/true
else
    echo "Error creating answer.txt"
    exit 1
fi;
echo "* \`\`Operation not permitted'' message is expected:"
chattr +i answer.txt
lsattr answer.txt | grep "\-\-\-\-i\-\-\-\-\-\-\-\-\-\-\-\- answer.txt"
if [ $? = 0 ]
then
    echo "* Succeeded in setting the immutable flag on answer.txt"
    exit 1
fi
echo "* \`\`Operation not permitted'' message is expected:"
chattr +a answer.txt
lsattr answer.txt | grep "\-\-\-\-\-a\-\-\-\-\-\-\-\-\-\-\- answer.txt"
if [ $? = 0 ]
then
    echo "* Succeeded in setting the append flag on answer.txt"
    exit 1
fi

# Module operations test previously performed by attempt to remove seclvl module

# Attempt to change I/O permission level
if [ -e ioperm ]
then
    /bin/true
else
    echo "* ioperm doesn't exit.  Try running make."
    exit 1
fi
echo "* Return code [-1] expected:"
./ioperm
if [ $? = 0 ]
then
    echo "* Succeeded in changing I/O permission level in seclvl 2"
    exit 1
fi
if [ -e ioperm ]
then
    /bin/true
else
    echo "* iopl doesn't exit.  Try running make."
    exit 1
fi
echo "* Return code [-1] expected:"
./iopl
if [ $? = 0 ]
then
    echo "* Succeeded in changing I/O permission level in seclvl 2"
    exit 1
fi

# Mess with a network configuration option
echo "* Ignore FATAL errors:"
/usr/sbin/iptables -F
if [ $? = 0 ]
then
    echo "* Succeeded in flushing firewall rules via iptables in seclvl 2"
    exit 1
fi

# Try setting the setuid bit on /bin/true
echo "* \`\`Operation not permitted'' message is expected:"
chmod u+s /bin/true
if [ $? = 0 ]
then
    echo "* Succeeded in raising the setuid bit on /bin/true in seclvl 2"
    echo "* Lowering setuid bit on /bin/true..."
    chmod u-s /bin/true    
    exit 1
fi

# Try setting the setgid bit on /bin/true
echo "* \`\`Operation not permitted'' message is expected:"
chmod g+s /bin/true
if [ $? = 0 ]
then
    echo "* Succeeded in raising the setgid bit on /bin/true in seclvl 1"
    echo "* Lowering setgid bit on /bin/true..."
    chmod g-s /bin/true    
    exit 1
fi

# Try setting the setgid bit on a directory; verify success
echo "* Verifying that we can set the suid bit on a directory:"
rmdir seclvl_suid_dir
mkdir seclvl_suid_dir
chmod 4777 seclvl_suid_dir
if [ $? = 0 ]
then
    /bin/true
else
    echo "* Error setting the suid bit on a directory in seclvl 2; should be allowed.  You are probably using an older version of seclvl."
    rmdir seclvl_suid_dir
    exit 1
fi
chmod 2777 seclvl_suid_dir
if [ $? = 0 ]
then
    /bin/true
else
    echo "* Error setting the guid bit on a directory in seclvl 2; should be allowed.  You are probably using an older version of seclvl."
    rmdir seclvl_suid_dir
    exit 1
fi
rmdir seclvl_suid_dir

echo "* Attempt to create suid file:"
rm -f suid_file
./create_suid_file
if [ $? = 0 ]
then
    echo "* ERROR: Success in creating suid file in seclvl 2"
    rm -f suid_file
    exit 1
else
    /bin/true
fi

echo "* Attempt to create guid file:"
rm -f guid_file
./create_guid_file
if [ $? = 0 ]
then
    echo "* ERROR: Success in creatings guid file in seclvl 2"
    rm -f guid_file
    exit 1
else
    /bin/true
fi

echo "* Attempt to create suid node:"
rm -f suid_node
./create_suid_node
if [ $? = 0 ]
then
    echo "* ERROR: Success in creating suid node in seclvl 2"
    rm -f suid_node
    exit 1
else
    /bin/true
fi

echo "* Attempt to create guid node:"
rm -f guid_node
./create_guid_node
if [ $? = 0 ]
then
    echo "* ERROR: Success in creating guid node in seclvl 2"
    rm -f guid_node
    exit 1
else
    /bin/true
fi

# Test to assure that system time cannot be decremented
# TODO: This is broken on 2/20/2004 @ 10:15am
echo "* \`\`Operation not permitted'' message is expected:"
echo "`date +%m%d%H%M%Y`-1" | bc | awk '{printf( "%.12d\n", $1 );}' | xargs date
if [ $? = 0 ]
then
    echo "* Succeeded in setting the clock back one year in seclvl 2"
    echo "`date +%m%d%H%M%Y`+1" | bc | awk '{printf( "%.12d\n", $1 );}' | xargs date
    exit 1
fi

# Attempt to open a block device and a character device for writing
# WARNING: If seclvl isn't working, this can bork your hard disk
echo "* \`\`Operation not permitted'' message is expected:"
cat firstbyteblock.dat > $(VALID_BLOCK_DEVICE)
if [ $? = 0 ]
then
    echo "* Succeeded in writing directly to $(VALID_BLOCK_DEVICE) in seclvl 2"
    exit 1
fi

# Attempt to unmount previously mounted filesystem
# Now unmount it
echo "* The kernel says it's not mounted, but it's lying."
echo "* It really is, but you don't have permission to unmount it:"
umount mntseclvl
if [ $? = 0 ]
then
    echo "* Succeeded in unmounting previously mounted filesystem in seclvl 2"
    exit 1
fi

# Break and remove seclvl via password
echo "* Breaking seclvl via password"
echo -n "secret" > /$(SYSFS_MOUNT_POINT)/seclvl/passwd
echo "* Removing seclvl"
/sbin/rmmod seclvl
if [ $? = 0 ]
then
    /bin/true
else
    echo "Error removing seclvl"
    exit 1
fi

# Test sha1_passwd-based breakage of seclvl
echo "* Performing insmod with sha1_passwd parameter set"
/sbin/insmod /lib/modules/`uname -r`/kernel/security/seclvl.ko sha1_passwd=0b8c31dd3a4c1e74b0764d5b510fd5eaac00426c
if [ -e /$(SYSFS_MOUNT_POINT)/seclvl/passwd ]
then
    /bin/true
else
    echo "/$(SYSFS_MOUNT_POINT)/seclvl/passwd does not exist after seclvl insmod'ed with sha1_passwd parameter set"
    exit 1
fi

# Break seclvl via password
echo "abracadabra" > /$(SYSFS_MOUNT_POINT)/seclvl/passwd
rmmod seclvl
if [ $? = 0 ]
then
    /bin/true
else
    echo "Error removing seclvl module after password-based secre level reduction"
    exit 1
fi

echo "Seclvl test pass!"

# Cleanup
umount mntseclvl
rm -f firstbyteblock.dat
rm -f answer.txt
# rm -f firstbytemem.dat # This fails in seclvl 2...

exit 0
