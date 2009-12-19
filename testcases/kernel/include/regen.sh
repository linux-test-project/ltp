#!/bin/sh

output="linux_syscall_numbers.h"
output_pid="${output}.$$"

srcdir=${0%/*}

cat << EOF > "${output_pid}"
/************************************************
 * GENERATED FILE: DO NOT EDIT/PATCH THIS FILE  *
 *  change your arch specific .in file instead  *
 ************************************************/

/*
 * Here we stick all the ugly *fallback* logic for linux
 * system call numbers (those __NR_ thingies).
 *
 * Licensed under the GPLv2 or later, see the COPYING file.
 */

#ifndef __LINUX_SYSCALL_NUMBERS_H__
#define __LINUX_SYSCALL_NUMBERS_H__

#include <errno.h>
#include <sys/syscall.h>
#include "cleanup.c"

#define syscall(NR, ...) ({ \\
	int __ret; \\
	if (NR == 0) { \\
		errno = ENOSYS; \\
		__ret = -1; \\
	} else { \\
		__ret = syscall(NR, ##__VA_ARGS__); \\
	} \\
	if (__ret == -1 && errno == ENOSYS) { \\
		tst_brkm(TCONF, CLEANUP, \\
			"syscall " #NR " not supported on your arch"); \\
		errno = ENOSYS; \\
	} \\
	__ret; \\
})

EOF

for arch in $(cat "${srcdir}/order") ; do
	echo -n "Generating data for arch $arch ... "

	echo "" >> "${output_pid}"
	echo "#ifdef __${arch}__" >> "${output_pid}"
	while read line ; do
		set -- $line
		nr="__NR_$1"
		shift
		if [ -z "$*" ] ; then
			echo "invalid line found"
			exit 1
		fi
		cat <<-EOF >> "${output_pid}"
		# ifndef $nr
		#  define $nr $*
		# endif
		EOF
	done < "${srcdir}/${arch}.in"
	echo "#endif" >> "${output_pid}"
	echo "" >> "${output_pid}"

	echo "OK!"
done

echo -n "Generating stub list ... "
echo "" >> "${output_pid}"
echo "/* Common stubs */" >> "${output_pid}"
for nr in $(awk '{print $1}' "${srcdir}/"*.in | sort -u) ; do
	nr="__NR_$nr"
	cat <<-EOF >> "${output_pid}"
	# ifndef $nr
	#  define $nr 0
	# endif
	EOF
done
echo "#endif" >> "${output_pid}"

mv "${output_pid}" "${output}"
echo "OK!"
