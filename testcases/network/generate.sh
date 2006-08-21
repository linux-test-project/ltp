#!/bin/sh
#
#   Copyright (c) International Business Machines  Corp., 2001
#
#   This program is free software;  you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY;  without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#   the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this pronram;  if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
#
#   FILE: generate.sh
#
#   PURPOSE: Creates data_dir for use in network file transfer tests.
#
#   AUTHOR: Robbie Williamson (robbiew@us.ibm.com)
#
############################################################################

data_dir=datafiles
small_file='ascii.sm'
medium_file='ascii.med'
large_file='ascii.lg'
jumbo_file='ascii.jmb'
jumbo_size=1600020
large_size=80020
medium_size=4020
small_size=220

if [ ! -d $data_dir ] ; then
	mkdir $data_dir
	chmod 777 $data_dir
fi

for m in .. ../.. ../../.. ../../../.. ; do
	makeit=$m/tools/make-file.sh
	if [ -e $makeit ] ; then
		break
	fi
done

$makeit $data_dir/$small_file $small_size
$makeit $data_dir/$medium_file $medium_size
$makeit $data_dir/$large_file $large_size
$makeit $data_dir/$jumbo_file $jumbo_size

if [ ! -e $data_dir/bin.sm ] ; then
	cnt=6
	while ((cnt--)) ; do
		gzip -1 -c datafiles/ascii.sm >> $data_dir/bin.sm
	done
fi

genfile() {
	local input=$data_dir/$1 output=$data_dir/$2
	local cnt=20

	if [ -e $output ] ; then
		return 0
	fi

	while ((cnt--)) ; do
		cat $input >> $output
	done
}

genfile bin.sm bin.med
genfile bin.med bin.lg
genfile bin.lg bin.jmb

chmod 666 $data_dir/bin.*
