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
#   PURPOSE: Creates datafiles for use in network file transfer tests.
#
#   AUTHOR: Robbie Williamson (robbiew@us.ibm.com)
#
############################################################################


COUNT=0
LIMIT=10

mkdir datafiles 2>/dev/null

for the_file in `echo ascii.sm ascii.med ascii.lg ascii.jmb`
do
  while [ $COUNT -le $LIMIT ]
  do
    echo -n "AAAAAAAAAA" >> datafiles/$the_file
    COUNT=$[ COUNT + 1 ] 
  done
  (( LIMIT = LIMIT * 20 ))
  COUNT=0
  cat datafiles/$the_file > tmpfile
  cat tmpfile >> datafiles/$the_file
  rm -f tmpfile
  chmod 666 datafiles/$the_file
done



gzip -3 -c datafiles/ascii.lg > datafiles/bin.sm
COUNT=0
while [ $COUNT -lt 3 ]
do
  gzip -9 -c datafiles/ascii.jmb >> datafiles/bin.med
  COUNT=$[ COUNT + 1 ]
done
COUNT=0
while [ $COUNT -lt 12 ]
do
  gzip -1 -c datafiles/ascii.jmb >> datafiles/bin.lg
  COUNT=$[ COUNT + 1 ]
done
COUNT=0
while [ $COUNT -lt 144 ]
do
  gzip -1 -c datafiles/ascii.jmb >> datafiles/bin.jmb
  COUNT=$[ COUNT + 1 ]
done
chmod 666 datafiles/bin.*


