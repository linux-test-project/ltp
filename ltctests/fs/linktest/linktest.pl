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


`mkdir hlink`;
`mkdir slink`;
`touch hlink/hfile`;
`touch slink/sfile`;

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
unlink <hlink/hfile*>;
unlink <slink/sfile*>;
rmdir hlink;
rmdir slink;
printf ("Hard Link Errors    :%d\n",$herrors);
printf ("Symbolic Link Errors:%d\n",$serrors);

