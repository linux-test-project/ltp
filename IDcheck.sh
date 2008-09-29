#!/bin/sh
#
#    Copyright (c) International Business Machines  Corp., 2001
#
#    This program is free software;  you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY;  without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#    the GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program;  if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
#   FILE        : IDcheck.sh
#   DESCRIPTION : checks for req'd users/groups and will create them if requested.
#   HISTORY     : see the cvs log
#

# Prompt user if ids/groups should be created
clear
echo "Checking for required user/group ids"
echo ""

# Check ids and create if needed.
NO_NOBODY_ID=1
NO_BIN_ID=1
NO_DAEMON_ID=1
NO_NOBODY_GRP=1
NO_BIN_GRP=1
NO_DAEMON_GRP=1
NO_USERS_GRP=1
NO_SYS_GRP=1
I_AM_ROOT=0

#
# id(1) for entry.
#
ife() {
	id "$@" > /dev/null
}

#
# grep(1) for entry.
#
gfe() {
	grep -q "$@"
}

prompt_for_create() {
	if [ -n "$CREATE_ENTRIES" ] ; then

		if [ $I_AM_ROOT -eq 0 ] ; then
			echo "Not root; can't create user / group entries on local machine".
			CREATE_ENTRIES=0
		fi
		echo "CREATE_ENTRIES variable set to $CREATE_ENTRIES ..."
		echo

	else

		if [ $NO_NOBODY_ID -ne 0 -o $NO_BIN_ID -ne 0 -o $NO_DAEMON_ID -ne 0 -o $NO_NOBODY_GRP -ne 0 -o $NO_BIN_GRP -ne 0 -o $NO_DAEMON_GRP -ne 0 -o $NO_USERS_GRP -ne 0 -o $NO_SYS_GRP -ne 0 -a $I_AM_ROOT -ne 0 ] ; then
			echo -n "If any required user ids and/or groups are missing, would you like these created? [y/N]"
			read ans
			case "$ans" in
				Y*|y*) CREATE_ENTRIES=1 ;;
				*)     CREATE_ENTRIES=0 ;;
			esac
		else
			CREATE_ENTRIES=0
		fi

	fi
}

if [ -z ${EUID} ] ; then
	EUID=$(id -u)
fi

if [ -e /etc/passwd -a ! -r /etc/passwd ] ; then
	echo "/etc/passwd not readable by uid $EUID"
	exit 1
elif [ -e /etc/group -a ! -r /etc/group ] ; then
	echo "/etc/group not readable by uid $EUID"
	exit 1
fi

ife bin; NO_BIN_ID=$?
ife daemon; NO_DAEMON_ID=$?
ife nobody; NO_NOBODY_ID=$?

gfe '^bin:' /etc/group; NO_BIN_GRP=$?
gfe '^daemon:' /etc/group; NO_DAEMON_GRP=$?
gfe '^nobody:' /etc/group; NO_NOBODY_GRP=$?
gfe '^sys:' /etc/group; NO_SYS_GRP=$?
gfe '^users:' /etc/group; NO_USERS_GRP=$?

if [ $EUID -eq 0 ] ; then
	I_AM_ROOT=1
fi

prompt_for_create

debug_vals() {

echo "Missing the following group / user entries:"
echo "nobody:        $NO_NOBODY_ID"
echo "bin:           $NO_BIN_ID"
echo "daemon:        $NO_DAEMON_ID"
echo "nobody grp:    $NO_NOBODY_GRP"
echo "bin grp:       $NO_BIN_GRP"
echo "daemon grp:    $NO_DAEMON_GRP"
echo "sys grp:       $NO_SYS_GRP"
echo "users grp:     $NO_USERS_GRP"
echo ""
echo "i am root:     $I_AM_ROOT"
echo ""

}

#debug_vals

if [ $CREATE_ENTRIES -ne 0 ] ; then
	if ! touch /etc/group ; then
		echo "Couldn't touch /etc/group"
		exit 1
	fi
fi

make_user_group() {
	local name=$1 id=$2 no_id=$3 no_grp=$4

	if [ $no_id -eq 0 -a $no_grp -eq 0 ] ; then
		echo "'$name' user id and group found."
	elif [ $CREATE_ENTRIES -ne 0 ] ; then
		echo "Creating entries for $name"

		# Avoid chicken and egg issue with id(1) call
		# made above and below.
		if ! gfe "^${name}:" /etc/passwd && [ $no_id -ne 0 ] ; then
			echo "${name}:x:${id}:${id}:${name}::" >> /etc/passwd
		fi
		if [ $no_grp -ne 0 ] ; then
			echo "${name}:x:$(id -u ${name}):" >> /etc/group
		fi
	fi
}
make_user_group nobody 99 $NO_NOBODY_ID $NO_NOBODY_GRP
make_user_group bin 1 $NO_BIN_ID $NO_BIN_GRP
make_user_group daemon 2 $NO_DAEMON_ID $NO_DAEMON_GRP

if [ $NO_USERS_GRP -eq 0 ] ; then
	echo "Users group found."
elif [ $CREATE_ENTRIES -ne 0 ] ; then
	echo 'users:x:100:' >> /etc/group
fi

if [ $NO_SYS_GRP -eq 0 ] ; then
	echo "Sys group found."
elif [ $CREATE_ENTRIES -ne 0 ] ; then
	echo 'sys:x:3:' >> /etc/group
fi

if ife nobody    && ife bin    && ife daemon &&
   ife -g nobody && ife -g bin && ife -g daemon &&
   gfe '^users:' /etc/group && gfe '^sys:' /etc/group && 
   gfe '^nobody:' /etc/group
then
	echo ""
	echo "Required users/groups exist."
	exit 0
fi

echo ""
echo "*****************************************"
echo "* Required users/groups do NOT exist!!! *"
echo "*                                       *"
echo "* Some kernel/syscall tests will FAIL!  *"
echo "*****************************************"
exit 1
