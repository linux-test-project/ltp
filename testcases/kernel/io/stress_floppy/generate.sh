#!/bin/sh

COUNT=0
FILE=1K_file
for the_file in `echo 10K_file 100K_file 1000K_file`
do
  if [ -e $the_file ]; then continue; fi
  echo "Creating $the_file"
  while [ $COUNT -le 10 ]
  do
    cat $FILE >> $the_file
    COUNT=$(( $COUNT + 1 ))
  done
  COUNT=0
  FILE=$the_file
done

if [ ! -d dumpdir ]; then
   mkdir dumpdir
fi

`diff --brief 1K_file dumpdir/1K_file &> /dev/null`
if [ $? ]; then
   cp 1K_file dumpdir
fi

`diff --brief 10K_file dumpdir/10K_file &> /dev/null`
if [ $? ]; then
   cp 10K_file dumpdir
fi

`diff --brief 100K_file dumpdir/100K_file &> /dev/null`
if [ $? ]; then
   cp 100K_file dumpdir
fi
