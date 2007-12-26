#!/bin/bash

i=0
while test $i -lt 2000
do
	sleep 10; sh cpunoise.sh &
	i=`expr $i + 1`
done
