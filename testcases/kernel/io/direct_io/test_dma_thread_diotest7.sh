#! /bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2009                 ##
## Copyright (c) Li Zefan <lizf@cn.fujitsu.com>, 2009                         ##
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

export TCID=dma_thread_diotest7
export TST_TOTAL=3
export TST_COUNT=3

tst_resm TINFO "Generating Test Files"
./dma_thread_diotest7

# test different alignments: 512, 1024, ..., (4096-512)
for i in $(seq 512 512 4095)
do
        tst_resm TINFO "i=$i"
        ./dma_thread_diotest7 -a="$i"
        if [ $? -ne 0 ]; then
             tst_res TFAIL "Test Failed"
             exit 1
        else
             tst_resm TPASS "Test Passed"
        fi
done
exit 0;
