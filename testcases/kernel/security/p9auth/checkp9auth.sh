#!/bin/sh
################################################################################
##										      ##
## Copyright (c) International Business Machines  Corp., 2009		   ##
##										      ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by	##
## the Free Software Foundation; either version 2 of the License, or	   ##
## (at your option) any later version.					     ##
##										      ##
## This program is distributed in the hope that it will be useful, but	 ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.								  ##
##										      ##
## You should have received a copy of the GNU General Public License	   ##
## along with this program;  if not, write to the Free Software		 ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##										      ##
################################################################################

yesno=0
if [ "$1" = "yesno" ]; then
	yesno=1
fi

# check for openssl
rm -f /tmp/ab
echo ab > /tmp/ab
openssl sha1 -hmac "ab" /tmp/ab > /dev/null
ret=$?
if [ $ret -ne 0 ]; then
	if [ $yesno -eq 1 ]; then echo
		 "no"
	else
		 echo "openssl not installed, skipping p9auth tests."
	fi
	exit 1
fi

majfile=/sys/module/p9auth/parameters/cap_major
minfile=/sys/module/p9auth/parameters/cap_minor
if [ ! -f "$majfile" ]; then
	if [ $yesno -eq 1 ]; then echo
		 "no"
	else
		 echo "p9auth not detected.  Skipping p9auth tests."
	fi
	exit 1
fi

if [ ! -c "/dev/caphash" ]; then
	rm -f /dev/caphash
	maj=`cat $majfile`
	mknod /dev/caphash c $maj 0
fi

if [ ! -c "/dev/capuse" ]; then
	rm -f /dev/capuse
	min=`cat $minfile`
	mknod /dev/capuse c $maj 1
fi
chmod ugo+w /dev/capuse

if [ $yesno -eq 1 ]; then
	echo "yes"
else
	echo "p9auth ready for testing"
fi
exit 0
