#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2001                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
## File:        runalltests.sh                                                ##
##                                                                            ##
## Description:  This script just calls runltp now, and is being phased out.  ##
##		If you rely on this script for automation reasons, please     ##
##                                                                            ## 
## History	Subrata Modak <subrata@linuc.vnet.ibm.com> changed the code   ##
##		to include testing other testcases which are not run by       ##
##		default, 08/09/2008                                           ##
##                                                                            ##
################################################################################

export LTPROOT=${0%/*}
RUNLTP="$LTPROOT/runltp"

echo  "*******************************************************************"
echo  "*******************************************************************"
echo  "**								**"
echo  "** This script is being re-written to cover all aspects of	**"
echo  "** testing LTP, which includes running all those tests which	**"
echo  "** are not run by default with ${RUNLTP##*/} script. Special setup	**"
echo  "** in system environment will be done to run all those tests	**"
echo  "** like the File System tests, SELinuxtest, etc			**"
echo  "**								**"
echo  "*******************************************************************"
echo  "*******************************************************************"

export RUN_BALLISTA=0
export RUN_OPENPOSIX=0
## Set this to 1 if mm-1.4.2.tar.gz is already installed in your system
## from ftp://ftp.ossp.org/pkg/lib/mm/mm-1.4.2.tar.gz
export RUN_MM_CORE_APIS=0
export LIBMM_INSTALLED=0
## This is required if already not set to /usr/local/lib/ as
## mm-1.4.2.tar.gz gets installed at /usr/local/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/:/lib/
export PATH=$PATH:/sbin/

## Set this to 1 if libaio-0.3.92.tar.gz is already installed in your system
## from http://www.kernel.org/pub/linux/kernel/people/bcrl/aio/libaio-0.3.92.tar.gz
export RUN_AIOTESTS=0
export LIBAIO_INSTALLED=0

## Set this to 1 if libcaps-2.11 or newer is already installed in your system
## from ftp://ftp.kernel.org/pub/linux/libs/security/linux-privs/libcap2, as well as,
## the libattr is available in the system. The kernel should also have been built
## with the following option: CONFIG_SECURITY_FILE_CAPABILITIES=y
export RUN_FILECAPS=0
export LIBCAPS_INSTALLED=0
export LIBATTR_INSTALLED=0

## Set this to 1 if you wish to execute the stress_cd tests
## Make sure you have FLOPPY inserted, be warned that you
## will loose all data on it after the tests,
export RUN_STRESS_FLOPPY=0

## Set this to 1 if you wish to execute the stress_floppy tests
## Make sure you have CD inserted in your CD-ROM drive,
export RUN_STRESS_CD=0

##Set this to 1 if you wish to run the CPUHOTPLUG tests
export RUN_CPU_HOTPLUG=0

##Set this to 1 if you wish to run the LTP-NETWORK tests. Please refer to:
## http://ltp.cvs.sourceforge.net/viewvc/ltp/ltp/testcases/network/LTP-Network-test_README.pdf
## for more information on how to run the tests.
export RUN_LTP_NETWORK_TESTS=0

##Set this to 1 if you wish to run the LTP-NETWORK-STRESS tests. Please refer to:
## http://ltp.cvs.sourceforge.net/viewvc/ltp/ltp/testcases/network/LTP-Network-test_README.pdf
## and http://ltp.cvs.sourceforge.net/viewvc/ltp/ltp/testcases/network/stress/README
## for more information on how to run the NETWORK-STRESS tests.
export RUN_LTP_NETWORK_STRESS_TESTS=0

## Set this to 1 if you wish to run the ltp/testscripts/adp tests
export RUN_LTP_ADP_TESTS=0

## Set this to 1 if you wish to run the AUTOFS tests
#  REQUIREMENTS:
#   1) System with a floppy device with a floppy disk in it.
#   2) A spare (scratch) disk partition of 100MB or larger.
export RUN_LTP_AUTOFS1_TESTS=0
export RUN_LTP_AUTOFS4_TESTS=0
export DISK_PARTITION1=0

