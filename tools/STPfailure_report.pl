#!/usr/bin/perl

####################################################
#    Copyright (c) Open Source Development Labs, 2004
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
#   FILE        : STPfailure_report.pl
#   DESCRIPTION : A script that will retrieve the run results through the net from
#		  the STP results and then finds the corresponding source code
#		  file in your LTP tree for each failure.  It then prints the
#		  description and details for each to STDOUT.
#   REQUIREMENTS: LWP::Simple, File::Find, and Text::Wrap must be installed 
#   AUTHOR      : Bryce Harrington <bryce@osdl.org>       
#   HISTORY     :
#       04/28/2004 Robbie Williamson (robbiew@austin.ibm.com)
#               Adapted for and added to LTP
####################################################

use strict;
use LWP::Simple;
use File::Find;
use Text::Wrap;

$Text::Wrap::columns = 72;

my $test_uid = $ARGV[0];
die "Usage: STPfailure_report.pl TEST_RUN_ID\n"
    unless ($test_uid =~ /^\d+$/);

# Location from which to retrieve test results
my $stp_url = "http://khack.osdl.org/stp/$test_uid/";

# Name of the file containing the fail report data
my $fail_report = $stp_url . "results/FAIL_summary.txt";

# Path to the top dir in the locally checked out version of LTP
my $ltp_base = "..";
die "Cannot find testcases directory in '$ltp_base'"
    unless (-d "$ltp_base/testcases");

# Retrieve the results for the test run
my $fail_results = get($fail_report) 
    || die "Could not retrieve URL $fail_report\n";

# Process the results, extracting each test name & generating a report
my $testname    = '';
my $description = '';
my $failures    = '';
foreach my $line (split /\n/, $fail_results) {
    next unless ($line =~ /^(\w+)\s/);

    # Is this a new testname or continuation of the previous?
    if ($1 ne $testname) {
        # Print the current test results
        print_report($testname, $description, $failures);

        # Init variables for next testcase
        $testname    = $1;
        $description = get_description($testname, $ltp_base);
        $failures    = '';
    }
    $failures .= wrap('', ' 'x26, ($line)) . "\n";
}

# Locates the matching .c file and extracts the DESCRIPTION field
sub get_description {
    my $testname = shift || return undef;
    my $dir = shift || return undef;

    # Find $testname.c
    my $path = `find $dir -name '$testname.c'`;

    chomp $path;
    open(FILEHANDLE, "< $path") or return undef;

    # Seek in $testname.c for the DESCRIPTION line
    my $line;
    while (defined ($line = <FILEHANDLE>)) {
        last if ($line =~ /^\s+\*\s+DESCRIPTION/);
        last if ($line =~ /^\s+\*\s+Test Description/);
    }

    # Extract the description
    my $description = '';
    while (defined ($line = <FILEHANDLE>)) {
        last if ($line !~ /^\s+\*\s\s+/);

        # Strip off the leading " * "
        $line =~ s/^\s+\*\s//;

        $description .= $line;
    }
    close(FILEHANDLE);

    $path =~ s|^$dir/||;
    return $description . "\nFor more see '$path'\n";
}

# Prints out the failed test case report
sub print_report {
    my ($testname, $description, $failures) = @_;

    return unless ($testname);

    $description ||= "No description available\n";

    # Remove any trailing newlines
    chomp $description;
    chomp $testname;
    chomp $failures;

    print qq|
========================================================================
Test name: $testname

Description: 

$description

Test Result: FAIL

Details:

$failures

========================================================================

|;
}
