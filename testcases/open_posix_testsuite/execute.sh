#!/bin/sh

# Copyright (c) 2005, Wipro Technologies Ltd. All rights reserved.
# Created by:  Dr. B. Thangaraju <balat.raju@wipro.com>
#              Prashant P Yendigeri <prashant.yendigeri@wipro.com>
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.
#
# This execute.sh script executes executables and format the results 
# including time statistics like time taken to execute OPTS. 
# This script doesn't use 'make' or 'gcc'. This script will be useful 
# to run test cases on embedded target. 


# Run all the tests in the conformance area.

# Function to calculate starting time
start_func()
{
      START_DATE=`date`
      START_HOUR=`date +%H`
      START_MIN=`date +%M`
      START_SEC=`date +%S`

      if [ $START_HOUR -eq 0 ]
      then
            TOTAL_START_HOUR=0
            
      else
            TOTAL_START_HOUR=`expr $START_HOUR '*' 3600`
      fi

      if [ $START_MIN -eq 0 ]
      then
            TOTAL_START_MIN=0
      else
            TOTAL_START_MIN=`expr $START_MIN '*' 60`
      fi

      TOTAL_START_TEMP=`expr $TOTAL_START_HOUR + $TOTAL_START_MIN`

      TOTAL_START_SECS=`expr $TOTAL_START_TEMP + $START_SEC`
}


end_func_noday_change()
{

 END_DATE=`date`
 END_HOUR=`date +%H`
 END_MIN=`date +%M`
 END_SEC=`date +%S`

 TOTAL_END_HOUR=`expr $END_HOUR '*' 3600`
 TOTAL_END_MIN=`expr $END_MIN '*' 60`
 TOTAL_END_TEMP=`expr $TOTAL_END_HOUR + $TOTAL_END_MIN`

 TOTAL_END_SECS=`expr $TOTAL_END_TEMP + $END_SEC`
 TOTAL_TIME=`expr $TOTAL_END_SECS - $TOTAL_START_SECS`

 TOTAL_HR=`expr $TOTAL_TIME / 3600`
 TOTAL_MIN=`expr $TOTAL_TIME / 60`
 TOTAL_SEC=$TOTAL_TIME

 if [ $TOTAL_SEC -gt 60 ]
 then
        TOTAL_SEC=`expr $TOTAL_SEC % 60`
 fi

 if [ $TOTAL_MIN -gt 60 ]
 then
        TOTAL_MIN=`expr $TOTAL_MIN % 60`
 fi

 if [ $TOTAL_HR -gt 60 ]
 then
        TOTAL_HR=`expr $TOTAL_HR % 60`
 fi

}

# Function to calculate end time
end_func()
{

 END_DATE=`date`
 END_HOUR=`date +%H`
 END_MIN=`date +%M`
 END_SEC=`date +%S`


if [ $END_HOUR -eq 0 ]
then
      TOTAL_END_HOUR=0
else
      TOTAL_END_HOUR=`expr $END_HOUR '*' 3600`
fi


if [ $END_MIN -eq 0 ]   
then
      TOTAL_END_MIN=0
else
      TOTAL_END_MIN=`expr $END_MIN '*' 60`
fi


 TOTAL_END_TEMP=`expr $TOTAL_END_HOUR + $TOTAL_END_MIN`

 TOTAL_END_SECS=`expr $TOTAL_END_TEMP + $END_SEC`
 TOTAL_START_SECS=`expr 86400 - $TOTAL_START_SECS`

 TOTAL_TIME=`expr $TOTAL_END_SECS + $TOTAL_START_SECS`

 TOTAL_HR=`expr $TOTAL_TIME / 3600`

 TOTAL_MIN=`expr $TOTAL_TIME / 60`
 TOTAL_SEC=$TOTAL_TIME

 if [ $TOTAL_SEC -gt 60 ]
 then
      TOTAL_SEC=`expr $TOTAL_SEC % 60`
 fi

 if [ $TOTAL_MIN -gt 60 ]
 then
      TOTAL_MIN=`expr $TOTAL_MIN % 60`
 fi

 if [ $TOTAL_HR -gt 60 ]
 then
      TOTAL_HR=`expr $TOTAL_HR % 60`
 fi
}

# Function to display the Execution Time Statistics
display_func()
{
            echo -ne "\n\n\n\n\t\t*******************************************\n"
            echo -ne "\t\t*         EXECUTION TIME STATISTICS       *\n"
            echo -ne "\t\t*******************************************\n"
            echo -ne "\t\t* START    : $START_DATE *\n"
            echo -ne "\t\t* END      : $END_DATE *\n"
            echo -ne "\t\t* DURATION :                              *\n"
    echo -ne "\t\t*            $TOTAL_HR hours                      *\n"
    echo -ne "\t\t*            $TOTAL_MIN minutes                    *\n"
    echo -ne "\t\t*            $TOTAL_SEC seconds                    *\n"
    echo -ne "\t\t*******************************************\n"

}


