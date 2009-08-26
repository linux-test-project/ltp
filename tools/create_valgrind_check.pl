#!/usr/bin/perl
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2009                 ##
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
# File :        create_valgrind_check					      ##
#                                                                             ##
# Usage:        create_valgrind_check\					      ##
#		<LTP_COMMAND_FILE> <VALGRIND_CHECK_TYPE>		      ##
#                                                                             ##
# Description:  This is a simple perl script which will take ltp command file ##
#		as input and then create a final command file while will have ##
#		the following entries for each test tag:		      ##
#		1) <tag_name> <test_binary_name>			      ##
#		2) <tag_name_valgrind_check_type> <valgrind test_binary_name> ##
#                                                                             ##
# Author:       Subrata Modak <subrata@linux.vnet.ibm.com>                    ##
#                                                                             ##
# History:      Aug 23 2009 - Created - Subrata Modak.                        ##
################################################################################

my $command_file	= shift (@ARGV) || syntax();
my $valgrind_check_type	= shift (@ARGV) || syntax();

sub syntax() {
	print "syntax: create_valgrind_check\
	<LTP_COMMAND_FILE> <VALGRIND_CHECK_TYPE>\n";
	exit (1);
}

sub print_memory_leak_check {
	$sub_line = shift;
	@sub_tag_and_actual_command = split(/\ /, $sub_line);
	my $sub_token_counter = 0;
	foreach my $sub_token (@sub_tag_and_actual_command) {
		if ($sub_token_counter == 0 ) {#print the tagname now
			print $sub_token . "_valgrind_memory_leak_check " .
				" valgrind -q --leak-check=full --trace-children=yes ";
			$sub_token_counter++;
			next;
		}
		print " " . $sub_token . " ";
	}
	print "\n";
}

sub print_thread_concurrency_check {
	$sub_line = shift;
	@sub_tag_and_actual_command = split(/\ /, $sub_line);
	my $sub_token_counter = 0;
	foreach my $sub_token (@sub_tag_and_actual_command) {
		if ($sub_token_counter == 0 ) {#print the tagname now
			print $sub_token . "_valgrind_thread_concurrency_check " .
				" valgrind -q --tool=helgrind --trace-children=yes ";
			$sub_token_counter++;
			next;
		}
		print " " . $sub_token . " ";
	}
	print "\n";
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
	print "$line\n"; #Print one instance for normal execution

	if ($valgrind_check_type == 3) {
		#Print for both Memory Leak and Thread Concurrency Checks
		print_memory_leak_check($line);
		print_thread_concurrency_check($line);
	}
	if ($valgrind_check_type == 2) {
		#Print only for Thread concurrency Check
		print_thread_concurrency_check($line);
	}
	if ($valgrind_check_type == 1) {
		#Print only for Memory leak Check
		print_memory_leak_check($line);
	}
}
close (FILE);

