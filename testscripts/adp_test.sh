#!/bin/bash

while :
do
	cat /proc/[0-9]*/cmdline > /dev/null 2>/dev/null
done
