#!/bin/sh -eu
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Generate the syscalls.h file, merging all architectures syscalls input file
# which are in the current folder and defined inside supported-arch.txt file.

SYSCALLS_FILE="$1"

if [ -z "${SYSCALLS_FILE}" ]; then
	echo "Please provide the syscalls.h directory:"
	echo ""
	echo "$0 path/of/syscalls.h"
	echo ""
	exit 1
fi

SCRIPT_DIR="$(realpath $(dirname "$0"))"
SUPPORTED_ARCH="${SCRIPT_DIR}/supported-arch.txt"

echo '// SPDX-License-Identifier: GPL-2.0-or-later
/************************************************
 * GENERATED FILE: DO NOT EDIT/PATCH THIS FILE  *
 *  change your arch specific .in file instead  *
 ************************************************/

/*
 * Here we stick all the ugly *fallback* logic for linux
 * system call numbers (those __NR_ thingies).
 */

#ifndef LAPI_SYSCALLS_H__
#define LAPI_SYSCALLS_H__

#include <errno.h>
#include <sys/syscall.h>
#include <asm/unistd.h>

#ifdef TST_TEST_H__
#define TST_SYSCALL_BRK__(NR, SNR) ({ \
tst_brk(TCONF, \
	"syscall(%d) " SNR " not supported on your arch", NR); \
})
#else
inline static void dummy_cleanup(void) {}

#define TST_SYSCALL_BRK__(NR, SNR) ({ \
tst_brkm(TCONF, dummy_cleanup, \
	"syscall(%d) " SNR " not supported on your arch", NR); \
})
#endif

#define tst_syscall(NR, ...) ({ \
intptr_t tst_ret; \
if (NR == __LTP__NR_INVALID_SYSCALL) { \
	errno = ENOSYS; \
	tst_ret = -1; \
} else { \
	tst_ret = syscall(NR, ##__VA_ARGS__); \
} \
if (tst_ret == -1 && errno == ENOSYS) { \
	TST_SYSCALL_BRK__(NR, #NR); \
} \
tst_ret; \
})

#define __LTP__NR_INVALID_SYSCALL -1' >${SYSCALLS_FILE}

while IFS= read -r arch; do
	(
		echo
		case ${arch} in
		sparc64) echo "#if defined(__sparc__) && defined(__arch64__)" ;;
		sparc) echo "#if defined(__sparc__) && !defined(__arch64__)" ;;
		s390) echo "#if defined(__s390__) && !defined(__s390x__)" ;;
		mips64n32) echo "#if defined(__mips__) && defined(_ABIN32)" ;;
		mips64) echo "#if defined(__mips__) && defined(_ABI64)" ;;
		mipso32) echo "#if defined(__mips__) && defined(_ABIO32) && _MIPS_SZLONG == 32" ;;
		parisc) echo "#ifdef __hppa__" ;;
		loongarch64) echo "#ifdef __loongarch__" ;;
		arm64) echo "#ifdef __aarch64__" ;;
		*) echo "#ifdef __${arch}__" ;;
		esac

		while read -r line; do
			set -- ${line}
			syscall_nr="__NR_$1"
			shift

			echo "# ifndef ${syscall_nr}"
			echo "#  define ${syscall_nr} $*"
			echo "# endif"
		done <"${SCRIPT_DIR}/${arch}.in"
		echo "#endif"
		echo
	) >>${SYSCALLS_FILE}
done <${SUPPORTED_ARCH}

(
	echo
	echo "/* Common stubs */"
	for num in $(awk '{print $1}' "${SCRIPT_DIR}/"*.in | sort -u); do
		syscall_nr="__NR_${num}"

		echo "# ifndef ${syscall_nr}"
		echo "#  define ${syscall_nr} __LTP__NR_INVALID_SYSCALL"
		echo "# endif"
	done
	echo "#endif"
) >>${SYSCALLS_FILE}
