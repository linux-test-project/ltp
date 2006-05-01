################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2001                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#                                                                              
# File:           file_test.sh
#
# Description: This program tests the file command. The tests are aimed at
#              testing if the file command can recognize some of the commonly
#              used file formats like, tar, tar.gz, rpm, C, ASCII, ELF etc. 
#
# Author:      Manoj Iyer, manjo@mail.utexas.edu
#
# History:     Dec 16 2002 - Created. - Manoj Iyer manjo@austin.ibm.com.
#              Dec 17 2002 - Added.   - GPL header and minor doc changes.
#                            If LTPTMP and LTPBIN are not exported set their
#                            values to /tmp and 'pwd' respectively.
#                          - Added.   - exit status, if any test fails the test
#                            exits with non-zero value if all tests pass test
#                            exits with zero.
#              Dec 18 2002 - Added.   - Code to read environment variable
#                            LTPROOT and TMPBASE and set LTPTMP and LTPBIN
#                            accordingly.
#! /bin/sh


export TST_TOTAL=10                # Number of tests in this testcase

if [ -z "$LTPTMP" -a -z "$TMPBASE" ]
then 
    LTPTMP=/tmp/
else
	LTPTMP=$TMPBASE
fi

if [ -z "$LTPBIN" -a -z "$LTPROOT" ]
then
    LTPBIN=./
else
	LTPBIN=$LTPROOT/testcases/bin/
fi

# set return code RC variable to 0, it will be set with a non-zero return code 
# in case of error. Set TFAILCNT to 0, increment if there occures a failure.

TFAILCNT=0
RC=0

# TEST #1
# Test if file command recognizes ASCII text files.

export TCID=file01
export TST_COUNT=7

$LTPBIN/tst_resm TINFO "TEST #1: file commad recogizes ASCII text files"

cat > $LTPTMP/test_file.txt <<EOF
This is a text file 
to test file command.
EOF

# Execute file command & check for output.
# Expected out put is the string "ASCII English text"

file $LTPTMP/test_file.txt &>$LTPTMP/file.out

if [ $? -eq 0 ]
then
    grep "ASCII text" $LTPTMP/file.out &>/dev/null
    if [ $? -eq 0 ]
    then
        $LTPBIN/tst_resm TPASS "file: Recognised ASCII file correctly"
        rm -f $LTPTMP/test_file.txt
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/file.out \
                "file: Failed to recognise ASCII file correctlyi. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
    fi
else
    $LTPBIN/tst_res TFAIL $LTPTMP/file.out  \
                "file: failed to recognize ASCII file correctly\t\t"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi

# TEST #2
# Test if file command can recognize bash shell script

export TCID=file02
export TST_COUNT=2

$LTPBIN/tst_resm TINFO "TEST #2: file command recognizes bash shell scripts"

cat > $LTPTMP/bash_script.sh <<EOF
#! /bin/bash

echo "this is a shell script"
echo "used to test file commad"

EOF

file $LTPTMP/bash_script.sh &>$LTPTMP/file.out

if [ $? -eq 0 ]
then
    grep "Bourne-Again shell script" $LTPTMP/file.out &>/dev/null
    if [ $? -eq 0 ]
    then
        $LTPBIN/tst_resm TPASS "file: Recognised bash shell script correctly"
        rm -f $LTPTMP/bash_script.sh
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/file.out \
            "file: Failed to recognise bash shell script. Reason"
        TFAILCNT=$(( $TFAILCNT+1 ))
    fi
else
    $LTPBIN/tst_resm TFAIL "file: Failed to recognize bash shell script"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi

# TEST #3
# Test if file command can recognize korn shell script

export TCID=file03
export TST_COUNT=3

$LTPBIN/tst_resm TINFO "TEST #3: file command recognizes korn shell scripts"

cat > $LTPTMP/ksh_script.sh <<EOF
#! /bin/ksh

echo "this is a shell script"
echo "used to test file commad"

EOF

file $LTPTMP/ksh_script.sh &>$LTPTMP/file.out

if [ $? -eq 0 ]
then
    grep "Korn shell script" $LTPTMP/file.out 2>&1 1>/dev/null
    if [ $? -eq 0 ]
    then
        $LTPBIN/tst_resm TPASS "file: recognised korn shell script"
        rm -f $LTPTMP/ksh_script.sh
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/file.out \
            "file: Failed to recognise korn shell script. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
    fi
else
    $LTPBIN/tst_resm TFAIL "File: Failed to recognize korn shell script"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi


# TEST #4
# Test if file command can recognize C shell script

export TCID=file04
export TST_COUNT=4

$LTPBIN/tst_resm TINFO "TEST #4: file command recognizes C shell scripts"

cat > $LTPTMP/C_script.sh <<EOF
#! /bin/csh

echo "this is a shell script"
echo "used to test file commad"

