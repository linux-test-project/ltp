#!/bin/sh

COUNT=0
FILE=1K_file
for the_file in `echo 10K_file 100K_file 1000K_file`
do
  while [[ $COUNT -le 10 ]]
  do
    cat $FILE >> $the_file
    COUNT=$[ COUNT + 1 ]
  done
  COUNT=0
  FILE=$the_file
done

mkdir dumpdir
cp 1K_file dumpdir
cp 10K_file dumpdir
cp 100K_file dumpdir
