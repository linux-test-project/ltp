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
#   PURPOSE: Creates data_dir for use in network file transfer tests.
#
#   AUTHOR: Robbie Williamson (robbiew@us.ibm.com)
#
############################################################################

my $data_dir = 'dat';
my $small_file = 'smallsize.fil';
my $medium_file = 'medsize.fil';
my $large_file = 'largesize.fil';
my $jumbo_file = 'maxsize.fil';
my $small_size = 1600020;
my $medium_size = 80020;
my $large_size = 4020;
my $jumbo_size = 220;

unless ( -d $data_dir ) {
	mkdir($data_dir,0777)
}
chdir($data_dir);
unless (-f $small_file) {
        open(DATAFILE, ">$small_file") or die "$0: could not create $small_file: $!\n";
        print DATAFILE 'A' x $small_size;
        close(DATAFILE);
        chmod 0666, $small_file;
}
unless (-f $medium_file) {
        open(DATAFILE, ">$medium_file") or die "$0: could not create $medium_file: $!\n";
        print DATAFILE 'A' x $medium_size;
        close(DATAFILE);
        chmod 0666, $medium_file;
}
unless (-f $large_file) {
        open(DATAFILE, ">$large_file") or die "$0: could not create $large_file: $!\n";
        print DATAFILE 'A' x $large_size;
        close(DATAFILE);
        chmod 0666, $large_file;
}
unless (-f $jumbo_file) {
        open(DATAFILE, ">$jumbo_file") or die "$0: could not create $jumbo_file: $!\n";
        print DATAFILE 'A' x $jumbo_size;
        close(DATAFILE);
        chmod 0666, $jumbo_file;
}
