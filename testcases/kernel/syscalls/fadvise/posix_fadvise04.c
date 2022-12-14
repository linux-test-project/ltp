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

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "tst_test.h"

#include "lapi/syscalls.h"

static int pipedes[2];

static int defined_advise[] = {
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
			TST_RET, tst_strerrno(TST_RET));
	} else {
		tst_res(TFAIL,
			"unexpected return value - %ld : %s - "
			"expected %d", TST_RET,
			tst_strerrno(TST_RET), ESPIPE);
	}
}

void setup(void)
{
	SAFE_PIPE(pipedes);

	SAFE_CLOSE(pipedes[1]);
}

void cleanup(void)
{
	if (pipedes[0] > 0)
		SAFE_CLOSE(pipedes[0]);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_fadvise,
	.tcnt = ARRAY_SIZE(defined_advise),
};
