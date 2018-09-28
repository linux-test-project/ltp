// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Red Hat Inc., 2007
 */

/*
 * NAME
 *	posix_fadvise02.c
 *
 * DESCRIPTION
 *	Check the value that posix_fadvise returns for wrong file descriptor.
 *
 * USAGE
 *	posix_fadvise02
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

#define WRONG_FD       42	/* The number has no meaning.
				   Just used as something wrong fd */

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
	     (WRONG_FD, 0, 0, defined_advise[n]));

	if (TST_RET == 0) {
		tst_res(TFAIL, "call succeeded unexpectedly");
		return;
	}

	/* Man page says:
	   "On error, an error number is returned." */
	if (TST_RET == EBADF) {
		tst_res(TPASS, "expected failure - "
			"returned value = %ld : %s",
			TST_RET, strerror(TST_RET));
	} else {
		tst_res(TFAIL,
			"unexpected returnd value - %ld : %s - "
			"expected %d", TST_RET,
			strerror(TST_RET), EBADF);
	}
}

void setup(void)
{
	/* Make WRONG_FD really wrong. */
retry:
	errno = 0;
	if (close(WRONG_FD) != 0) {
		if (errno == EBADF) {}	/* Good. Do nothing. */
		if (errno == EINTR)
			goto retry;
		else if (errno == EIO)
			tst_brk(TBROK,
				"Unable to close a file descriptor(%d): %s\n",
				WRONG_FD, strerror(EIO));
	}
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_fadvise,
	.tcnt = ARRAY_SIZE(defined_advise),
};

#else
	TST_TEST_TCONF("This test can only run on kernels that implements "
			"fadvise64 which is used from posix_fadvise");
#endif
