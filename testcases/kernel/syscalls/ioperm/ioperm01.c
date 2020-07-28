// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (c) Linux Test Project, 2020
 *  Copyright (c) Wipro Technologies Ltd, 2002
 */

/*
 * This is a basic test for ioperm(2) system call.
 * It is intended to provide a limited exposure of the system call.
 *
 * Author: Subhab Biswas <subhabrata.biswas@wipro.com>
 */

#include <errno.h>
#include <unistd.h>

#include "tst_test.h"

#if defined __i386__ || defined(__x86_64__)
#include <sys/io.h>

unsigned long io_addr;
#define NUM_BYTES 3
#ifndef IO_BITMAP_BITS
#define IO_BITMAP_BITS 1024
#endif

static void verify_ioperm(void)
{
	TEST(ioperm(io_addr, NUM_BYTES, 1));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "ioperm() failed for port address "
				"%lu,  errno=%d : %s", io_addr,
				TST_ERR, tst_strerrno(TST_ERR));
	} else {
		tst_res(TPASS, "ioperm() passed for port "
				"address %lu, returned %lu",
				io_addr, TST_RET);
	}
}

static void setup(void)
{
	/* ioperm() is restricted under kernel lockdown. */
	if (tst_lockdown_enabled())
		tst_brk(TCONF, "Kernel is locked down, skip this test");

	/*
	 * The value of IO_BITMAP_BITS (include/asm-i386/processor.h) changed
	 * from kernel 2.6.8 to permit 16-bits ioperm
	 *
	 * Ricky Ng-Adam, rngadam@yahoo.com
	 * */
	if (tst_kvercmp(2, 6, 8) < 0)
		io_addr = IO_BITMAP_BITS - NUM_BYTES;
	else
		io_addr = IO_BITMAP_BITS - NUM_BYTES;
}

static void cleanup(void)
{
	/*
	 * Reset I/O privileges for the specified port.
	 */
	if ((ioperm(io_addr, NUM_BYTES, 0)) == -1)
		tst_brk(TBROK | TTERRNO, "ioperm() cleanup failed");
}

static struct tst_test test = {
	.test_all = verify_ioperm,
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
};

#else
TST_TEST_TCONF("LSB v1.3 does not specify ioperm() for this architecture. (only for i386 or x86_64)");
#endif /* __i386_, __x86_64__*/
