#!/bin/bash

. pm_include.sh

if [ $# -lt 1 ] ; then
	echo "Usage: ${0} <filename>"
	RC=1
	return ${RC}
fi
filename=${1}

RC=0
grep_command=grep
config_file=/boot/config-$(uname -r)
if [ ! -f ${config_file} ]; then
	echo "MISSING_FILE: can't find the required config file at /boot/config-$(uname -r)"
	echo "Trying for alternate file at /proc/config.gz"
	config_file=/proc/config.gz
fi
if [ ! -f ${config_file} ]; then
	echo "MISSING_FILE: can't find the required config file at /proc/config.gz"
	RC=1
fi
if [ "${config_file}" = "/proc/config.gz" ] ; then
	grep_command=zgrep
fi

while read config_option 
do
	check_config_options "${config_option}=*" ${config_file} ${grep_command}
done < ${filename}
exit $RC