# Variables for formatting the OPTS results
declare -i TOTAL=0
declare -i PASS=0
declare -i FAIL=0
declare -i UNRES=0
declare -i UNSUP=0
declare -i UNTEST=0
declare -i INTR=0
declare -i HUNG=0
declare -i SEGV=0
declare -i OTH=0

# Maximum Two minutes waiting time period to execute a test. If it exceeds, the test case will go into the 'HUNG' category.
TIMEOUT_VAL=120

# if gcc available then remove the below line comment else put the t0 in posixtestsuite directory.
#gcc -o t0 t0.c
./t0 0 > /dev/null 2>&1
TIMEVAL_RET=$?

# Find executable files from the conformance directory
# If you want to execute any specific test cases, you should modify here.

FINDFILESsh=`find ./conformance/ -name '*-*.sh' -print`
FINDFILES=`find ./conformance/ -name '*-*.test' -print | grep -v core`

NEWSTR=`echo $FINDFILES $FINDFILESsh`


# Main program

start_func
PM_TO_AM=`date +%P`
if [ $PM_TO_AM  = "pm" ]
then
      COUNT=1
fi
echo "Run the conformance tests"
echo "=========================================="

count=1
while $TRUE
do
      FILE=`echo $NEWSTR | cut -f$count -d" "`
        if [ -z $FILE ]
      then

PM_TO_AM=`date +%P`
if [ $PM_TO_AM = "am" ]
then
      COUNT=`expr $COUNT + 1`
fi

if [ $COUNT -eq 2 ]
then
      end_func
else
      end_func_noday_change
fi
            echo
            echo -ne "\t\t***************************\n"
            echo -ne "\t\t CONFORMANCE TEST RESULTS\n"
            echo -ne "\t\t***************************\n"
            echo -ne "\t\t* TOTAL:  " $TOTAL "\n"
            echo -ne "\t\t* PASSED: " $PASS "\n"
            echo -ne "\t\t* FAILED: " $FAIL "\n"
            echo -ne "\t\t* UNRESOLVED: " $UNRES "\n"
            echo -ne "\t\t* UNSUPPORTED: " $UNSUP "\n"
            echo -ne "\t\t* UNTESTED: " $UNTEST "\n"
            echo -ne "\t\t* INTERRUPTED: " $INTR "\n"
            echo -ne "\t\t* HUNG: " $HUNG "\n"
            echo -ne "\t\t* SEGV: " $SEGV "\n"  
            echo -ne "\t\t* OTHERS: " $OTH "\n" 
      echo -ne "\t\t***************************\n"
            display_func
                  echo "Finished"
            exit

      elif [ -x $FILE ]
            then
            FILEcut=`echo $FILE | cut -b3-80`
            TOTAL=$TOTAL+1
            ./t0 $TIMEOUT_VAL $FILE > /dev/null 2>&1

            RET_VAL=$?

      if [ $RET_VAL -gt 5  -a  $RET_VAL -ne $TIMEVAL_RET ]
      then 
            INTR_VAL=10
      fi 

      case $RET_VAL in

      0) 
                  PASS=$PASS+1
            echo  "$FILEcut:execution:PASS "
            ;;
      1)     
                  FAIL=$FAIL+1
            echo  "$FILEcut:execution:FAIL "
            ;;

      
      255)     
                  FAIL=$FAIL+1
            echo  "$FILEcut:execution:FAIL "
            ;;


      2)
            UNRES=$UNRES+1
            echo  "$FILEcut:execution:UNRESOLVED "
            ;;

      3)
            ;;

      4)
            UNSUP=$UNSUP+1
            echo  "$FILEcut:execution:UNSUPPORTED "
            ;;

      5)
            UNTEST=$UNTEST+1
            echo  "$FILEcut:execution:UNTESTED "
            ;;

      10) 
            INTR=$INTR+1
            echo  "$FILEcut:execution:INTERRUPTED "
            ;;

      $TIMEVAL_RET)
                  HUNG=$HUNG+1      
                  echo  "$FILEcut:execution:HUNG "
                  ;;
      139)
            SEGV=$SEGV+1
            echo "$FILEcut:execution:Segmentaion Fault "
            ;;
            
      *)
            OTH=$OTH+1
            echo "OTHERS: RET_VAL for $FILE : $RET_VAL"
            ;;
      esac
      fi
      count=`expr $count + 1`
done


######################################################################################
 


