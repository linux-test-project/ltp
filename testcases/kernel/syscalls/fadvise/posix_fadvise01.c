// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Red Hat Inc., 2007
 */

/*
 * NAME
 *	posix_fadvise01.c
 *
 * DESCRIPTION
 *	Check the value that posix_fadvise returns for wrong ADVISE value.
 *
 * USAGE
 *	posix_fadvise01
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

char fname[] = "/bin/cat";	/* test executable to open */
int fd = -1;			/* initialized in open */

int expected_return = 0;

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
	TEST(posix_fadvise(fd, 0, 0, defined_advise[n]));

	/* Man page says:
	   "On error, an error number is returned." */
	if (TST_RET == expected_return) {
		tst_res(TPASS, "call succeeded expectedly");
	} else {
		tst_res(TFAIL,
			"unexpected return value - %ld : %s, advise %d - "
			"expected %d",
			TST_RET,
			tst_strerrno(TST_RET),
			defined_advise[n], expected_return);
	}
}

static void setup(void)
{
	fd = SAFE_OPEN(fname, O_RDONLY);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_fadvise,
	.tcnt = ARRAY_SIZE(defined_advise),
};
