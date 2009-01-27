#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2007                 ##
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

exit_code=0
ret=0
echo "****************** sysvipc tests ******************"
for type in none clone unshare; do
      echo "sysvipc: SharedMemory $type"
      shmnstest $type
      ret=$?
      if [ $ret -ne 0 ]; then
              exit_code=$ret
      fi
      shmem_2nstest $type
      ret=$?
      if [ $ret -ne 0 ]; then
              exit_code=$ret
      fi
done
echo
for type in none clone unshare; do
      echo "sysvipc: MesgQ $type"
      mesgq_nstest $type
	  ret=$?
      if [ $exit_code -ne 0 ]; then
              exit_code=$ret
      fi
done
echo
for type in none clone unshare; do
      echo "sysvipc: Semaphore $type"
      sem_nstest $type
	  ret=$?
      if [ $exit_code -ne 0 ]; then
              exit_code=$ret
      fi
      semtest_2ns $type
      ret=$?
      if [ $ret -ne 0 ]; then
              exit_code=$ret
      fi
done
echo
exit $exit_code
