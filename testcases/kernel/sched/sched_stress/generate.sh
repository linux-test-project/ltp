#!/usr/bin/perl
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
#   PURPOSE: Creates dat for use in network file transfer tests.
#
#   AUTHOR: Randy Hron (rwhron@earthlink.net)
#
############################################################################

my $file = 'sched_datafile';
my $size = 1200010;

unless (-f $file) {
	open(DATAFILE, ">$file") or die "$0: could not create $file: $!\n";
	print DATAFILE 'A' x $size;
	close(DATAFILE);
	chmod 0666, $file;
}