## Set this to 1 if you wish to run the EXPORTFS tests
#  DESCRIPTION : A script that will test exportfs on Linux system.
#  REQUIREMENTS:
#   1) NFS Server system with rsh enabled between client & server.
#   2) 100MB Disk partition on NFS server.
export RUN_EXPORTFS_TESTS=0
export NFS_SERVER1=xxx
export NFS_SERVER_DISK_PARTITION1=yyy
export NFS_SERVER_FS_TYPE1=zzz


## Set this to 1 if you wish to run the FS tests on READ ONLY File Systems. Refer to http://ltp.cvs.sourceforge.net/viewvc/ltp/ltp/testscripts/Readme_ROBind for more info
export RUN_RO_ONLY_FS_TESTS=0
export READ_ONLY_DIRECTORY1=xxxx

## Set this to 1 if you wish to run the ISOFS tests
#  REQUIREMENTS:
#   Must have root access to execute this script
export RUN_ISOFS_TESTS=0

## Set this to 1 if you wish to run the DMMAPPER tests
#Note: In order to run this test, you must turn on "device mapper"
#      component in kernel (it is under device drivers item when you
#      run make menuconfig); and you must install userspace supporting
#      files (libdevmapper and dmsetup). They are in the device-mapper
#      package. You can download it from http://www.sistina.com. Follow
#      the README/INSTALL file within the package to install it.
export RUN_DMMAPPER_TESTS=0
export DISK_PARTITION2=xxxxx
export DISK_PARTITION3=yyyyy

## Set this to 1 if you wish to run the FSLVM tests
#Note: fdisk needs to be run and the 4 HD partitions marked as 0x8e -- Linux LVM
#      - If this is run on a 2.4 kernel system then LVM must be configured and the kernel rebuilt. In a 2.5 environment
#        you must configure Device Mapper and install LVM2 from www.systina.com for the testcase to run correctly.
#      - These operations are destructive so do NOT point the tests to partitions where the data shouldn't be overwritten.
#        Once these tests are started all data in the partitions you point to will be destroyed.
export RUN_FSLVM_TESTS=0
export DISK_PARTITION4=xxxxxx
export DISK_PARTITION5=yyyyyy
export DISK_PARTITION6=zzzzzz
export DISK_PARTITION7=iiiiii
export NFS_PARTITION1=jjjjjj

## Set this to 1 if you wish to run the FSNOLVM tests
#Note: fdisk needs to be run and the 4 HD partitions marked as 0x8e -- Linux LVM
#      - If this is run on a 2.4 kernel system then LVM must be configured and the kernel rebuilt. In a 2.5 environment
#        you must configure Device Mapper and install LVM2 from www.systina.com for the testcase to run correctly.
#      - These operations are destructive so do NOT point the tests to partitions where the data shouldn't be overwritten.
#        Once these tests are started all data in the partitions you point to will be destroyed.
export RUN_FSNOLVM_TESTS=0

## Set this to 1 if you wish to run the LTP SCSI DEBUG tests
#Note: Build scsi_debug as a module first before running the test
export RUN_LTP_SCSI_DEBUG_TEST=0

## Set this to 1 if you wish to run the LTP SYSFS tests
#Note: Must have root access to execute this script. 
#  USAGE       : sysfs.sh [ -k <kernel_module> ]
#  DESCRIPTION : A script that will test sysfs on Linux system.
#  REQUIREMENTS: CONFIG_DUMMY must have been used to build kernel, and the 
#                dummy network module must exist.
export RUN_LTP_SYSFS_TEST=0
export KERNEL_MODULE1=xxxxxxx

## Set this to 1 if you wish to run the LTP TIRPC tests
export RUN_LTP_TIRPC_TEST=0

