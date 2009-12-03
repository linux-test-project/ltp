#!/usr/bin/perl

#       $Id: testconformance.pl,v 1.10 2009/12/03 15:31:01 subrata_modak Exp $

#  (C) Copyright IBM Corp. 2004

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

# set up env
my $plugroot = "$start/plugins";
$ENV{OPENHPI_CONF} = "$start/openhpi.conf";
$ENV{OPENHPI_UID_MAP} = "$start/uid_map";
$ENV{LD_LIBRARY_PATH} .= "$start/src/.libs:$start/utils/.libs";
$ENV{LIBRARY_PATH} .= "$start/src/.libs:$start/utils/.libs";
$ENV{OPENHPI_PATH} .= "$plugroot/dummy:$plugroot/ipmi:$plugroot/ipmidirect:$plugroot/watchdog:$plugroot/sysfs:$plugroot/text_remote:$plugroot/snmp_bc";


# first we have to rebuild the library with --enable-testcover
system("./bootstrap && ./configure --enable-testcover @ARGV && make clean && make");

chdir("../hpitest");

system("./hpitest --clean --hpiversion B.1.01 openhpi");


foreach my $dir (qw(src utils)) {
    chdir($start);

    my $report = "";

    chdir($dir);
    my @files = ();

    # We are only testing files in openhpi/src for conformance coverage
    opendir(IN,".");
    while(my $file = readdir(IN)) {
        if($file =~ /\.c$/) {
            push @files, "$file";
        }
    }
    close(IN);

    foreach my $file (@files) {

        print STDERR "Cwd is now" . cwd() . "\n";
        my $cmd = "gcov -blf -o .libs $file";

        my $report = "Coverage Report for $dir/$file\n\n";
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
}

