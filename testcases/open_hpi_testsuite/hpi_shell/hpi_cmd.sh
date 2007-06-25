#! /bin/sh
#
# hpi_cmd.sh    Start the hpi_shell program.
#
# description: hpi_cmd script starts hpi_shell.
# 
# Author(s):
#	Kouzmich	< Mikhail.V.Kouzmich@intel.com >

while getopts c:ef: arg
do  case $arg in
	c)
		CONF_FILE=$OPTARG
		export OPENHPI_CONF=${CONF_FILE} ;;
	e)
		;;
	f)
		;;
	\?)
		echo "Usage: $0 [-c <cfgfile>][-e][-f <file>]"
		echo "	-c <cfgfile> - use passed file as configuration file"
		echo "	-e - show short events, discover after subscribe"
		echo "	-f <file> - execute command file"
		exit 1 ;;
	esac
done

BASEDIR=`dirname $0`
${BASEDIR}/hpi_shell $*
