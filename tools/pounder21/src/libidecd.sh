#!/bin/bash

# Library to find CD devices.
#
# Copyright (C) 2003-2006 IBM
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.


# Create a list of potential devices.  Note that this may pick up some non-block
# devices; it is assumed that they will be filtered out by find_discs_with_media.
function find_disc_devices() {
	NUM_DEVICES=`/bin/ls $(egrep '(cdr|dvd)' /etc/fstab | awk -F " " '{print $1}') /dev/cdr* /dev/dvd* /dev/cdrom/* /dev/sr* 2> /dev/null | sort | uniq | wc -l`
	if [ $NUM_DEVICES -lt 1 ]; then
		# No CDs at all?
		echo NONE
	fi
	/bin/ls $(egrep '(cdr|dvd)' /etc/fstab | awk -F " " '{print $1}') /dev/cdr* /dev/dvd* /dev/cdrom/* /dev/sr* 2> /dev/null | sort | uniq
}

# Try to find a disc with media in it.  Hopefully, $DEFAULT_MOUNT already exists.
function find_discs_with_media() {
	# If the caller doesn't specify a DEFAULT_MOUNT point, specify one.
	if [ -z "$DEFAULT_MOUNT" ]; then
		DEFAULT_MOUNT=/mnt
	fi
	POTENTIAL_DEVICES=`find_disc_devices`
	# Grab a list of all CD/DVD devices that we can find.
	for i in `echo "$POTENTIAL_DEVICES"`
	do
		# Did we get nothing at all?
		if [ "$i" == "NONE" ]; then
			echo NONE 0
			return
		fi

		# Is this a link pointing to a device that's in the
		# list of potential discs AND isn't in fstab?
		# We want to avoid considering /dev entries that are symlinked
		# elsewhere ... but we also assume that anything in fstab was
		# put there for a reason and ought to be considered anyway.
		if [ -L "$i" ]; then
			IN_LIST=`echo "$POTENTIAL_DEVICES" | grep "$(readlink $i)" -c`
			if [ $IN_LIST -gt 0 ]; then
				IN_FSTAB=`grep "^$i[ 	]" /etc/fstab -c`
				if [ $IN_FSTAB -eq 0 ]; then
					continue;
				fi
			fi
		fi

		# Block device?
		if [ -b "$i" ]; then
			IN_FSTAB=`grep -c "^$i[ 	]" /etc/fstab`
			FSTAB_TYPE=`grep "^$i[ 	]" /etc/fstab | awk -F " " '{print $3}'`
			if [ $IN_FSTAB -gt 0 -a "$FSTAB_TYPE" != "subfs" ]; then
				# This device is listed in fstab and is NOT of
				# type "subfs" (SLES9 weirdness); try to mount it.
				mount "$i" > /dev/null 2> /dev/null
				RESULT=$?

				if [ $RESULT -eq 0 ]; then
					# Mounted ok!
					umount "$i"
					echo "$i" 1
					continue
				fi
			fi

			# Not in fstab, or the mount failed.
			mount "$i" "$DEFAULT_MOUNT" -t auto > /dev/null 2> /dev/null
			RESULT=$?

			if [ $RESULT -eq 0 ]; then
				# Mounted ok once we gave it options.
				umount "$i"
				echo "$i" 0
				continue
			fi
		fi
	done
}
