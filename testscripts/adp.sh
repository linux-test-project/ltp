#!/bin/bash
#
# adp.sh -- run ADP's stress test on /proc/[0-9]*/cmdline
#
#

usage()
{
  cat << EOF
  usage: $0 options

  This script runs ADP's stress test on /proc/[0-0]*/cmdline.

  OPTIONS
    -h    display this message and exit
    -d    delay for top, in seconds
    -n    number of iterations for top
EOF
}


checkvar()
{
  VAR=$1
  eval VALUE='$'$VAR
  if [ "x$VALUE" = "x" ]; then
      echo "`basename $0`: $VAR is not set."
      return 1
	else
      return 0
  fi
}


while getopts hd:n: OPTION
do
  case $OPTION in
    h)
      usage
      exit 1
      ;;
    d)
      delay=$OPTARG
      ;;
    n)
      iterations=$OPTARG
      ;;
    ?)
      usage
      exit 1
      ;;
  esac
done


#check all vars
checkvar delay && checkvar iterations || {
  usage
  exit 2
}

echo "-------------------------------------------------------------------------"
date
echo "Starting tests..."

for i in 1 2 3 4 5
do
	./adp_children.sh &
done

echo "Stressing /proc/[0-9]*/cmdline..."

for i in 1 2 3 4 5
do
	./adp_test.sh &
done

echo "Starting 'top', redirecting output to 'adp.log'..."
top -b -d $delay -n $iterations > adp.log &

echo "LTP ADP Test done. Killing processes..."
killall adp_test.sh
killall adp_children.sh

echo "Done. Please check adp.log."
date

echo "-------------------------------------------------------------------------"