##Set this to 1 if you wish to run the SE-Linux tests
# These testcases test the SELinux Security Module.
# A kernel with SELinux configured, and SELinux policy and userspace
# tools installed, are required to build and run the SELinux testsuite.
# Also, /usr/sbin should be included in PATH to ensure SELinux utilities
# such as getenforce are found. The test_selinux.sh script adds /usr/sbin
# to the PATH.
# You must also have the line: 
#	expand-check = 0
# in your /etc/selinux/semanage.conf file as the test policy will violate some
# of the neverallow rules in the base policy.  This line may already be present
# depending on your distribution; if not, add it before running the test suite
# and remove it when done. For more info, please refer to:
# http://ltp.cvs.sourceforge.net/viewvc/ltp/ltp/testcases/kernel/security/selinux-testsuite/README
export RUN_SE_LINUX_TESTS=0

##Set this to 1 if you wish to run the dma_thread_diotest7 test
export RUN_DMA_THREAD_DIOTEST7=0

##Set this to 1 if you wish to run the Controller area network (CAN)
##protocol support tests. You would also like to review the Kernel
##config options need to be set for this at ltp/README
export RUN_CONTROLLER_AREA_NETWORK_TESTS=0

##Set this to 1 if you wish to run the SMACK Security tests
## Remember that you cannot run both the SELINUX and SMACK tests at a time,
## as both of them cannot be tested in the same running kernel
export RUN_SMACK_SECURITY_TESTS=0

##Set this to 1 if you wish to run the Basic PERFORMANCE COUNTER tests
export RUN_PERFORMANCE_COUNTERS_TESTS=0

export LTP_VERSION=`"${RUNLTP}" -e`
export TEST_START_TIME=`date +"%Y_%b_%d-%Hh_%Mm_%Ss"`
export HARDWARE_TYPE=$(uname -i)
export HOSTNAME=$(uname -n)
export KERNEL_VERSION=$(uname -r)
export HTML_OUTPUT_FILE_NAME=$LTP_VERSION_$HOSTNAME_$KERNEL_VERSION_$HARDWARE_TYPE_$TEST_START_TIME.html

if ! cd "${LTPROOT}"; then
	rc=$?
	echo "${0##*/}: ERROR : Could not cd to ${LTPROOT}"
	exit $rc
fi

## The First one i plan to run is the default LTP run ##
## START => Test Series 1                             ##
echo "Running Default LTP..."
"${RUNLTP}" -g $HTML_OUTPUT_FILE_NAME
printf "Completed running Default LTP\n\n\n"
## END => Test Series 1                               ##

## The next one i plan to run is ballista             ##
## START => Test Series 2                             ##
if [ $RUN_BALLISTA -eq 1 ]
then
	echo "Running Ballista..."
	export TEST_START_TIME=`date +"%Y_%b_%d-%Hh_%Mm_%Ss"`
	"${RUNLTP}" -f ballista -o $LTP_VERSION-BALLISTA_RUN_ON-$HOSTNAME-$KERNEL_VERSION-$HARDWARE_TYPE-$TEST_START_TIME.out
	printf "Completed running Ballista\n\n\n"
fi
## END => Test Series 2                               ##

## The next one i plan to run is open_posix_testsuite ##
## START => Test Series 3                             ##
if [ $RUN_OPENPOSIX -eq 1 ]
then
	echo "Running Open Posix Tests..."
	(cd testcases/open_posix_testsuite; make)
	printf "Completed running Open Posix Tests\n\n\n"
fi
## END => Test Series 3                               ##


