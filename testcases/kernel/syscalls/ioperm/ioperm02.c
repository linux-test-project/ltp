// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (c) Linux Test Project, 2020
 *  Copyright (c) Wipro Technologies Ltd, 2002
 */

/*
 * This is an error test for ioperm(2) system call.
 *
 * Verify that
 * 1) ioperm(2) returns -1 and sets errno to EINVAL for I/O port
 *    address greater than 0x3ff.
 * 2) ioperm(2) returns -1 and sets errno to EPERM if the current
 *    user is not the super-user.
 *
 * Author: Subhab Biswas <subhabrata.biswas@wipro.com>
 */

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include "tst_test.h"
#include "tst_safe_macros.h"

#if defined __i386__ || defined(__x86_64__)
#include <sys/io.h>

#define NUM_BYTES 3
#ifndef IO_BITMAP_BITS
#define IO_BITMAP_BITS 1024	/* set to default value since some H/W may not support 0x10000 even with a 2.6.8 kernel */
#define IO_BITMAP_BITS_16 65536
#endif

static struct tcase_t {
	long from;
	long num;
	int turn_on;
	char *desc;
	int exp_errno;
} tcases[] = {
	{0, NUM_BYTES, 1, "Invalid I/O address", EINVAL},
	{0, NUM_BYTES, 1, "Non super-user", EPERM},
};

static void setup(void)
{
	tcases[0].from = (IO_BITMAP_BITS_16 - NUM_BYTES) + 1;
	tcases[1].from = IO_BITMAP_BITS_16 - NUM_BYTES;

	struct passwd *pw;
	pw = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(pw->pw_uid);
}

static void cleanup(void)
{
	SAFE_SETEUID(0);
}

static void verify_ioperm(unsigned int i)
{
	TEST(ioperm(tcases[i].from, tcases[i].num, tcases[i].turn_on));

	if ((TST_RET == -1) && (TST_ERR == tcases[i].exp_errno)) {
		tst_res(TPASS | TTERRNO, "Expected failure for %s, "
				"errno: %d", tcases[i].desc, TST_ERR);
	} else {
		tst_res(TFAIL | TTERRNO, "Unexpected results for %s ; "
				"returned %ld (expected -1), errno %d "
				"(expected errno %d)", tcases[i].desc,
				TST_RET, TST_ERR, tcases[i].exp_errno);
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_ioperm,
	.needs_root = 1,
	/* ioperm() is restricted under kernel lockdown. */
	.skip_in_lockdown = 1,
	.setup = setup,
	.cleanup = cleanup,
};

#else
TST_TEST_TCONF("LSB v1.3 does not specify ioperm() for this architecture. (only for i386 or x86_64)");
#endif /* __i386_, __x86_64__*/
