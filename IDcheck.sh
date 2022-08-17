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
#    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
#   FILE        : IDcheck.sh
#   DESCRIPTION : checks for req'd users/groups and will create them if requested.
#   HISTORY     : see the cvs log
#

# Prompt user if ids/groups should be created
echo "Checking for required user/group ids"
echo ""

# Check ids and create if needed.
NO_ROOT_ID=1
NO_NOBODY_ID=1
NO_BIN_ID=1
NO_DAEMON_ID=1
NO_ROOT_GRP=1
NO_NOBODY_GRP=1
NO_BIN_GRP=1
NO_DAEMON_GRP=1
NO_USERS_GRP=1
NO_SYS_GRP=1

group="$DESTDIR/etc/group"
passwd="$DESTDIR/etc/passwd"

# find entry.
fe() {
    ID=$1
    FILE=$2
    [ -e "$FILE" ] || return $?
    grep -q "^$ID:" "$FILE"
}

prompt_for_create() {
	if [ -z "$CREATE_ENTRIES" ] ; then

		if [ $NO_ROOT_ID -ne 0 -o $NO_NOBODY_ID -ne 0 -o $NO_BIN_ID -ne 0 -o $NO_DAEMON_ID -ne 0 -o $NO_ROOT_GRP -ne 0 -o $NO_NOBODY_GRP -ne 0 -o $NO_BIN_GRP -ne 0 -o $NO_DAEMON_GRP -ne 0 -o $NO_USERS_GRP -ne 0 -o $NO_SYS_GRP -ne 0 ] ; then
			echo -n "If any required user ids and/or groups are missing, would you like these created? [y/N]"
			read ans
			case "$ans" in
			[Yy]*) CREATE_ENTRIES=1 ;;
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

for i in "$passwd" "$group"; do
    if [ -e "$i" -a ! -r "$i" ] ; then
	echo "$i not readable by uid $EUID"
	exit 1
    fi
done

fe root "$passwd"; NO_ROOT_ID=$?
fe bin "$passwd"; NO_BIN_ID=$?
fe daemon "$passwd"; NO_DAEMON_ID=$?
fe nobody "$passwd"; NO_NOBODY_ID=$?

fe root "$group"; NO_ROOT_GRP=$?
fe bin "$group"; NO_BIN_GRP=$?
fe daemon "$group"; NO_DAEMON_GRP=$?
fe nobody "$group" || fe nogroup "$group"; NO_NOBODY_GRP=$?
fe sys "$group"; NO_SYS_GRP=$?
fe users "$group"; NO_USERS_GRP=$?

prompt_for_create

debug_vals() {

echo "Missing the following group / user entries:"
echo "Group file:		$group"
echo "Password file:		$passwd"
echo "root			$NO_ROOT_ID"
echo "nobody:			$NO_NOBODY_ID"
echo "bin:			$NO_BIN_ID"
echo "daemon:			$NO_DAEMON_ID"
echo "root grp:			$NO_ROOT_GRP"
echo "nobody[/nogroup] grp:	$NO_NOBODY_GRP"
echo "bin grp:			$NO_BIN_GRP"
echo "daemon grp:		$NO_DAEMON_GRP"
echo "sys grp:			$NO_SYS_GRP"
echo "users grp:		$NO_USERS_GRP"
echo ""

}

#debug_vals

if [ $CREATE_ENTRIES -ne 0 ] ; then
    if ! touch "$group" "$passwd" 2>/dev/null; then
        echo "Failed to touch $group or $passwd"
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
		if ! fe "$name" "$passwd" && [ $no_id -ne 0 ] ; then
			echo "${name}:x:${id}:${id}:${name}::" >> "$passwd"
		fi
		if [ $no_grp -ne 0 ] ; then
			echo "${name}:x:$(id -u ${name}):" >> "$group"
		fi
	fi
}
make_user_group root 0 $NO_ROOT_ID $NO_ROOT_GRP
make_user_group nobody 65534 $NO_NOBODY_ID $NO_NOBODY_GRP
make_user_group bin 1 $NO_BIN_ID $NO_BIN_GRP
make_user_group daemon 2 $NO_DAEMON_ID $NO_DAEMON_GRP

if [ $NO_USERS_GRP -eq 0 ] ; then
	echo "Users group found."
elif [ $CREATE_ENTRIES -ne 0 ] ; then
	echo 'users:x:100:' >> "$group"
fi

if [ $NO_SYS_GRP -eq 0 ] ; then
	echo "Sys group found."
elif [ $CREATE_ENTRIES -ne 0 ] ; then
	echo 'sys:x:3:' >> "$group"
fi

MISSING_ENTRY=0

# For entries that exist in both $group and $passwd.
for i in root bin daemon; do
    for file in "$group" "$passwd"; do
        if ! fe "$i" "$file"; then
            MISSING_ENTRY=1
            break
        fi
    done
    if [ $MISSING_ENTRY -ne 0 ]; then
        break
    fi
done

# nobody is a standard group on all distros, apart from debian based ones;
# let's account for the fact that they use the nogroup group instead.
if ! fe "nobody" "$passwd" || ! (fe "nogroup" "$group" || fe "nobody" "$group")
then
    MISSING_ENTRY=1
fi

# For entries that only exist in $group.
for i in users sys; do
    if ! fe "$i" "$group" ; then
        MISSING_ENTRY=1
    fi
done

if [ $MISSING_ENTRY -eq 0 ] ; then
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