## The next one i plan to run is                      ##
## ltp/testcases/kernel/mem/libmm/mm_core_apis        ##
## START => Test Series 4                             ##
if [ $RUN_MM_CORE_APIS -eq 1 ]
then
	echo "Initializing ltp/testcases/kernel/mem/libmm/mm_core_apis ..."
	# Check to see if User is Root
	if [ $(id -ru) -ne 0 ]
	then
		echo -n "You need to be root to Install libmm and run "
		echo -n "mem/libmm/mm_core_apis; aborting "
		echo "ltp/testcases/kernel/mem/libmm/mm_core_apis"
	else
		if [ $LIBMM_INSTALLED -ne 1 ]
		then
			echo Installing libmm-1.4.2 .............
			(cd /tmp;
			 wget -c ftp://ftp.ossp.org/pkg/lib/mm/mm-1.4.2.tar.gz;
			 tar -xzf mm-1.4.2.tar.gz;
			 cd mm-1.4.2;
			 ./configure && make all install)
			rm -rf /tmp/mm-1.4.2*
			echo "libmm-1.4.2 Installed ............."
		else
			echo "libmm-1.4.2 already installed in your system"
		fi
			echo -n "Running "
			echo "ltp/testcases/kernel/mem/libmm/mm_core_apis ..."
	   (make -C testcases/kernel/mem/libmm all install;
	    $LTPROOT/testcases/bin/mm_core_apis; )
	fi
	printf "Completed running ltp/testcases/kernel/mem/libmm/mm_core_apis...\n\n\n"
fi
## END => Test Series 4                               ##


## The next one i plan to run is                      ##
## ltp/testcases/kernel/io/aio                        ## 
## START => Test Series 5                             ##
if [ $RUN_AIOTESTS -eq 1 ]
then
	echo "Initializing ltp/testcases/kernel/io/aio ..."
	# Check to see if User is Root
	if [ $(id -ru) -ne 0 ]
	then
		echo -n "You need to be root to Install libaio-0.3.92 and run"
		echo -n "ltp/testcases/kernel/io/aio; aborting "
		echo "ltp/testcases/kernel/io/aio"
	else
		if [ $LIBAIO_INSTALLED -ne 1 ]
		then
			echo "Installing libaio-0.3.92............."
			(cd /tmp;
			 wget -c http://www.kernel.org/pub/linux/kernel/people/bcrl/aio/libaio-0.3.92.tar.gz;
			 tar -xzf libaio-0.3.92.tar.gz;
			 make -C libaio-0.3.92 all install)
			rm -rf /tmp/libaio-0.3.92*
			echo "libaio-0.3.92 Installed ............."
		else
			echo "libaio-0.3.92 already installed in your system"
		fi
		# XXX (garrcoop): this needs to be fixed so that it's callable
		# via standalone runltp in a standard installed tree.
		echo "Building & Running ltp/testcases/kernel/io/aio..."
		(make -C testcases/kernel/io/aio all;
		 ./aio01/aio01;
		 ./aio02/runfstests.sh -a aio02/cases/aio_tio;
		 make clean 1>&2 > /dev/null )
		printf "Completed running ltp/testcases/kernel/io/aio...\n\n\n"
	fi
fi
## END => Test Series 5                               ##

## The next one i plan to run is                      ##
## ltp/testcases/kernel/security/filecaps             ## 
## START => Test Series 6                             ##
if [ $RUN_FILECAPS -eq 1 ]
then
	echo "Initializing ltp/testcases/kernel/security/filecaps ..."
	# Check to see if User is Root
	if [ $(id -ru) -ne 0 ]
	then
		echo -n "You need to be root to Install libcaps and run "
		echo -n "ltp/testcases/kernel/security/filecaps; aborting "
		echo "ltp/testcases/kernel/security/filecaps"
	else
		if [ $LIBCAPS_INSTALLED -ne 1 ]
		then
			echo "Installing libcaps............."
			(cd /tmp;
			 wget -c ftp://ftp.kernel.org/pub/linux/libs/security/linux-privs/libcap2/libcap-2.14.tar.gz;
			 tar -xzf libcap-2.14.tar.gz;
			 make -C libcap-2.14 all install)
			rm -rf /tmp/libcap-2.14*
			echo "libcaps Installed ............."
		else
			echo "libcaps already installed in your system"
		fi
		echo -n "Building & Running "
		echo "ltp/testcases/kernel/security/filecaps"
		make -C ltp/testcases/kernel/security/filecaps all install
		"${RUNLTP}" -f filecaps
		printf "Completed running ltp/testcases/kernel/io/aio...\n\n\n"

	fi

