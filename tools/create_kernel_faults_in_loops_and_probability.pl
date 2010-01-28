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
# File :        create_kernel_faults_in_loops_and_probability.pl	      ##
#                                                                             ##
# Usage:        create_kernel_faults_in_loops_and_probability.pl\	      ##
#		<LTP_COMMAND_FILE> <NO_OF_LOOPS_EACH_TEST_WILL_RUN>\	      ##
#		<PROBABILITY_OF_FAULT_INJECTION>			      ##
#                                                                             ##
# Description:  This is a simple perl script which will take ltp command file ##
#		as input and then create a final command file while will have ##
#		the following entries for each test tag:		      ##
#		1) <tag_name_loop1> <test_binary_name>			      ##
#		2) <tag_name_loop2> <insert_kernel_faults.sh test_binary_name>##
#		3) tag entried from loop3 to loop(n-1)			      ##
#		4) <tag_name_loopn_under_kernel_fault> <restore_kernel_faults_default.sh test_binary_name>##
#                                                                             ##
# Author:       Subrata Modak <subrata@linux.vnet.ibm.com>                    ##
#                                                                             ##
# History:      Aug 11 2009 - Created - Subrata Modak.                        ##
#		Aug 17 2009 - Made some changes as specified by Paul Larson   ##
#		1) tag the results to say when a fault was or was not in the  ##
#		process of being generated - Subrata Modak.		      ##
################################################################################

my $command_file	= shift (@ARGV) || syntax();
my $loops		= shift (@ARGV) || syntax();
my $failure_probability	= shift (@ARGV) || syntax();

sub syntax() {
	print "syntax: create_fault_in_loops_and_probability.pl\
	<LTP_COMMAND_FILE> <NO_OF_LOOPS_EACH_TEST_WILL_RUN>\
	<PROBABILITY_OF_FAULT_INJECTION>\n";
	exit (1);
}
#$ENV{TEST_START_TIME})


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
	print "$line\n"; #Print one instance for stable execution
	@tag_and_actual_command = split(/\ /, $line);

	#The remaining loops should be running under fault injection
	for ($counter=1; $counter<=$loops; $counter++) {
		my $token_counter = 0;
		foreach my $token (@tag_and_actual_command) {
			if ($token_counter == 0 ) {
				#Time to append the actual command tag with the loop no.
				print $token . "_loop_" . $counter . "_under_fault_kernel ";
				$token_counter++;
				next;
			}
			if ($token_counter == 1 && $counter == 1) {
				#Time to include the fault injection script in the first loop
				print "\$LTPROOT/bin/insert_kernel_faults.sh " . $failure_probability . "; " . $token;
				$token_counter++;
				next;
			}
			print " " . $token . " ";
		}
		if ($counter == $loops) {
			#Time to withdraw the faults once the last loop has been executed
			#Until all faults has been successfully restored to default values...
			#Keep restoring them
			print "; " . "\$LTPROOT/bin/restore_kernel_faults_default.sh; RC=\$?; while [ \$RC -ne 0 ]; do \$LTPROOT/bin/restore_kernel_faults_default.sh; RC=\$?; done\n"
		} else {
			print "\n"
		}
	}

}
close (FILE);

