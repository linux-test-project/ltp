#!/bin/sh -eu
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2009-2024
# Copyright (c) Marcin Juszkiewicz, 2023-2024
#
# This is an adaptation of the update-tables.sh script, included in the
# syscalls-table project (https://github.com/hrw/syscalls-table) and released
# under the MIT license.
#
# Author: Andrea Cervesato <andrea.cervesato@suse.com>

if [ "$#" -eq "0" ]; then
	echo "Please provide kernel sources:"
	echo ""
	echo "$0 path/to/Linux/kernel/sources"
	echo ""
	exit 1
fi

KERNELSRC="$1"

# to keep sorting in order
export LC_ALL=C

if [ ! -d "${KERNELSRC}" ]; then
	echo "${KERNELSRC} is not a directory"
	exit 1
fi

if [ ! -e "${KERNELSRC}/Makefile" ]; then
	echo "No Makefile in ${KERNELSRC} directory"
	exit 1
fi

TEMP="$(mktemp -d)"
trap 'rm -rf "${TEMP}"' EXIT
KVER="$(make -C ${KERNELSRC} kernelversion -s)"

SCRIPT_DIR="$(realpath $(dirname "$0"))"
SUPPORTED_ARCH="${SCRIPT_DIR}/supported-arch.txt"
LINUX_HEADERS="${TEMP}/headers"

grab_syscall_names_from_tables() {
	for tbl_file in $(find ${KERNELSRC}/arch -name syscall*.tbl); do
		grep -E -v "^(#|$)" "$tbl_file" |
			awk '{ print $3 }' >>${TEMP}/syscall-names.tosort
	done

	grep -E -h "^#define __NR_" \
		${KERNELSRC}/include/uapi/asm-generic/unistd.h \
		${KERNELSRC}/arch/*/include/uapi/asm/unistd.h \
		>>${TEMP}/syscall-names.tosort

	awk '{ print $1 }' ${SCRIPT_DIR}/*.in >>${TEMP}/syscall-names.tosort

	drop_bad_entries
}

drop_bad_entries() {
	grep -E -v "(unistd.h|NR3264|__NR_syscall|__SC_COMP|__NR_.*Linux|__NR_FAST)" \
		${TEMP}/syscall-names.tosort |
		grep -E -vi "(not implemented|available|unused|reserved|spill|xtensa)" |
		grep -E -v "(__SYSCALL|SYSCALL_BASE|SYSCALL_MASK)" |
		sed -e "s/#define\s*__NR_//g" -e "s/\s.*//g" |
		sort -u >${TEMP}/syscall-names.txt
}

generate_table() {
	echo "- $arch"

	local flags="${extraflags:-}"
	if [ "$bits" -eq "32" ]; then
		flags="${flags} -D__BITS_PER_LONG=32"
	fi

	local uppercase_arch=$(echo "$arch" | tr '[:lower:]' '[:upper:]')

	rm -f "${TEMP}/list-syscalls"
	gcc "${TEMP}/list-syscalls.c" -U__LP64__ -U__ILP32__ -U__i386__ \
		-D${uppercase_arch} \
		-D__${arch}__ ${flags} \
		-I "${LINUX_HEADERS}/usr/include/" \
		-o "${TEMP}/list-syscalls" || {
		echo "Failed to compile list-syscalls for $arch" >&2
		if grep -Fxq "$arch" "${SUPPORTED_ARCH}"; then
			return 1
		fi
		return 0
	}

	"${TEMP}/list-syscalls" >"${TEMP}/${arch}.in.tosort"

	sort -k2,2n "${TEMP}/${arch}.in.tosort" >"${TEMP}/${arch}.in"
}

generate_list_syscalls_c() {
	local syscalls

	syscalls=$(awk '{ printf "#ifdef __NR_%s\n\tprintf(\"%s %%d\\n\", __NR_%s);\n#endif\n", $1, $1, $1 }' \
		"${TEMP}/syscall-names.txt")

	cat << EOF >"${TEMP}/list-syscalls.c"
#include <stdio.h>
#include <asm/unistd.h>

int main(void)
{
${syscalls}
	return 0;
}
EOF
}

export_headers() {
	make -s -C ${KERNELSRC} ARCH=${arch} O=${LINUX_HEADERS} \
		headers_install >/dev/null 2>&1
}

do_all_tables() {
	for archdir in ${KERNELSRC}/arch/*; do
		arch=$(basename $archdir)

		bits=64
		extraflags=

		case ${arch} in
		Kconfig|um)
			continue
			;;
		esac

		if [ ! -f "${archdir}/Kconfig" ]; then
			continue
		fi

		export_headers

		case ${arch} in
		arm)
			bits=32
			arch=armoabi generate_table
			arch=arm extraflags=-D__ARM_EABI__ generate_table
			;;
		loongarch)
			# 32-bit variant of loongarch may appear
			arch=loongarch64 extraflags=-D_LOONGARCH_SZLONG=64 generate_table
			;;
		mips)
			arch=mips64 extraflags=-D_MIPS_SIM=_MIPS_SIM_ABI64 generate_table
			bits=32
			arch=mipso32 extraflags=-D_MIPS_SIM=_MIPS_SIM_ABI32 generate_table
			arch=mips64n32 extraflags=-D_MIPS_SIM=_MIPS_SIM_NABI32 generate_table
			;;
		powerpc)
			bits=32 generate_table
			bits=64 arch=powerpc64 generate_table
			;;
		riscv)
			arch=riscv64 extraflags=-D__LP64__ generate_table
			bits=32
			arch=riscv32 extraflags=-D__SIZEOF_POINTER__=4 generate_table
			;;
		s390)
			# Older kernels define both ABIs directly in unistd.h.
			gcc -E -dM -x c -D__s390__ -U__s390x__ \
				-I "${LINUX_HEADERS}/usr/include/" \
				-include asm/unistd.h /dev/null >"${TEMP}/s390-macros"
			if grep -q '^#define __NR_getuid32 ' "${TEMP}/s390-macros"; then
				bits=32 generate_table
			else
				echo "- s390 (31-bit ABI not available, skipping)"
			fi
			bits=64
			arch=s390x generate_table
			;;
		sparc)
			bits=32
			extraflags=-D__32bit_syscall_numbers__ generate_table
			bits=64
			arch=sparc64 extraflags=-D__arch64__ generate_table
			;;
		x86)
			arch=x86_64 extraflags=-D__LP64__ generate_table
			bits=32
			arch=i386 generate_table
			arch=x32 extraflags=-D__ILP32__ generate_table
			;;
		arc | csky | hexagon | m68k | microblaze | nios2 | openrisc | sh | xtensa)
			bits=32 generate_table
			;;
		*)
			generate_table
			;;
		esac
	done
}

copy_supported_arch() {
	while IFS= read -r arch; do
		if [ -f "${TEMP}/${arch}.in" ]; then
			echo "- ${arch}"
			cp "${TEMP}/${arch}.in" "${SCRIPT_DIR}/${arch}.in"
		fi
	done <${SUPPORTED_ARCH}
}

echo "Extracting syscalls from Linux ${KVER}"

grab_syscall_names_from_tables
generate_list_syscalls_c

do_all_tables

echo "Copying supported syscalls"
copy_supported_arch