fi
## END => Test Series 6                               ##


## The next one i plan to run is tpm_tools            ##
## START => Test Series 7                             ##
"${RUNLTP}" -f tpm_tools
## END => Test Series 7                               ##

## The next one i plan to run is tcore_patch_test_suites
## START => Test Series 8                             ##
"${RUNLTP}" -f tcore
## END => Test Series 8                               ##


## The next one i plan to run is stress_cd tests
## START => Test Series 9                             ##
if [ $RUN_STRESS_CD -eq 1 ]
then
	"${RUNLTP}" -f io_cd
fi
## END => Test Series 9                               ##

## The next one i plan to run is stress_floppy tests
## START => Test Series 10                             ##
if [ $RUN_STRESS_FLOPPY -eq 1 ]
then
	"${RUNLTP}" -f io_floppy
fi
## END => Test Series 10                               ##

## The next one i plan to run is CPUHOTPLUG tests
## START => Test Series 11                             ##
if [ $RUN_CPU_HOTPLUG -eq 1 ]
then
	"${RUNLTP}" -f cpuhotplug
fi
## END => Test Series 11                               ##

## The next one i plan to run are the LTP Network tests
## START => Test Series 12                             ##
if [ $RUN_LTP_NETWORK_TESTS -eq 1 ]
then
	(cd $LTPROOT/testscripts/; ./networktests.sh)
fi
## END => Test Series 12                               ##

## The next one i plan to run are the LTP Network Stress tests
## START => Test Series 13                             ##
if [ $RUN_LTP_NETWORK_STRESS_TESTS -eq 1 ]
then
	(cd $LTPROOT/testscripts/; ./networkstress.sh)
fi
## END => Test Series 13                               ##


## The next one i plan to run are the LTP ADP tests
## START => Test Series 14                             ##
if [ $RUN_LTP_ADP_TESTS -eq 1 ]
then
	(cd $LTPROOT/testscripts/; ./adp.sh -d 3 -n 100)
fi
## END => Test Series 14                               ##


## The next one i plan to run are the LTP AUTOFS tests
## START => Test Series 15                             ##
if [ $RUN_LTP_AUTOFS1_TESTS -eq 1 ]
then
	if [ $DISK_PARTITION1 ]
	then
		(cd $LTPROOT/testscripts/; ./autofs1.sh $DISK_PARTITION1)
	else
		echo "Disk Partition not set. Aborting running AUTOFS1"
	fi
fi
if [ $RUN_LTP_AUTOFS4_TESTS -eq 1 ]
then
	if [ $DISK_PARTITION1 ]
	then
		(cd $LTPROOT/testscripts/; ./autofs4.sh $DISK_PARTITION1)
	else
		echo "Disk Partition not set. Aborting running AUTOFS4"
	fi
fi
## END => Test Series 15                               ##


## The next one i plan to run are the LTP EXPORTFS tests
## START => Test Series 16                             ##
if [ $RUN_EXPORTFS_TESTS -eq 1 ]
then
	(cd $LTPROOT/testscripts/; ./exportfs.sh -h $NFS_SERVER1 -d $NFS_SERVER_DISK_PARTITION1 -t $NFS_SERVER_FS_TYPE1)
fi
## END => Test Series 16                               ##


## The next one i plan to run the FS tests on READ ONLY File Systems
## START => Test Series 17                             ##
if [ $RUN_RO_ONLY_FS_TESTS -eq 1 ]
then
	(cd $READ_ONLY_DIRECTORY1; $LTPROOT/testscripts/test_robind.sh)
fi
## END => Test Series 17                               ##


