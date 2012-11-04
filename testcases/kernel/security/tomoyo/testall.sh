#! /bin/sh
################################################################################
##                                                                            ##
## Copyright (c) Tetsuo Handa <penguin-kernel@I-love.SAKURA.ne.jp>, 2009      ##
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
################################################################################

cd ${0%/*}
export PATH=$PWD:${PATH}

echo "Testing all. (All results are reported)"
newns tomoyo_accept_test
newns tomoyo_filesystem_test
newns tomoyo_file_test
newns tomoyo_rewrite_test
newns tomoyo_new_file_test
newns tomoyo_new_test
echo
echo
echo
echo "Testing all. (Only ERRORS are reported)"
newns tomoyo_accept_test | grep -vF Done
newns tomoyo_filesystem_test | grep -vF OK | grep -F '('
newns tomoyo_file_test | grep -vF OK | grep -F '('
newns tomoyo_rewrite_test | grep -vF OK | grep -F '('
newns tomoyo_new_test | grep -vF OK
echo
echo
echo
echo "Testing policy I/O.  (Only ERRORS are reported)"
newns tomoyo_policy_io_test | grep -vF OK
newns tomoyo_new_file_test | grep -vF OK
