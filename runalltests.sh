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

echo -e  "*******************************************************************"
echo -e  "*******************************************************************"
echo -e  "**                                                               **"
echo -e "** This script is being re-written to cover all aspects of    **"
echo -e "** testing LTP, which includes running all those tests which  **"
echo -e "** are not run by default with ./runltp script. Special setup **"
echo -e "** in system environment will be done to run all those tests  **"
echo -e "** like the File System tests, SELinuxtest, etc               **"
echo -e  "**                                                               **"
echo -e  "*******************************************************************"
echo -e  "*******************************************************************"

export LTPROOT=${PWD}
## Set this to 1 if mm-1.4.2.tar.gz is already installed in your system
## from ftp://ftp.ossp.org/pkg/lib/mm/mm-1.4.2.tar.gz
export LIBMM_INSTALLED=0
## This is required if already not set to /usr/local/lib/ as
## mm-1.4.2.tar.gz gets installed at /usr/local/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/

## Set this to 1 if libaio-0.3.92.tar.gz is already installed in your system
## from http://www.kernel.org/pub/linux/kernel/people/bcrl/aio/libaio-0.3.92.tar.gz
export LIBAIO_INSTALLED=0

export LTP_VERSION=`./runltp -e`
export TEST_START_TIME=`date +"%Y_%b_%d-%Hh_%Mm_%Ss"`
export HARDWARE_TYPE=$(uname -i)
export HOSTNAME=$(uname -n)
export KERNEL_VERSION=$(uname -r)
export HTML_OUTPUT_FILE_NAME=$LTP_VERSION-$HOSTNAME-$KERNEL_VERSION-$HARDWARE_TYPE-$TEST_START_TIME.html


## The First one i plan to run is the default LTP run ##
## START => Test Series 1                             ##
echo -e "Running Default LTP..."
./runltp -g $HTML_OUTPUT_FILE_NAME
echo -e "Completed running Default LTP\n\n"
## END => Test Series 1                               ##

## The next one i plan to run is ballista             ##
## START => Test Series 2                             ##
echo -e "Running Ballista..."
export TEST_START_TIME=`date +"%Y_%b_%d-%Hh_%Mm_%Ss"`
./runltp -f ballista -o $LTP_VERSION-BALLISTA_RUN_ON-$HOSTNAME-$KERNEL_VERSION-$HARDWARE_TYPE-$TEST_START_TIME.out
echo -e "Completed running Ballista\n\n"
## END => Test Series 2                               ##

## The next one i plan to run is open_posix_testsuite ##
## START => Test Series 3                             ##
echo -e "Running Open Posix Tests..."
(cd testcases/open_posix_testsuite; make)
echo -e "Completed running Open Posix Tests\n\n"
## END => Test Series 3                               ##


## The next one i plan to run is                      ##
## ltp/testcases/kernel/mem/libmm/mm_core_apis        ##
## START => Test Series 4                             ##
echo -e "Initializing ltp/testcases/kernel/mem/libmm/mm_core_apis ..."
# Check to see if User is Root
if [ $EUID -ne 0 ]
then
    echo You need to be root to Install libmm and run mem/libmm/mm_core_apis
    echo Aborting ltp/testcases/kernel/mem/libmm/mm_core_apis
else

    if [ $LIBMM_INSTALLED -ne 1 ]
    then
        echo Installing libmm-1.4.2 .............
        (cd /tmp; \
         wget -c ftp://ftp.ossp.org/pkg/lib/mm/mm-1.4.2.tar.gz; \
         tar -xzf mm-1.4.2.tar.gz; \
         cd mm-1.4.2; \
         ./configure && make && make install )
        rm -rf /tmp/mm-1.4.2*
        echo libmm-1.4.2 Installed .............
    else
        echo libmm-1.4.2 already installed in your system
    fi
        echo -e "Running ltp/testcases/kernel/mem/libmm/mm_core_apis ..."
    (cd testcases/kernel/mem/libmm; \
     make; \
     make install; \
     $LTPROOT/testcases/bin/mm_core_apis; )
fi
echo -e "Completed running ltp/testcases/kernel/mem/libmm/mm_core_apis...\n\n"
## END => Test Series 4                               ##


## The next one i plan to run is                      ##
## ltp/testcases/kernel/io/aio                        ## 
## START => Test Series 5                             ##
echo -e "Initializing ltp/testcases/kernel/io/aio ..."
# Check to see if User is Root
if [ $EUID -ne 0 ]
then
    echo You need to be root to Install libaio-0.3.92 and run ltp/testcases/kernel/io/aio
    echo Aborting ltp/testcases/kernel/io/aio
else
    if [ $LIBAIO_INSTALLED -ne 1 ]
    then
        echo Installing libaio-0.3.92.............
        (cd /tmp; \
         wget -c http://www.kernel.org/pub/linux/kernel/people/bcrl/aio/libaio-0.3.92.tar.gz; \
         tar -xzf libaio-0.3.92.tar.gz; \
         cd libaio-0.3.92; \
         make > /dev/null && make install > /dev/null)
         rm -rf /tmp/libaio-0.3.92*
         echo libaio-0.3.92 Installed .............
    else
        echo libaio-0.3.92 already installed in your system
    fi
        echo -e "Building & Running ltp/testcases/kernel/io/aio..."
        (cd testcases/kernel/io/aio; \
         make > /dev/null; \
         ./aio01/aio01; \
         ./aio02/runfstests.sh -a aio02/cases/aio_tio; \
         make clean 1>&2 > /dev/null )
    echo -e "Completed running ltp/testcases/kernel/io/aio...\n\n"
fi
## END => Test Series 5                               ##