## The next one i plan to run the ISOFS tests
## START => Test Series 18                             ##
if [ $RUN_ISOFS_TESTS -eq 1 ]
then
	(cd $LTPROOT/testscripts/; ./isofs.sh)
fi
## END => Test Series 18                               ##


## The next one i plan to run the DMMAPPER tests
## START => Test Series 19                             ##
if [ $RUN_DMMAPPER_TESTS -eq 1 ]
then
	(cd $LTPROOT/testscripts/; ./ltpdmmapper.sh -a $DISK_PARTITION2 -b $DISK_PARTITION3)
fi
## END => Test Series 19                               ##


## The next one i plan to run the FSLVM tests
## START => Test Series 20                             ##
if [ $RUN_FSLVM_TESTS -eq 1 ]
then
	(cd $LTPROOT/testscripts/; ./ltpfslvm.sh -a $DISK_PARTITION4 -b $DISK_PARTITION5 -c $DISK_PARTITION6 -d $DISK_PARTITION7 -n $NFS_PARTITION1)
fi
## END => Test Series 20                               ##


## The next one i plan to run the FSNOLVM tests
## START => Test Series 21                             ##
if [ $RUN_FSNOLVM_TESTS -eq 1 ]
then
	(cd $LTPROOT/testscripts/; ./ltpfsnolvm.sh -a $DISK_PARTITION4 -b $DISK_PARTITION5 -c $DISK_PARTITION6 -d $DISK_PARTITION7 -n $NFS_PARTITION1)
fi
## END => Test Series 21                               ##

## The next one i plan to run the LTP SCSI DEBUG tests
## START => Test Series 22                             ##
if [ $RUN_LTP_SCSI_DEBUG_TEST -eq 1 ]
then
	(cd $LTPROOT/testscripts/; ./ltp-scsi_debug.sh)
fi
## END => Test Series 22                               ##

## The next one i plan to run the LTP SYSFS tests
## START => Test Series 23                             ##
if [ $RUN_LTP_SYSFS_TEST -eq 1 ]
then
	(cd $LTPROOT/testscripts/; ./sysfs.sh -k $KERNEL_MODULE1)
fi
## END => Test Series 23                               ##

## The next one i plan to run the LTP TIRPC tests
## START => Test Series 24                             ##
if [ $RUN_LTP_TIRPC_TEST -eq 1 ]
then
	"${RUNLTP}" -f rpctirpc
fi
## END => Test Series 24                               ##

## The next one i plan to run the LTP SE-Linux tests
## START => Test Series 25                             ##
if [ $RUN_SE_LINUX_TESTS -eq 1 ]
then
	make -C $LTPROOT/testcases/kernel/security/selinux-testsuite \
	 all install
	(cd $LTPROOT/testscripts/; ./test_selinux.sh)
fi
## END => Test Series 25                               ##

## The next one i plan to run the DMA_THREAD_DIOTEST7 tests
## START => Test Series 26                             ##
if [ $RUN_DMA_THREAD_DIOTEST7 -eq 1 ]
then
	"${RUNLTP}" -f test_dma_thread_diotest7
fi
## END => Test Series 26                               ##

## The next one i plan to run the Controller area network
## (CAN) protocol support tests
## START => Test Series 27                             ##
if [ $RUN_CONTROLLER_AREA_NETWORK_TESTS -eq 1 ]
then
	"${RUNLTP}" -f can
fi
## END => Test Series 27                               ##

## The next one i plan to run the SMACK SECURITY tests
## START => Test Series 28                             ##
if [ $RUN_SMACK_SECURITY_TESTS -eq 1 ]
then
	"${RUNLTP}" -f smack
fi
## END => Test Series 28                               ##

## The next one i plan to run the PERFORMANCE COUNTERS tests
## START => Test Series 29                             ##
if [ $RUN_PERFORMANCE_COUNTERS_TESTS -eq 1 ]
then
	"${RUNLTP}" -f perfcounters
fi
## END => Test Series 29                               ##


