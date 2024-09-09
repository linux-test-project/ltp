#!/bin/sh

output="syscalls.h"
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

#ifndef LAPI_SYSCALLS_H__
#define LAPI_SYSCALLS_H__

#include <errno.h>
#include <sys/syscall.h>
#include <asm/unistd.h>

#ifdef TST_TEST_H__
#define TST_SYSCALL_BRK__(NR, SNR) ({ \\
	tst_brk(TCONF, \\
		"syscall(%d) " SNR " not supported on your arch", NR); \\
})
#else
inline static void dummy_cleanup(void) {}

#define TST_SYSCALL_BRK__(NR, SNR) ({ \\
	tst_brkm(TCONF, dummy_cleanup, \\
		"syscall(%d) " SNR " not supported on your arch", NR); \\
})
#endif

#define tst_syscall(NR, ...) ({ \\
	intptr_t tst_ret; \\
	if (NR == __LTP__NR_INVALID_SYSCALL) { \\
		errno = ENOSYS; \\
		tst_ret = -1; \\
	} else { \\
		tst_ret = syscall(NR, ##__VA_ARGS__); \\
	} \\
	if (tst_ret == -1 && errno == ENOSYS) { \\
		TST_SYSCALL_BRK__(NR, #NR); \\
	} \\
	tst_ret; \\
})

EOF

jobs=0
for arch in $(cat "${srcdir}/order") ; do
	(
	echo "Generating data for arch $arch ... "

	(
	echo
	case ${arch} in
		sparc64) echo "#if defined(__sparc__) && defined(__arch64__)" ;;
		sparc) echo "#if defined(__sparc__) && !defined(__arch64__)" ;;
		s390) echo "#if defined(__s390__) && !defined(__s390x__)" ;;
		mips_n32) echo "#if defined(__mips__) && defined(_ABIN32)" ;;
		mips_n64) echo "#if defined(__mips__) && defined(_ABI64)" ;;
		mips_o32) echo "#if defined(__mips__) && defined(_ABIO32) && _MIPS_SZLONG == 32" ;;
		*) echo "#ifdef __${arch}__" ;;
	esac
	while read line ; do
		set -- ${line}
		nr="__NR_$1"
		shift
		if [ $# -eq 0 ] ; then
			err "invalid line found: $line"
		fi
		echo "# ifndef ${nr}"
		echo "#  define ${nr} $*"
		echo "# endif"
	done < "${srcdir}/${arch}.in"
	echo "#endif"
	echo
	) >> "${output_pid}.${arch}"

	) &

	jobs=$(( jobs + 1 ))
	if [ ${jobs} -ge ${max_jobs} ] ; then
		wait || exit 1
		jobs=0
	fi
done

echo "Generating stub list ... "
(
echo
echo "/* Common stubs */"
echo "#define __LTP__NR_INVALID_SYSCALL -1" >> "${output_pid}"
for nr in $(awk '{print $1}' "${srcdir}/"*.in | sort -u) ; do
	nr="__NR_${nr}"
	echo "# ifndef ${nr}"
	echo "#  define ${nr} __LTP__NR_INVALID_SYSCALL"
	echo "# endif"
done
echo "#endif"
) >> "${output_pid}._footer"

wait || exit 1

printf "Combining them all ... "
for arch in $(cat "${srcdir}/order") _footer ; do
	cat "${output_pid}.${arch}"
done >> "${output_pid}"
mv "${output_pid}" "../${output}"
rm -f "${output_pid}"*
echo "OK!"
