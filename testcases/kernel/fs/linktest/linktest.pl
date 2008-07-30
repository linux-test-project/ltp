#!/usr/bin/perl

#   Copyright (c) International Business Machines  Corp., 2000
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
#   along with this program;  if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA


#
#  FILE(s)     : linktest.pl README
#  DESCRIPTION : Regression test for max links per file
#  USE         : linktest.pl <number of symlinks> <number of hardlinks>
#  AUTHOR      : Jeff Martin (martinjn@us.ibm.com)
#  HISTORY     : 
#     (05/07/01)v.99  Needed a quick script to test a hardlink limitation that was found. 
#     (28/07/08) Veerendra C <vechandr@in.ibm.com>   
#                     Modified to return proper return code on failure/success. 
#                     Also modified to return errors if unable to create dir/files

$ret = 0;
if ( system("mkdir hlink") ) {
     $ret = -1 ;
}

if ( system("mkdir slink") ) {
     $ret = -1 ;
}

if ( system("touch hlink/hfile") ) {
     $ret = -1 ;
}

if ( system("touch slink/sfile") ) {
     $ret = -1 ;
}

if ( $ret == -1 ) {
    printf ("Error %d: Not able to create dir/file's\n " , $ret);
    exit -1;
}

$scount=shift @ARGV;
chdir "slink";
for($x=0;$x<$scount;$x++) {
   $result=symlink("sfile","sfile$x");
   if(!$result) {
      $serrors++;
      }
   }
chdir "..";
$hcount=shift @ARGV;
for($x=0;$x<$hcount;$x++) {
   $result=link("hlink/hfile","hlink/hfile$x");
   if(!$result) {
      $herrors++;
      }
   }

if($herrors) {
    printf ("linker01 : FAIL Hard Link Errors = %d\n", $herrors);
    $ret = -1
}
else {
    printf ("linker01 : HardLink Test PASS\n" )
}
if($serrors) {
    printf ("linker01 : FAIL Soft Link Errors = %d\n", $serrors);
    $ret = -1
}
else {
    printf ("linker01 : SoftLink Test PASS\n" );
}

unlink <hlink/hfile*>;
unlink <slink/sfile*>;
rmdir hlink;
rmdir slink;
exit $ret;
