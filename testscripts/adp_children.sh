#!/bin/bash

while :
do
	for i in 1 2 3 4 5 6 7 8 9 0
	do
		sleep 1 &
	done

	wait
done
