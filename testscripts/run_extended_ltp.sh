#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2004						      ##
##									      ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	runfiv.sh
#
# Description:	Invoke runalltests.sh with standard options for FIV.
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	06 Jan 2004
#		07 Jan 2004 (rcp) Better usage message. Error msgs to stderr.
#		21 Jan 2004 (rcp) Added quick hack for 'only' 
#				TODO: Better command line parsing.
#               19 Nov 2004  Change to use runltp instead of runalltest
#			     Changed to include process number in the name
#			     of the logfiles

PROJECT=$1
ONLY=$2		# quick hack -- if $2 = 'only' we only run this scenario,
		#			no other tests.
LTPROOT=$PWD
TMP=/tmp
CMDFILE=$TMP/$PROJECT-tests
LOGFILE=$PROJECT-`date +%Y%m%d-`$$.log
LOGFILE_FULL=$LTPROOT/results/$LOGFILE
OUTFILE=$TMP/$PROJECT-`date +%Y%m%d-`$$.out

function usage()
{
	1>&2 cat <<-EOF
	USAGE: $0 <project name>
	.	There must be a scenario file named after the project:
	.		$LTPROOT/runtest/<project name>.scenario
	.	Optionally, there can be a list of tests to exclude:
	.		$LTPROOT/runtest/<project name>.exclude
	EOF
	exit 1
}

[ "$PROJECT" ] && [ -r $LTPROOT/runtest/$PROJECT.scenario ] || usage

# Don't allow existing logfile to be reused.
#if [ -s $LOGFILE_FULL ] ; then
#	1>&2 echo "Cowardly refusing to run since $LOGFILE_FULL already exists"
#	exit 1
#fi

# Don't allow existing output file to be reused.
#if [ -s $OUTFILE ] ; then
#	1>&2 echo "Cowardly refusing to run since $OUTFILE already exists"
#	exit 1
#fi

# Create our own command file based on the project's scenario file.
# (The following was removed:	"$LTPROOT/runtest/dio \")

if [ "$ONLY" = "only" ] ; then
	cp $LTPROOT/runtest/$PROJECT.scenario $CMDFILE
else
	cat \
		$LTPROOT/runtest/$PROJECT.scenario \
		$LTPROOT/runtest/syscalls \
		$LTPROOT/runtest/fs \
		$LTPROOT/runtest/fsx \
		$LTPROOT/runtest/dio \
		$LTPROOT/runtest/mm \
		$LTPROOT/runtest/ipc \
		$LTPROOT/runtest/sched \
		$LTPROOT/runtest/math \
		$LTPROOT/runtest/pty \
		> $CMDFILE
fi

# If there is an exclude list, use it to remove tests from the command file.
if [ -s $LTPROOT/runtest/$PROJECT.exclude ] ; then
	mv -f $CMDFILE $TMP/work
	while read line ; do
		if [ "$line" ] ; then
			skip="N";
			set $line	# $1 is tcname in scenario file
					# $2 is script name in scenario file
			while read tcname junk ; do
				[ "${tcname:0:1}" = "#" ] && continue
				[ "$tcname" = "" ] && continue
				[ "$1" = "$tcname" ] && skip="Y" && break
				[ "$2" = "$tcname" ] && skip="Y" && break
			done < $LTPROOT/runtest/$PROJECT.exclude
			[ "$skip" = "N" ] && echo "$line" >> $CMDFILE
		else
			echo "" >> $CMDFILE # preserve blank lines
		fi
	done < $TMP/work
	rm -f $TMP/work
fi

# Invoke with our command file; put results in known places.
$LTPROOT/runltp -p -f $CMDFILE -l $LOGFILE -o $OUTFILE

