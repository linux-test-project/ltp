#!/bin/sh
# This script should be run to execute the ACPI (Avanced Control Power & Integration) tests.
# One word of warning, since ACPI will control devices and possibly put them to sleep,
# it's not recommended that any other tests be run concurrently with these tests. You may
# get unexpected results.
# 06/10/03 mridge@us.ibm.com initial script created

cd `dirname $0`
export LTPROOT=${PWD}
export TMPBASE="/tmp"
run=$run
ltproot=$TPROOT
tmpdir=$TMPBASE


usage() 
{
	cat <<-END >&2
	usage: ${0##*/} [ -r run ] 
                

	example: ${0##*/} -r run

        ACPI must be enabled in the kernel. Since ACPI will control devices and possibly put them to sleep,
        it's not recommended that any other tests be run concurrently with these tests since you may
        get unexpected results. 
        These tests are currently ONLY supported on the 2.5 kernels. 2.4 kernels probably won't even build much 
        less run.

	END
exit
}

while getopts :r: arg
do      case $arg in
		          r)	   run=$OPTARG;;
			
                \?)     echo "************** Help Info: ********************"
                        usage;;
        esac
done

if [ ! -n "$run"  ]; then
  usage;
  exit
fi

echo "****** Loading ACPI test module ******"

insmod ${LTPROOT}/LtpAcpiCmds.ko || insmod ${LTPROOT}/LtpAcpiCmds.o
lsmod
${LTPROOT}/LtpAcpiMain
rmmod LtpAcpiCmds
  
