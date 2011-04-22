#!/usr/bin/env perl

################################################################################ 
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2008                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
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
## Author:      Veerendra <veeren@linux.vnet.ibm.com>                         ##
################################################################################ 

use File::Temp 'tempdir';
use Net::FTP;

if ($#ARGV == -1) {
	print "usage: $0 host\n";
	exit 1;
}
my $host =  $ARGV[0];

my $newname;
my $i = 0;
my $kount = 51;
my $file="junkfile";

my $tmpdir = defined($ENV{TMPDIR}) ? $ENV{TMPDIR} : "/tmp";

my $dir;
$dir = tempdir("container_ftp.XXXXXXX", DIR => $tmpdir);
if (!defined($dir)) {
	push @ERRORS, "Failed to create a temporary directory: $!\n";
	printerr();
}
if (chmod(0777, $dir) == 0) {
	push @ERRORS, "Failed to change mode for temporary directory: $!\n";
	printerr();
}
chdir $dir;
system("dd if=/dev/zero of=$file bs=512 count=10 > /dev/null 2>&1 ");

while ( $i < $kount )
{
        $ftp=Net::FTP->new($host,Timeout=>240) or $newerr=1;
        push @ERRORS, "Can't ftp to $host: $!\n" if $newerr;
        printerr() if $newerr;

        $ftp->login("anonymous","passwd") or $newerr=1;
        push @ERRORS, "Can't login to $host: $!\n" if $newerr;
        $ftp->quit if $newerr;
        printerr() if $newerr; 

        $ftp->cwd($dir) or $newerr=1; 
        push @ERRORS, "Can't cd  $!\n" if $newerr;
        $ftp->quit if $newerr;
        printerr() if $newerr; 

        $newname = $file . "_" . $i ;
        $ftp->put($file,$newname) or $newerr=1;
        push @ERRORS, "Can't get file $file $!\n" if $newerr;
        printerr() if $newerr;

        $i++;
        $ftp->quit;
}

sub printerr {
	print "Error: ";
	print @ERRORS;
	exit 1;
}

END {
	unlink("$dir/$file");
	rmdir("$dir");
}
