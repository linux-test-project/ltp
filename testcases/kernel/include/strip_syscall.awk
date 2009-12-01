#!/usr/bin/awk -f
#
# Dumb script that can be used to strip all of the syscall information from
# the arch-respective unistd*.h.
#
# Examples:
#
# 1. Grab the i386 32-bit syscalls from unistd_32.h and put them in i386.in
# strip_syscall.awk arch/x86/include/asm/unistd_32.h > i386.in
#

/^#define[[:space:]]+__NR_[0-9a-z]+/ {

	sub (/#define[[:space:]]+__NR_/, "", $0);
	sub (/[[:space:]]*(\/\*.*)/, "", $0);
	sub (/[[:space:]]+/, " ", $0);

	print
}