EOF

file $LTPTMP/C_script.sh &>$LTPTMP/file.out

if [ $? -eq 0 ]
then
    grep "C shell script" $LTPTMP/file.out &>/dev/null
    if [ $? -eq 0 ]
    then
        $LTPBIN/tst_resm TPASS "file: Recognised C shell script correctly"
        rm -f $LTPTMP/C_script.sh
    else
        $LTPBIN/tst_resm TFAIL $LTPTMP/file.out \
            "file: Failed to recognise C shell script correctly. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
    fi
else
    $LTPBIN/tst_resm TFAIL "file: Failed to recognize C shell script"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi


# TEST #5
# Test if file command can recognize C program text

export TCID=file05
export TST_COUNT=5

$LTPBIN/tst_resm TINFO "TEST #5: file command recognizes C programs text"

cat > $LTPTMP/cprog.c <<EOF
#include <stdio.h>

main()
{
    printf("Hello Hell\n");
    exit(0);
}
EOF

file $LTPTMP/cprog.c &>$LTPTMP/file.out

if [ $? -eq 0 ]
then
    grep "ASCII C program text" $LTPTMP/file.out &>/dev/null
    if [ $? -eq 0 ]
    then
        $LTPBIN/tst_resm TPASS "file: Recognised C program text correctly"
        rm -f $LTPTMP/cprog.c
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/file.out \
             "file: Failed to Recognize C program text correctly. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
        
    fi
else
    $LTPBIN/tst_resm TFAIL "file: Failed to recognize C programi text"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi


# TEST #6
# Test if file command can recognize ELF binay executables

export TCID=file06
export TST_COUNT=6

$LTPBIN/tst_resm TINFO \
        "TEST #6: file command recognizes ELF executables"


cat > $LTPTMP/cprog.c <<EOF
#include <stdio.h>

main()
{
    printf("Hello Hell\n");
    exit(0);
}
EOF

cc -o $LTPTMP/cprog $LTPTMP/cprog.c &>/dev/null

file $LTPTMP/cprog &>$LTPTMP/file.out

if [ $? -eq 0 ]
then
    grep "ELF .*-bit LSB executable, .*" $LTPTMP/file.out &>/dev/null
    if [ $? -eq 0 ]
    then
        $LTPBIN/tst_resm TPASS "file: Recognized ELF binary executable"
        rm -f $LTPTMP/cprog.c $LTPTMP/cprog
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/file.out \
             "file: Failed to Recognize ELF binary executable. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
    fi
else
    $LTPBIN/tst_resm TFAIL "file: Failed to recognize ELF binay executable"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi


# TEST #7
# Test if file command can recognize tar files

export TCID=file07
export TST_COUNT=7

$LTPBIN/tst_resm TINFO "TEST #7: file command recognizes tar files."

cat > $LTPTMP/file1 <<EOF
This is a simple test file
EOF

cat > $LTPTMP/file2 <<EOF
This is a simple test file
EOF

cat > $LTPTMP/file3 <<EOF
This is a simple test file
EOF

tar -cf $LTPTMP/files.tar $LTPTMP/file1 $LTPTMP/file2 $LTPTMP/file3 &>/dev/null

file $LTPTMP/files.tar &>$LTPTMP/file.out

if [ $? -eq 0 ]
then
    grep "GNU tar archive" $LTPTMP/file.out &>/dev/null
    if [ $? -eq 0 ]
    then
        $LTPBIN/tst_resm TPASS "file: Recognised tar files"
        rm -f $LTPTMP/files.tar # save $LTPTMP/file[123] for next test case
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/file.out \
             "file: Failed to Recognize tar files. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
    fi
else
    $LTPBIN/tst_resm TFAIL "file: Failed to recognize tar files."
    TFAILCNT=$(( $TFAILCNT+1 ))
fi


# TEST #8
# Test if file command can tar zip files

export TCID=file08
export TST_COUNT=8

$LTPBIN/tst_resm TINFO "TEST #8: file command recognizes tar zip files"

tar cf $LTPTMP/files.tar $LTPTMP/file1 $LTPTMP/file2 $LTPTMP/file3 \
    &>$LTPTMP/file.out
if [ $? -ne 0 ]
then
    $LTPBIN/tst_brk TBROK $LTPTMP/file.out NULL \
        "file: tar failed unexpectedly. Reason:"
fi
    
gzip -f $LTPTMP/files.tar
if [ $? -ne 0 ]
then
    $LTPBIN/tst_brk TBROK $LTPTMP/file.out NULL \
        "file: gzip failed unexpectedly. Reason:"
fi
    
