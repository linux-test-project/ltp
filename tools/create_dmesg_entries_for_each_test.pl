#!/usr/bin/perl
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2010                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and/or modify      ##
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
#                                                                             ##
# File :        create_dmesg_entries_for_each_test.pl			      ##
#                                                                             ##
# Usage:        create_dmesg_entries_for_each_test.pl\			      ##
#		<LTP_COMMAND_FILE> <DMESG_DIRECTORY>			      ##
#                                                                             ##
# Description:  This is a simple perl script which will take ltp command file ##
#		as input and then create a final command file which will have ##
#		the following entries for each test tag:		      ##
#		<tag_name__with_dmesg_entry> <test_binary_name>;\	      ##
#		<save_dmesg_logs>			      		      ##
#                                                                             ##
# Author:       Subrata Modak <subrata@linux.vnet.ibm.com>                    ##
#                                                                             ##
# History:      May 09 2010 - Created - Subrata Modak.                        ##
################################################################################

my $command_file	= shift (@ARGV) || syntax();
my $dmesg_dir		= shift (@ARGV) || syntax();

sub syntax() {
	print "syntax: create_dmesg_entries_for_each_test.pl <LTP_COMMAND_FILE> <DMESG_DIRECTORY>\n";
	exit (1);
}


open (FILE, $command_file) || die "Cannot open file: $command_file\n";
while ($line = <FILE>) {
	if ($line =~ /^#/) {
		print "$line";
		next;
	}
	if ($line =~ /^\n$/) {
		next;
	}
	chomp $line;
	@tag_and_actual_command = split(/\ /, $line);

	my $token_counter = 0;
	my $tag_name = "";
	foreach my $token (@tag_and_actual_command) {
		if ($token_counter == 0 ) {
			print $token . "__with_dmesg_entry" . " ";
			print "dmesg -c 1>/dev/null 2>/dev/null;" . " ";
			$token_counter++;
			$tag_name = $token;
			next;
		}
		print " " . $token . " ";
	}
	print "; dmesg > $dmesg_dir/$tag_name.dmesg.log \n";
}
close (FILE);

