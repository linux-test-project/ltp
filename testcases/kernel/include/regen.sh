#!/bin/sh

output="linux_syscall_numbers.h"
rm -f "${output}".[1-9]*
output_pid="${output}.$$"

max_jobs=$(getconf _NPROCESSORS_ONLN 2>/dev/null)
: ${max_jobs:=1}

srcdir=${0%/*}

err() {
	echo "$*" 1>&2
	exit 1
}

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

#define ltp_syscall(NR, ...) ({ \\
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

jobs=0
for arch in $(cat "${srcdir}/order") ; do
	(
	echo "Generating data for arch $arch ... "

	(
	echo
	echo "#ifdef __${arch}__"
	while read line ; do
		set -- ${line}
		nr="__NR_$1"
		shift
		if [ $# -eq 0 ] ; then
			err "invalid line found"
		fi
		cat <<-EOF
		# ifndef ${nr}
		#  define ${nr} $*
		# endif
		EOF
	done < "${srcdir}/${arch}.in"
	echo "#endif"
	echo
	) >> "${output_pid}.${arch}"

	) &

	: $(( jobs += 1 ))
	if [ ${jobs} -ge ${max_jobs} ] ; then
		wait || exit 1
		jobs=0
	fi
done

echo "Generating stub list ... "
(
echo
echo "/* Common stubs */"
for nr in $(awk '{print $1}' "${srcdir}/"*.in | sort -u) ; do
	nr="__NR_${nr}"
	cat <<-EOF
	# ifndef ${nr}
	#  define ${nr} 0
	# endif
	EOF
done
echo "#endif"
) >> "${output_pid}._footer"

wait || exit 1

printf "Combining them all ... "
for arch in $(cat "${srcdir}/order") _footer ; do
	cat "${output_pid}.${arch}"
done >> "${output_pid}"
mv "${output_pid}" "${output}"
rm -f "${output_pid}"*
echo "OK!"