file $LTPTMP/files.tar.gz &>$LTPTMP/file.out
if [ $? -eq 0 ]
then
    grep "gzip compressed data, .*" $LTPTMP/file.out &>$LTPTMP/file1.out
    if [ $? -eq 0 ]
    then
        $LTPBIN/tst_resm TPASS "file: Recognised tar zip file"
        rm -f $LTPTMP/files.tar.gz $LTPTMP/file1 $LTPTMP/file2 $LTPTMP/file3
        rm -f $LTPTMP/file1.out
    else
        $LTPBIN/tst_brkm TBROK NULL \
                "expected string: gzip compressed data, deflated,
original filename, \`files.tar'"
        $LTPBIN/tst_res TFAIL $LTPTMP/file.out \
             "file: Failed to Recognize tar zip. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
    fi
else
    $LTPBIN/tst_brk TBROK $LTPTMP/file.out NULL \
        "file: Failed to recognize tar zip file. Reason:"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi


# TEST #9
# Test if file command can recognize RPM files.

export TCID=file09
export TST_COUNT=9

$LTPBIN/tst_resm TINFO "TEST #9: file command recognizes RPM files"
if [ -f /etc/redhat-release ]; then
	bDIR=/usr/src/redhat
	bCMD=rpmbuild
else
	bDIR=/usr/src/packages
	bCMD=rpmbuild
fi

rpmversion=`rpm --version | awk -F ' ' '{print $3}' | cut -d '.' -f1 `

if [ "$rpmversion" -ge "4" ]; then
 gpl="License: GPL"
else
 gpl="Copyright: GPL"
fi


cat > $LTPTMP/files.spec <<EOF

Summary: Dummy package used to test file command
Name: cprog
Version: 0.0.7
Release: 3
$gpl
Group: LillB test case 
Source: ./cprog.c
BuildRoot: /var/tmp/%{name}-buildroot

%description
Dummy RPM package used for testing file command.

%prep
%setup -q

%build
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT
install -s -m 755 cprog $RPM_BUILD_ROOT/tmp

%clean
rm -rf $RPM_BUILD_ROOT

%files -f ./cprog.c
%defattr(-,root,root)
%doc README TODO COPYING ChangeLog

EOF

RC=0
if [ -d $bDIR/SOURCES ]
then
    echo "directory exists" &>$LTPTMP/file.out
else
    mkdir -p $bDIR/SOURCES/ &>$LTPTMP/file.out || RC=$?
fi

if [ $RC -ne 0 ]
then
    $LTPBIN/tst_brk TBROK $LTPTMP/file.out NULL "mkdir: brok. Reason:"
fi

cat > $bDIR/SOURCES/cprog.c <<EOF
#include <stdio.h>

main()
{
    printf("Hello Hell\n");
    exit(0);
}
EOF
if [ $? -ne 0 ]
then
    $LTPBIN/tst_brkm TBROK NULL "cat: failed to create test file cprog.c"
fi

$bCMD -bs  $LTPTMP/files.spec &>$LTPTMP/file.out
if [ $? -ne 0 ]
then
    $LTPBIN/tst_brk TBROK $LTPTMP/file.out NULL "rpm command brok. Reason:"
fi

file $bDIR/SRPMS/cprog-0.0.7-3.src.rpm &>$LTPTMP/file.out

if [ $? -eq 0 ]
then
    grep "RPM v3 src" $LTPTMP/file.out &>/dev/null
    if [ $? -eq 0 ]
    then
        $LTPBIN/tst_resm TPASS "file: Recognised RPM file correctly"
        rm -f $LTPTMP/files.spec
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/file.out \
             "file: Failed to Recognize RPM file. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
        
    fi
else
    $LTPBIN/tst_resm TFAIL "file: Failed to recognize RPM file"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi


# TEST #10
# Test if file command can recognize vmlinuz file

export TCID=file10
export TST_COUNT=10

$LTPBIN/tst_resm TINFO "TEST #10: file command recognizes vmlinuz file"

# Red Hat creates a user-mode-linux vmlinuz file (ends in .uml) - ignore it
KERNFILE=$(find /boot ! -type l -name 'vmlinuz*' | grep -v '.uml' | tail -1)
file $KERNFILE &> $LTPTMP/file.out

if [ $? -eq 0 ]
then
#####
# There are lots of possible strings to look for, given the number
# of different architectures...
#####
    MATCHED=""
    grep -i "kernel" $LTPTMP/file.out && MATCHED="y"
    grep -i "compressed data" $LTPTMP/file.out && MATCHED="y"
    grep -i "x86 boot sector" $LTPTMP/file.out && MATCHED="y"
    if [ -n "$MATCHED" ]
    then
        $LTPBIN/tst_resm TPASS "file: Recognised vmlinuz file correctly"
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/file.out \
             "file: Failed to Recognize vmlinuz correctly. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
    fi
else
    $LTPBIN/tst_resm TFAIL "file: Failed to recognize vmlinuz file"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi

rm -f $LTPTMP/file.out
exit $TFAILCNT
