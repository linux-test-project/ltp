#!/bin/sh
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
#   PURPOSE: Creates dat for use in network file transfer tests.
#
#   AUTHOR: Robbie Williamson (robbiew@us.ibm.com)
#
############################################################################


COUNT=0
LIMIT=10

mkdir dat 2>/dev/null

for the_file in `echo smallsize.fil medsize.fil largesize.fil maxsize.fil`
do
  while [ $COUNT -le $LIMIT ]
  do
    echo -n "AAAAAAAAAA" >> dat/$the_file
    COUNT=$[ COUNT + 1 ] 
  done
  LIMIT=$[ LIMIT * 12 ]
  COUNT=0
  cat dat/$the_file > tmpfile
  cat tmpfile >> dat/$the_file
  rm -f tmpfile
  chmod 666 dat/$the_file
done

