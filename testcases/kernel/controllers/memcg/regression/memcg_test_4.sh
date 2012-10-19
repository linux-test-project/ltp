#! /bin/sh

################################################################################
##                                                                            ##
## Copyright (c) 2009 FUJITSU LIMITED                                         ##
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
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
## Author: Li Zefan <lizf@cn.fujitsu.com>                                     ##
##                                                                            ##
################################################################################

# attach current task to memcg/0/
mkdir memcg/0
echo $$ > memcg/0/tasks

./memcg_test_4 &
pid=$!
sleep 1

# let $pid allocate 100M memory
/bin/kill -SIGUSR1 $pid
sleep 1

# shrink memory, and then 80M will be swapped
echo 40M > memcg/0/memory.limit_in_bytes

# turn off swap, and swapoff will be killed
swapoff -a
sleep 1
echo $pid > memcg/tasks 2> /dev/null
echo $$ > memcg/tasks 2> /dev/null

# now remove the cgroup
rmdir memcg/0

