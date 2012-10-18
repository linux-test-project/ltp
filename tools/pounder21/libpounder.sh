# Common shell functions and variables that all pounder scripts can use.

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

# Start by setting environment variables...
export DATE=`date +%Y%h%d-%H%M%S`

export DEFAULT_SCHEDPACK=default
export TESTS=`/bin/ls test_scripts/`
export BUILDS=`/bin/ls build_scripts/`
export POUNDER_HOME=`pwd`
export POUNDER_PIDFILE="$POUNDER_HOME/pounder.pid"
export POUNDER_LOGLOCAL="$POUNDER_HOME/log"
export POUNDER_LOGDIR="$POUNDER_LOGLOCAL/$DATE/"
export POUNDER_TMPDIR="$POUNDER_HOME/tmp/"
export POUNDER_OPTDIR="$POUNDER_HOME/opt/"
export POUNDER_SRCDIR="$POUNDER_HOME/src/"
export POUNDER_VERSION=`head -1 "$POUNDER_HOME/README" | awk -F " " '{print $3}' | sed -e 's/\.//g'`
export NR_CPUS=`getconf _NPROCESSORS_ONLN`
export NFS_LOGLOCAL="`echo "$HOSTNAME" | sed -e 's/\..*//g'`/`uname -r`-`uname -m`"

if [ -e "$POUNDER_HOME/config" ]; then
	source "$POUNDER_HOME/config"
fi

function get_from_sourceforge {
	PROGNAME=$1
	TARNAME=$2

	# Correct arguments?
	if [ -z "$1" -o -z "$2" ]; then
		echo "get_from_sourceforge: Called with empty arguments."
		exit
	fi

	# File already exists?
	if [ -f "$TARNAME" ]; then
		echo "get_from_sourceforge: Target file already exists."
		exit
	fi

	# Else try download...
	for SERVER in voxel.dl.sourceforge.net easynews.dl.sourceforge.net umn.dl.sourceforge.net; do
		wget -t 1 --timeout=15 "http://$SERVER/sourceforge/$PROGNAME/$TARNAME"

		if [ -f "$TARNAME" ]; then
			break
		fi
	done
}
