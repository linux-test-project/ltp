#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2008                 ##
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

check_simple_capset
ret=$?
if [ $ret -ne 0 ]; then
	echo Posix capabilities not compiled into the kernel.  Please
	echo modprobe capability or recompile your kernel with
	echo CONFIG_SECURITY_CAPABILITIES=y.
	exit 1
fi

touch testme
setcap cap_sys_admin=ip testme
ret=$?
rm -f testme
if [ $ret -ne 0 ]; then
	echo File capabilities not compiled into kernel.  Please
	echo make sure your kernel is compiled with
	echo CONFIG_SECURITY_FILE_CAPABILITIES=y.
	exit 1
fi

exit 0
