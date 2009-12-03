#!/usr/bin/perl

#       $Id: testcoverage.pl,v 1.10 2009/12/03 15:31:01 subrata_modak Exp $

#  (C) Copyright IBM Corp. 2004-2006

#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
#  file and program are licensed under a BSD style license.  See
#  the Copying file included with the OpenHPI distribution for
#  full licensing terms.

#  Authors:
#      Sean Dague <http://dague.net/sean>

use strict;
use Cwd;
use File::Basename;

chdir("../../");

my $start = cwd();

my $report = "";

# this needs to be made more generic over time
my %files = (
             "src/ohpi.c" => ".libs",
             "utils/el_utils.c" => "t/el",
             "utils/epath_utils.c" => "t/epath",
             "utils/rpt_utils.c" => "t/rpt",
             "utils/uid_utils.c" => "t/uid",
             "utils/sahpi_enum_utils.c" => "t/sahpi",
             "utils/sahpi_event_encode.c" => "t/sahpi",
             "utils/sahpi_event_utils.c" => "t/sahpi",
             "utils/sahpi_struct_utils.c" => "t/sahpi",
             "utils/sahpi_time_utils.c" => "t/sahpi",
             # now for the blade center stuff
#             "plugins/snmp_bc/snmp_bc.c" => "t",
#             "plugins/snmp_bc/snmp_bc_control.c" => "t",
#             "plugins/snmp_bc/snmp_bc_discover.c" => "t",
#            "plugins/snmp_bc/snmp_bc_event.c" => "t",
#             "plugins/snmp_bc/snmp_bc_hotswap.c" => "t",
#            "plugins/snmp_bc/snmp_bc_inventory.c" => "t",
#             "plugins/snmp_bc/snmp_bc_sel.c" => "t",
#             "plugins/snmp_bc/snmp_bc_sensor.c" => "t",
#             "plugins/snmp_bc/snmp_bc_session.c" => "t",
#            "plugins/snmp_bc/snmp_bc_time.c" => "t",
#             "plugins/snmp_bc/snmp_bc_utils.c" => "t",
#             "plugins/snmp_bc/snmp_bc_watchdog.c" => "t",

#             "plugins/snmp_bc/t/bc_str2event.c" => ".libs",
#             "plugins/snmp_bc/t/snmp_bc.c" => ".libs",
#             "plugins/snmp_bc/t/snmp_bc_control.c" => ".libs",
#             "plugins/snmp_bc/t/snmp_bc_discover.c" => ".libs",
#            "plugins/snmp_bc/t/snmp_bc_event.c" => ".libs",
#             "plugins/snmp_bc/t/snmp_bc_hotswap.c" => ".libs",
#            "plugins/snmp_bc/t/snmp_bc_inventory.c" => ".libs",
#             "plugins/snmp_bc/t/snmp_bc_sel.c" => ".libs",
#             "plugins/snmp_bc/t/snmp_bc_sensor.c" => ".libs",
#             "plugins/snmp_bc/t/snmp_bc_session.c" => ".libs",
#            "plugins/snmp_bc/t/snmp_bc_time.c" => ".libs",
#             "plugins/snmp_bc/t/snmp_bc_utils.c" => ".libs",
#             "plugins/snmp_bc/t/snmp_bc_watchdog.c" => ".libs",
#             "plugins/snmp_bc/t/snmp_util.c" => ".libs",
            );

# we must ensure that we have coverage created
system("./bootstrap && ./configure --enable-testcover @ARGV && make clean && make && make -sk check");
#system("make -ks clean check");

foreach my $fullfile (sort keys %files) {
    chdir($start);
    my $file = basename($fullfile);
    my $dir = dirname($fullfile);
    chdir($dir);
    print STDERR "Cwd is now" . cwd() . "\n";
    my $cmd = "gcov -blf -o $files{$fullfile} $file";

    my $report = "Coverage Report for $fullfile\n\n";
    my $body = "";
    my $header = "";
    open(GCOV,"$cmd |");
    while(<GCOV>) {
        if(s{^(File.*)($file)}{$1$dir/$file}) {
            $header .= $_; # File
            $header .= <GCOV>; # Lines
            $header .= <GCOV>; # Branches
            $header .= <GCOV>; # Taken
            $header .= <GCOV>; # Calls
            $header .= "\n";
            last; # and now we are *done*
        } else {
            $body .= $_;
        }
    }
    close(GCOV);

    open(OUT,">$file.summary");
    print OUT $report, $header, $body;
    close(OUT);
}
