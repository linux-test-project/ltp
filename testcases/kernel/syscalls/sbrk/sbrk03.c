// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Linux Test Project.
 */

/*
 * DESCRIPTION
 *
 * Total s390 2^31 addr space is 0x80000000.
 *
 *     0x80000000 - 0x10000000 = 0x70000000
 *
 * 0x70000000 is a valid positive intptr_t and adding it to the current offset
 * produces a valid uintptr_t without overflow (since the MSB being set is OK),
 * but that is irrelevant for s390 since it has 31-bit pointers and not 32-bit
 * pointers. Consequently, the brk syscall behaves incorrectly with the invalid
 * address and changes the program break to the overflowed address. The glibc
 * part of the implementation detects this overflow and returns a failure with
 * ENOMEM, but does not reset the program break.
 *
 * So the bug is in sbrk as well as the brk syscall. brk() should validate the
 * address being passed and return an error. sbrk() should not result in a brk
 * call at all for an invalid address. One could argue in favour of fixing brk
 * in glibc, but it should be the kernel since one could call the syscall
 * directly without using the glibc entry points.
 *
 * The kernel part was fixed on v3.15 by commits:
 *     473a06572fcd (s390/compat: convert system call wrappers to C part 02)
 *
 * Note:
 *     The reproducer should be built(gcc -m31) in 32bit on s390 platform
 *
 */

#include <stdio.h>
#include <unistd.h>
#include "lapi/abisize.h"
#include "tst_test.h"

static void sbrk_test(void)
{
	void *ret1, *ret2;

	/* set bkr to 0x10000000 */
	tst_res(TINFO, "initial brk: %d", brk((void *)0x10000000));

	/* add 0x10000000, up to total of 0x20000000 */
	tst_res(TINFO, "sbrk increm: %p", sbrk(0x10000000));
	ret1 = sbrk(0);

	/* sbrk() returns -1 on s390, but still does overflowed brk() */
	tst_res(TINFO, "sbrk increm: %p", sbrk(0x70000000));
	ret2 = sbrk(0);

	if (ret1 != ret2) {
		tst_res(TFAIL, "Bug! sbrk: %p", ret2);
		return;
	}

	tst_res(TPASS, "sbrk verify: %p", ret2);
}

static struct tst_test test = {
	.test_all = sbrk_test,
	.supported_archs = (const char *const []) {
		"s390",
		NULL
	},
	.needs_abi_bits = 32,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "473a06572fcd"},
		{}
	}
};
