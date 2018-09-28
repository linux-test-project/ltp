// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Red Hat Inc., 2007
 */

/*
 * NAME
 *	posix_fadvise03.c
 *
 * DESCRIPTION
 *	Check the value that posix_fadvise returns for wrong ADVISE value.
 *
 * USAGE
 *	posix_fadvise03
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
#include <limits.h>
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

char fname[] = "/bin/cat";	/* test executable to open */
int fd = -1;			/* initialized in open */

int expected_error = EINVAL;

int defined_advise[] = {
	POSIX_FADV_NORMAL,
	POSIX_FADV_SEQUENTIAL,
	POSIX_FADV_RANDOM,
	POSIX_FADV_WILLNEED,
#if defined(__s390__) && __WORDSIZE == 32
	/* POSIX_FADV_DONTNEED and POSIX_FADV_NOREUSE are 6,7 on 31bit s390,
	 * but the kernel accepts 4,5 as well and rewrites them internally,
	 * see Linux kernel commit 068e1b94bbd268f375349f68531829c8b7c210bc
	 *
	 * since header definitions are incomplete - posix fcntl.h doesn't care
	 * and defines them as 4,5 while linux/fadvise.h (which uses 6,7)
	 * matches only 64bit - we need to hardcode the values here for
	 * all 4 cases, unfortunately
	 */
	4, /* POSIX_FADV_DONTNEED */
	5, /* POSIX_FADV_NOREUSE */
	6, /* POSIX_FADV_DONTNEED */
	7, /* POSIX_FADV_NOREUSE */
#else
	POSIX_FADV_DONTNEED,
	POSIX_FADV_NOREUSE,
#endif
};

const int defined_advise_total = ARRAY_SIZE(defined_advise);

#define ADVISE_LIMIT 32

/* is_defined_advise:
   Return 1 if advise is in defined_advise.
   Return 0 if not. */
static int is_defined_advise(int advise)
{
	int i;
	for (i = 0; i < defined_advise_total; i++) {
		if (defined_advise[i] == advise)
			return 1;
	}

	return 0;
}

static void verify_fadvise(unsigned int n)
{
	/* Don't use defined advise as an argument. */
	if (is_defined_advise(n)) {
		tst_res(TPASS, "skipping defined - advise = %d", n);
		return;
	}

	TEST(posix_fadvise(fd, 0, 0, n));

	if (TST_RET == 0) {
		tst_res(TFAIL, "call succeeded unexpectedly");
		return;
	}

	/* Man page says:
	   "On error, an error number is returned." */
	if (TST_RET == expected_error) {
		tst_res(TPASS,
			"expected failure - "
			"returned value = %ld, advise = %d : %s",
			TST_RET,
			n, strerror(TST_RET));
	} else {
		tst_res(TFAIL,
			"unexpected return value - %ld : %s, advise %d - "
			"expected %d",
			TST_RET,
			strerror(TST_RET),
			n, expected_error);
	}
}

static void setup(void)
{
	fd = SAFE_OPEN(fname, O_RDONLY);
}

static void cleanup(void)
{
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_fadvise,
	.tcnt = ADVISE_LIMIT,
};

#else
	TST_TEST_TCONF("This test can only run on kernels that implements "
			"fadvise64 which is used from posix_fadvise");
#endif
