#! /bin/sh


export TCID=file_cmd_test		# Name of this test case
export TST_TOTAL=8				# Number of tests in this testcase

LTPTMP=/tmp
LTPBIN=../../../bin

# set return code RC variable to 0, it will be set with a non-zero return code 
# in case of error.

RC=0

# TEST #1
# Test if file command recognizes ASCII text files.

export TST_COUNT=1

$LTPBIN/tst_resm TINFO "TEST #1: file commad recogizes ASCII text files"

cat > $LTPTMP/test_file.txt <<EOF
This is a text file 
to test file command.
EOF

# Execute file command & check for output.
# Expected out put is the string "ASCII English text"

file $LTPTMP/test_file.txt &>$LTPTMP/file.out || RC=$?

if [ $RC -eq 0 ]
then
	grep "ASCII text" $LTPTMP/file.out &>/dev/null
	if [ $? -eq 0 ]
	then
		$LTPBIN/tst_resm TPASS "file: Recognised ASCII file correctly"
	else
		$LTPBIN/tst_res TFAIL $LTPTMP/file.out \
				"file: Failed to recognise ASCII file correctlyi. Reason:"
	fi
else
	$LTPBIN/tst_res TFAIL $LTPTMP/file.out  \
				"file: failed to recognize ASCII file correctly\t\t"
fi

# TEST #2
# Test if file command can recognize bash shell script

export TST_COUNT=2

$LTPBIN/tst_resm TINFO "TEST #2: file command recognizes bash shell scripts"

cat > $LTPTMP/bash_script.sh <<EOF
#! /bin/bash

echo "this is a shell script"
echo "used to test file commad"

EOF

file $LTPTMP/bash_script.sh &>$LTPTMP/file.out || RC=$?

if [ $RC -eq 0 ]
then
	grep "Bourne-Again shell script" $LTPTMP/file.out &>/dev/null
	if [ $? -eq 0 ]
	then
		$LTPBIN/tst_resm TPASS "file: Recognised bash shell script correctly"
	else
		$LTPBIN/tst_res TFAIL $LTPTMP/file.out \
			"file: Failed to recognise bash shell script. Reason"
	fi
else
	$LTPBIN/tst_resm TFAIL "file: Failed to recognize bash shell script"
fi

# TEST #3
# Test if file command can recognize korn shell script

export TST_COUNT=3

$LTPBIN/tst_resm TINFO "TEST #3: file command recognizes korn shell scripts"

cat > $LTPTMP/ksh_script.sh <<EOF
#! /bin/ksh

echo "this is a shell script"
echo "used to test file commad"

EOF

file $LTPTMP/ksh_script.sh &>$LTPTMP/file.out || RC=$?

if [ $RC -eq 0 ]
then
	grep "Korn shell script" $LTPTMP/file.out 2>&1 1>/dev/null
	if [ $? -eq 0 ]
	then
		$LTPBIN/tst_resm TPASS "file: recognised korn shell script"
	else
		$LTPBIN/tst_res TFAIL $LTPTMP/file.out \
			"file: Failed to recognise korn shell script. Reason:"
	fi
else
	$LTPBIN/tst_resm TFAIL "File: Failed to recognize korn shell script"
fi


# TEST #4
# Test if file command can recognize C shell script

export TST_COUNT=4

$LTPBIN/tst_resm TINFO "TEST #4: file command recognizes C shell scripts"

cat > $LTPTMP/C_script.sh <<EOF
#! /bin/csh

echo "this is a shell script"
echo "used to test file commad"

EOF

file $LTPTMP/C_script.sh &>$LTPTMP/file.out || RC=$?

if [ $RC -eq 0 ]
then
	grep "C shell script" $LTPTMP/file.out &>/dev/null
	if [ $? -eq 0 ]
	then
		$LTPBIN/tst_resm TPASS "file: Recognised C shell script correctly"
	else
		$LTPBIN/tst_resm TFAIL $LTPTMP/file.out \
			"file: Failed to recognise C shell script correctly. Reason:"
	fi
else
	$LTPBIN/tst_resm TFAIL "file: Failed to recognize C shell script"
fi


# TEST #5
# Test if file command can recognize C program text

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

file $LTPTMP/cprog.c &>$LTPTMP/file.out || RC=$?

if [ $RC -eq 0 ]
then
	grep "ASCII C program text" $LTPTMP/file.out &>/dev/null
	if [ $? -eq 0 ]
	then
		$LTPBIN/tst_resm TPASS "file: Recognised C program text correctly"
	else
		$LTPBIN/tst_res TFAIL $LTPTMP/file.out \
			 "file: Failed to Recognize C program text correctly. Reason:"
		
	fi
