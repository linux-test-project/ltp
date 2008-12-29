#!/bin/sh

echo "Starting the Tests"

passes=0
fails=0

if [ ! -e testdir ]
then
         mkdir -m 777 testdir
fi

echo "Test run starting at " `date`

usage() 
{
	cat <<-END >&2
	usage: ${0##*/} [ -a test name ] 

	example: ${0##*/} -a cases/aio_tio

	END
exit
}

while getopts :a: arg
do      case $arg in
		a)	this_test=$OPTARG;;
			
                \?)     echo "************** Help Info: ********************"
                        usage;;
        esac
done

if [ ! -n "$this_test"  ]; then
  echo "Missing the test program. You must pass a test path/name for testing"
  usage;
  exit
fi

#Execute only once at present circumstances
#while [ $# -ge 1 ] ; do
   echo "Starting $this_test"
   $this_test 
   res=$?
   if [ $res -eq 0 ] ; 
   then str="";
   passes=$[passes +1];
   else
   str=" -- FAILED";
   fails=$[fails +1];
   fi
#done

echo "Pass: $passes Fail: $fails"
echo "Test run complete at" `date`

rm -rf testdir
