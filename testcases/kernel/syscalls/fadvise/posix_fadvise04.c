// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Red Hat Inc., 2007
 */

/*
 * NAME
 *	posix_fadvise04.c
 *
 * DESCRIPTION
 *	Check the value that posix_fadvise returns for pipe descriptor.
 *
 * USAGE
 *	posix_fadvise04
 *
 * HISTORY
 *	11/2007 Initial version by Masatake YAMATO <yamato@redhat.com>
 *
 * RESTRICTIONS
 *	None
 */

#define _XOPEN_SOURCE 600
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "tst_test.h"

#include "lapi/syscalls.h"
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 32
#endif

#ifndef __NR_fadvise64
#define __NR_fadvise64 0
#endif

/* Check this system has fadvise64 system which is used in posix_fadvise. */
#if ((_FILE_OFFSET_BITS == 64) || (__NR_fadvise64 != 0))

int pipedes[2];

int defined_advise[] = {
	POSIX_FADV_NORMAL,
	POSIX_FADV_SEQUENTIAL,
	POSIX_FADV_RANDOM,
	POSIX_FADV_NOREUSE,
	POSIX_FADV_WILLNEED,
	POSIX_FADV_DONTNEED,
};

static void verify_fadvise(unsigned int n)
{
	TEST(posix_fadvise
	     (pipedes[0], 0, 0, defined_advise[n]));

	if (TST_RET == 0) {
		tst_res(TFAIL, "call succeeded unexpectedly");
		return;
	}

	/* Man page says:
	   "On error, an error number is returned." */
	if (TST_RET == ESPIPE) {
		tst_res(TPASS, "expected failure - "
			"returned value = %ld : %s",
			TST_RET, strerror(TST_RET));
	} else {
		tst_res(TFAIL,
			"unexpected return value - %ld : %s - "
			"expected %d", TST_RET,
			strerror(TST_RET), ESPIPE);
	}
}

void setup(void)
{
	/* Make a pipe */
	SAFE_PIPE(pipedes);

	/* Close write side first.  I don't use it in test. */
	SAFE_CLOSE(pipedes[1]);
}

void cleanup(void)
{
	/* Close pipe of read side */
	SAFE_CLOSE(pipedes[0]);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_fadvise,
	.tcnt = ARRAY_SIZE(defined_advise),
	.min_kver = "2.6.16",
};

#else
	TST_TEST_TCONF("This test can only run on kernels that implements "
			"fadvise64 which is used from posix_fadvise");
#endif