else
	$LTPBIN/tst_resm TFAIL "file: Failed to recognize C programi text"
fi


# TEST #6
# Test if file command can recognize ELF binay executables

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

cc -o /tmp/cprog /tmp/cprog.c &>/dev/null

file $LTPTMP/cprog &>$LTPTMP/file.out || RC=$?

if [ $RC -eq 0 ]
then
	grep "ELF 32-bit LSB executable, Intel 80386" $LTPTMP/file.out &>/dev/null
	if [ $? -eq 0 ]
	then
		$LTPBIN/tst_resm TPASS "file: Recognized ELF binary executable"
	else
		$LTPBIN/tst_res TFAIL $LTPTMP/file.out \
			 "file: Failed to Recognize ELF binary executable. Reason:"
		
	fi
else
	$LTPBIN/tst_resm TFAIL "file: Failed to recognize ELF binay executable"
fi


# TEST #7
# Test if file command can recognize tar files

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

file $LTPTMP/files.tar &>$LTPTMP/file.out || RC=$?

if [ $RC -eq 0 ]
then
	grep "GNU tar archive" $LTPTMP/file.out &>/dev/null
	if [ $? -eq 0 ]
	then
		$LTPBIN/tst_resm TPASS "file: Recognised tar files"
	else
		$LTPBIN/tst_res TFAIL $LTPTMP/file.out \
			 "file: Failed to Recognize tar files. Reason:"
	fi
else
	$LTPBIN/tst_resm TFAIL "file: Failed to recognize tar files."
fi


# TEST #8
# Test if file command can tar zip files

export TST_COUNT=8

$LTPBIN/tst_resm TINFO "TEST #8: file command recognizes tar zip files"

tar cf $LTPTMP/files $LTPTMP/file1 $LTPTMP/file2 $LTPTMP/file3 \
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
	
file $LTPTMP/files.tar.gz &>$LTPTMP/file.out || RC=$?
if [ $RC -eq 0 ]
then
	grep "gzip compressed data, deflated, original filename, \`files.tar'" \
			$LTPTMP/file.out &>$LTPTMP/file1.out
	if [ $? -eq 0 ]
	then
		$LTPBIN/tst_resm TPASS "file: Recognised tar zip file"
	else
		$LTPBIN/tst_brkm TBROK NULL \
				"expected string: gzip compressed data, deflated,
original filename, \`files.tar'"
		$LTPBIN/tst_res TFAIL $LTPTMP/file.out \
			 "file: Failed to Recognize tar zip. Reason:"
	fi
else
	$LTPBIN/tst_brk TBROK $LTPTMP/file.out NULL \
		"file: Failed to recognize tar zip file. Reason:"
fi


# TEST #9
# Test if file command can recognize RPM files.

export TST_COUNT=9

$LTPBIN/tst_resm TINFO "TEST #9: file command recognizes RPM files"

cat > $LTPTMP/files.spec <<EOF

Summary: Dummy package used to test file command
Name: cprog
Version: 0.0.7
Release: 3
Copyright: GPL
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

if [ -d /usr/src/packages/SOURCES ]
then
	echo "directory exists" &>$LTPTMP/file.out
else
	mkdir -p /usr/src/packages/SOURCES/ &>$LTPTMP/file.out || RC=$?
fi

if [ $RC -ne 0 ]
then
	$LTPBIN/tst_brk TBROK $LTPTMP/file.out NULL "mkdir: brok. Reason:"
fi

cat > /usr/src/packages/SOURCES/cprog.c <<EOF || RC=$?
#include <stdio.h>

main()
{
    printf("Hello Hell\n");
    exit(0);
}
EOF
if [ $RC -ne 0 ]
then
	$LTPBIN/tst_brkm TBROK NULL "cat: failed to create test file cprog.c"
fi

rpm -bs $LTPTMP/files.spec &>$LTPTMP/file.out
if [ $? -ne 0 ]
then
	$LTPBIN/tst_brk TBROK $LTPTMP/file.out NULL "rpm command brok. Reason:"
fi

file /usr/src/packages/SRPMS/cprog-0.0.7-3.src.rpm &>$LTPTMP/file.out || RC=$?

if [ $RC -eq 0 ]
then
	grep "RPM v3 src i386" $LTPTMP/file.out &>/dev/null
	if [ $? -eq 0 ]
	then
		$LTPBIN/tst_resm TPASS "file: Recognised RPM file correctly"
	else
		$LTPBIN/tst_res TFAIL $LTPTMP/file.out \
			 "file: Failed to Recognize RPM file. Reason:"
		
	fi
else
	$LTPBIN/tst_resm TFAIL "file: Failed to recognize RPM file"
fi
